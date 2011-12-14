#ifndef _FOO_UNPACK_7Z_DLL_ARCHIVE_INFO_CACHE_H_
#define _FOO_UNPACK_7Z_DLL_ARCHIVE_INFO_CACHE_H_

namespace unpack_7z
{
    class archive_info_cache : public cfg_var
    {
        struct entry
        {
            pfc::string8       m_path; // Archive path
            t_filestats        m_stats; // Archive stats
            archive::file_list m_files; // Archive contents
            
            inline void init (const unpack_7z::archive &p_archive)
            {
                tie (m_path, m_stats, m_files) = make_tuple (p_archive.get_path (), p_archive.get_stats (), p_archive.get_list ());
            }
        };

        typedef pfc::map_t<t_uint64, entry> t_hash_map;
        typedef t_hash_map::t_key t_key; typedef t_hash_map::t_value t_value;


        // cfg_var overrides
        void get_data_raw (stream_writer *p_stream, abort_callback &p_abort) // Called on shutdown for storing to disk
        {
            insync (m_lock);

            stream_writer_formatter<> out (*p_stream, p_abort);
            out << m_size;
            m_data.enumerate ([&](const t_key &key, const t_value &val) { out << key << val.m_path << val.m_stats << val.m_files; });
        }

	    void set_data_raw (stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort) // Called on startup for reading from disk
        {
            insync (m_lock);

            stream_reader_formatter<> in (*p_stream, p_abort);
            decltype(m_size) count; in >> count;
            m_size = count;
            while (count --> 0) {
                t_key key; in >> key;
                t_value &e = m_data.find_or_add (key);
                in >> e.m_path >> e.m_stats >> e.m_files;
            }
	    }

        // helpers
        inline t_value * find_or_add (const char *p_archive, abort_callback &p_abort)
        {
            t_key key = hash (p_archive);
            t_value * e;
            
            bool is_new = m_data.query_ptr (key, e) == false;
            if (!is_new) {
                t_filestats stats;
                bool dummy;
                filesystem::g_get_stats (p_archive, stats, dummy, p_abort);
                if (e->m_stats != stats) { // archive was updated, need to reload it
                    remove_one_item (key);
                    is_new = true;
                }
            }

            if (is_new) {
                check_size_overflow ();
                t_value & new_entry = m_data.find_or_add (key);
                new_entry.init (unpack_7z::archive (p_archive, p_abort));
                m_size++;
                return &new_entry;
            }
            
            return e;
        }

        inline void check_size_overflow ()
        {
            if (cfg::archive_history_max != pfc_infinite)
                if (m_size > 0 && m_size + 1 > cfg::archive_history_max)
                    remove_random_items ();
        }

        inline void remove_random_items (t_uint32 count = 1)
        {
            auto r = genrand_service::g_create ();
            r->seed (static_cast<unsigned>(ReadTimeStampCounter ()));
            while (count --> 0) {
                auto n = r->genrand (m_size);
                auto walk = m_data.first ();
                while (n --> 0) walk++;
                remove_one_item (walk);
            }
        }

        template <class T> inline void remove_one_item (const T & were) { m_data.remove (were); m_size--; }

        // Member variables
        t_hash_map       m_data; // t_uint64 key is made of hash () from canonical path to archive
        t_size           m_size; // Item count
        critical_section m_lock; // Synchronization for accessing m_data
            
    public:
        archive_info_cache () : cfg_var (guid_inline<0x8D96A7C4, 0x9855, 0x4076, 0xB9, 0xD7, 0x88, 0x82, 0x23, 0x50, 0xBF, 0xCA>::guid) {}

        inline t_filestats get_file_stats (const char *p_archive, const char *p_file, abort_callback &p_abort)
        {
            insync (m_lock);
            t_value *e = find_or_add (p_archive, p_abort);
            auto n = e->m_files.find_item (p_file);
            if (n == pfc_infinite) throw exception_arch_file_not_found ();
            return e->m_files[n].m_stats;    
        }

        inline void get_file_list (const char *p_archive, archive::file_list &p_out, abort_callback &p_abort)
        {
            insync (m_lock);
            p_out = find_or_add (p_archive, p_abort)->m_files;
        }

        inline void set_max_size (t_uint32 new_size)
        {
            insync (m_lock);
            if (m_size > new_size) remove_random_items (m_size - new_size);
            cfg::archive_history_max = new_size;
        }

        inline void remove_dead_items ()
        {
            insync (m_lock);
            abort_callback_dummy p_abort;
            t_hash_map new_data;
            auto init_size = m_size;
            m_data.enumerate ([&](const t_key &key, const t_value &val) {
                if (!val.m_path.is_empty () && filesystem::g_exists (val.m_path, p_abort))
                    new_data.set (key, val);
                else
                    m_size--;
            });
            m_data = new_data;
            debug_log () << "Removed " << init_size - m_size << " dead archives from history";
        }

        inline void clear ()
        {
            insync (m_lock);
            m_data.remove_all ();
            m_size = 0;
        }

        inline void print_stats ()
        {
            insync (m_lock);
            console::formatter f;
            f << "Archive info cache contains " << m_size;
            if (cfg::archive_history_max != pfc_infinite)
                f << " / " << cfg::archive_history_max;
            f << " archives\n";
        }
    };
}
#endif