#ifndef _FOO_UNPACK_7Z_DLL_ARCHIVE_INFO_CACHE_H_
#define _FOO_UNPACK_7Z_DLL_ARCHIVE_INFO_CACHE_H_

namespace unpack_7z
{
    class archive_info_cache : public cfg_var
    {
        struct entry_t
        {
            t_filetimestamp    m_timestamp; // Archive timestamp
            archive::file_list m_files; // Archive contents

            inline void init (const unpack_7z::archive &p_archive)
            {
                m_files = p_archive.get_info ();
                m_timestamp = p_archive.get_timestamp ();
            }
        };

        // cfg_var overrides
        void get_data_raw (stream_writer *p_stream, abort_callback &p_abort) // Called on shutdown for storing to disk
        {
            stream_writer_formatter<> out (*p_stream, p_abort);
            auto max_items = min (cfg::archive_history_max, m_size); // Check max data size and truncate if needed
            out << max_items;
            for (pfc::map_t<GUID, entry_t>::const_iterator walk = m_data.first (); walk.is_valid () && max_items; ++walk, --max_items)
                out << walk->m_key << walk->m_value.m_timestamp << walk->m_value.m_files;
        }

	    void set_data_raw (stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort) // Called on startup for reading from disk
        {
            stream_reader_formatter<> in (*p_stream, p_abort);
            t_uint32 count; in >> count;
            m_size = count;
            while (count --> 0) {
                GUID key; in >> key;
                entry_t &e = m_data.find_or_add (key);
                in >> e.m_timestamp >> e.m_files;
            }
	    }

        // helpers
        inline entry_t * find_or_add (const char *p_archive, abort_callback &p_abort)
        {
            GUID key = make_key (p_archive);
            entry_t * e;
            
            bool is_new = m_data.query_ptr (key, e) == false;
            if (!is_new) {
                t_filestats stats;
                bool is_writable;
                filesystem::g_get_stats (p_archive, stats, is_writable, p_abort);
                if (e->m_timestamp != stats.m_timestamp) // archive was updated, need to reload it
                    is_new = true;
            }

            if (is_new) {
                make_room_for_new_item ();
                entry_t & new_entry = m_data.find_or_add_ex (key, is_new);
                new_entry.init (unpack_7z::archive (p_archive, p_abort));
                if (is_new) m_size++;
                return &new_entry;
            }
            
            return e;
        }

        inline GUID make_key (const char *p_str) const { return GUID_from_text_md5 (string_lower (p_str)); }

        inline void make_room_for_new_item ()
        {
            if (m_size + 1 > cfg::archive_history_max) {
                if (m_size > 0) { // remove random item
                    auto n = ReadTimeStampCounter () % m_size; // Pick item to remove
                    auto walk = m_data.first ();
                    while (n --> 0) walk++; // Go to its logical position
                    m_data.remove (walk); // And delete
                    m_size--;
                }
            }
        }

        // Member variables
        pfc::map_t<GUID, entry_t> m_data; // GUID is made of md5 from canonical path to archive
        t_uint32                  m_size;
        mutable critical_section  m_lock; // Synchronization for accessing m_data
            
    public:
        archive_info_cache () : cfg_var (guid_inline<0x8D96A7C4, 0x9855, 0x4076, 0xB9, 0xD7, 0x88, 0x82, 0x23, 0x50, 0xBF, 0xCA>::guid) {}

        inline t_filestats get_file_stats (const char *p_archive, const char *p_file, abort_callback &p_abort)
        {
            insync (m_lock);
            entry_t *e = find_or_add (p_archive, p_abort);
            auto n = e->m_files.find_item (p_file);
            if (n == pfc_infinite) throw exception_arch_file_not_found ();
            return e->m_files[n].m_stats;    
        }

        inline void get_file_list (const char *p_archive, archive::file_list &p_out, abort_callback &p_abort)
        {
            insync (m_lock);
            p_out = find_or_add (p_archive, p_abort)->m_files;
        }

        inline bool query_file_list (const char *p_archive, archive::file_list &p_out) const
        {
            insync (m_lock);
            const entry_t *e;
            bool res = m_data.query_ptr (make_key (p_archive), e);
            if (res) p_out = e->m_files;
            return res;
        }

        inline void add_entry (const unpack_7z::archive &p_archive)
        {
            insync (m_lock);

            bool is_new;
            entry_t & e = m_data.find_or_add_ex (make_key (p_archive.get_path ()), is_new);
            e.init (p_archive);
            if (is_new) {
                make_room_for_new_item ();
                m_size++;
            }
        }

        inline void print_stats (pfc::string_formatter & out) const
        {
            insync (m_lock);
            out << "Archive info cache: " << m_size << " item(s)\n";
        }
    };
}   

#endif