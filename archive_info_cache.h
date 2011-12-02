#ifndef _FOO_UNPACK_7Z_DLL_ARCHIVE_INFO_CACHE_H_
#define _FOO_UNPACK_7Z_DLL_ARCHIVE_INFO_CACHE_H_

namespace unpack_7z
{
    class archive_info_cache : public cfg_var
    {
        struct entry_t
        {
            t_filetimestamp    m_timestamp; // archive timestamp
            archive::file_list m_files; // archive contents
        };

        // cfg_var overrides
        void get_data_raw (stream_writer *p_stream, abort_callback &p_abort) // called on shutdown for storing to disk
        {
            stream_writer_formatter<> out (*p_stream, p_abort);
            auto max_items = min (cfg::archive_history_max, m_data_size); // check max data size and truncate if needed
            out << max_items;
            for (pfc::map_t<GUID, entry_t>::const_iterator walk = m_data.first (); walk.is_valid () && max_items; ++walk, --max_items)
                out << walk->m_key << walk->m_value.m_timestamp << walk->m_value.m_files;
        }

	    void set_data_raw (stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort) // called on startup for reading from disk
        {
            stream_reader_formatter<> in (*p_stream, p_abort);
            t_uint32 count; in >> count;
            m_data_size = count;
            while (count --> 0) {
                GUID key; in >> key;
                entry_t &e = m_data.find_or_add (key);
                in >> e.m_timestamp >> e.m_files;
            }
	    }

        // helpers
        inline entry_t & find_or_add (const char *p_archive, abort_callback &p_abort)
        {
            t_filestats stats;
            bool is_new, dummy;

            GUID key = key_from_string (p_archive);
            entry_t & e = m_data.find_or_add_ex (key, is_new);
            filesystem::g_get_stats (p_archive, stats, dummy, p_abort);

            if (!is_new && e.m_timestamp != stats.m_timestamp)
                is_new = true;

            if (is_new) {
                if (m_data_size + 1 > cfg::archive_history_max)
                    remove_random_item ();
                
                entry_t & new_entry = m_data.find_or_add (key);
                unpack_7z::archive a (p_archive, p_abort);
                new_entry.m_files = a.get_info ();
                new_entry.m_timestamp = stats.m_timestamp;
                m_data_size++;
                return new_entry;
            }
            else
                return e;
        }

        inline void remove_random_item ()
        {
            if (m_data_size) {
                auto n = ReadTimeStampCounter () % m_data_size; // choose witch item to remove
                auto walk = m_data.first ();
                while (n --> 0) walk++; // go to it logical position
                m_data.remove (walk); // and delete it
                m_data_size--;
            }
        }

        inline GUID key_from_string (const char *p_str) const { return GUID_from_text_md5 (string_lower (p_str)); }


        // member variables
        pfc::map_t<GUID, entry_t> m_data; // GUID is made of md5 from canonical path to archive
        t_uint32                  m_data_size;
        mutable critical_section  m_lock; // synchronization for accessing m_data
            
    public:
        archive_info_cache () : cfg_var (guid_inline<0x8D96A7C4, 0x9855, 0x4076, 0xB9, 0xD7, 0x88, 0x82, 0x23, 0x50, 0xBF, 0xCA>::guid) {}

        inline t_filestats get_file_stats (const char *p_archive, const char *p_file, abort_callback &p_abort)
        {
            insync (m_lock);
            entry_t &e = find_or_add (p_archive, p_abort);
            auto n = e.m_files.find_item (p_file);
            if (n == pfc_infinite) throw exception_arch_file_not_found ();
            return e.m_files[n].m_stats;    
        }

        inline void get_file_list (const char *p_archive, archive::file_list &p_out, abort_callback &p_abort)
        {
            insync (m_lock);
            p_out = find_or_add (p_archive, p_abort).m_files;
        }

        inline bool query_file_list (const char *p_archive, archive::file_list &p_out)
        {
            insync (m_lock);
            entry_t e;
            bool res = m_data.query (key_from_string (p_archive), e);
            if (res) p_out = e.m_files;
            return res;
        }

        inline void add_entry (const unpack_7z::archive &p_archive)
        {
            insync (m_lock);

            bool is_new;
            entry_t & e = m_data.find_or_add_ex (key_from_string (p_archive.get_path ()), is_new);
            e.m_files = p_archive.get_info ();
            e.m_timestamp = p_archive.get_timestamp ();
            if (is_new) {
                if (m_data_size + 1 > cfg::archive_history_max)
                    remove_random_item ();
                
                m_data_size++;
            }
        }

        inline void print_stats () const
        {
            insync (m_lock);
            console::formatter() << "Archive info cache: " << m_data.get_count () << " item(s)";
        }
    };
}   

#endif