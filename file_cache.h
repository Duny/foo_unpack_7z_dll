#ifndef _FOO_UNPACK_7Z_DLL_FILE_CACHE_H_
#define _FOO_UNPACK_7Z_DLL_FILE_CACHE_H_

namespace unpack_7z
{
    class file_cache
    {
        struct entry_t
        {
            t_filestats  m_stats; // Stats of file from archive
            pfc::string8 m_path; // Physical path of cache entry file. Empty if default system temp folder was used
            file_ptr     m_file; // Cache entry file

            ~entry_t () { free (); }

            inline void init (const char *p_file, const t_filestats &p_stats, abort_callback &p_abort)
            {
                free ();
                if (cfg::use_sys_tmp_for_cache)
                    filesystem::g_open_temp (m_file, p_abort);
                else {
                    generate_temp_location_for_file (m_path, cfg::custom_cache_path, "tmp", p_file);
                    filesystem::g_open_write_new (m_file, m_path, p_abort);
                }
                m_stats = p_stats;
            }

            inline void free ()
            {
                if (m_file.is_valid ()) m_file.release ();
                // delete file if not system temp
                if (!m_path.is_empty ()) { uDeleteFile (m_path); m_path.reset (); }
            }
        };

        inline entry_t * alloc_entry (const char *p_archive, const char *p_file, t_filesize file_size, bool & is_new)
        {
            t_filesize max_size = cfg::file_cache_max; max_size <<= 20;
            if (file_size > max_size) return nullptr;

            auto new_size = m_size + file_size;
            if (new_size > max_size) {
                typedef boost::tuple<GUID, t_filesize> cache_file;
                
                pfc::list_t<cache_file> files;
                m_data.enumerate ([&](const GUID &key, const entry_t &value) { files.add_item (boost::make_tuple (key, value.m_stats.m_size)); });
                files.sort_t ([] (const cache_file &left, const cache_file &right) -> int { return pfc::compare_t (left.get<1>(), right.get<1>()); });

                t_size i = 0, n = files.get_size ();
                for (; i < n; i++) // search for the smallest possible file to delete
                    if ((new_size - files[i].get<1> ()) < max_size) break;

                if (i != n) { // file was found
                    m_data.remove (files[i].get<0>());
                    m_size -= files[i].get<1> ();
                }
                else { // there is no such big enough file in cache to accommodate our new file
                    // Very ugly solution:
                    // Delete each file starting from the end, until we reach desired cache size
                    while ((n --> 0) && (new_size > max_size)) {
                        m_data.remove (files[n].get<0>());
                        m_size -= files[n].get<1> ();
                        new_size -= files[n].get<1> ();
                    }
                }
            }
            
            return &m_data.find_or_add_ex (make_key (p_archive, p_file), is_new);
        }

        // Helpers
        inline GUID make_key (const char *p_archive, const char *p_file) const 
        { 
            pfc::string8_fast str = string_lower (p_archive);
            str.add_string (string_lower (p_file));
            return GUID_from_text_md5 (str);
        }

        pfc::map_t<GUID, entry_t> m_data; // GUID is made of md5 from canonical path to archive+file path in archive
        t_filesize                m_size; // size in bytes of used space
        mutable critical_section  m_lock;

    public:
        bool fetch (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort) const
        {
            insync (m_lock);

            const entry_t *e;
            if (m_data.query_ptr (make_key (p_archive, p_file), e)) {
                p_out = new file_tempmem (e->m_stats);
                try {
                    file::g_transfer_file (e->m_file, p_out, p_abort);
                    p_out->seek (0, p_abort);
                    return true;
                } catch (const std::exception &e) {
                    error_log () << "Disk cache fetch exception \"" << e.what () << "\"";
                    p_out.release ();
                }
            }
            return false;
        }

        void store (const file_ptr &p_in, const char *p_archive, const char *p_file, abort_callback &p_abort)
        {
            insync (m_lock);            

            auto stats = p_in->get_stats (p_abort);
            bool is_new;
            entry_t *e = alloc_entry (p_archive, p_file, stats.m_size, is_new);
            if (e && is_new) {
                try {
                    e->init (p_file, stats, p_abort);
                    file::g_transfer_file (p_in, e->m_file, p_abort);
                    p_in->seek (0, p_abort);
                    m_size += stats.m_size;
                } catch (const std::exception &ex) {
                    error_log () << "Disk cache store exception \"" << ex.what () << "\"";
                    m_data.remove (make_key (p_archive, p_file));
                }
            }
        }

        inline void print_stats () const
        {
            t_filesize max_size = cfg::file_cache_max; max_size <<= 20;
            console::formatter () << "File cache disk usage: " << pfc::format_file_size_short (m_size) << " of " << pfc::format_file_size_short (max_size) << "\n";
        }
    };
}
#endif