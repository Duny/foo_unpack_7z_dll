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

            inline void init (const char *p_file, abort_callback &p_abort)
            {
                if (cfg::use_sys_tmp_for_cache) {
                    generate_temp_location_for_file (m_path, cfg::custom_cache_path, "tmp", p_file);
                    filesystem::g_open_write_new (m_file, m_path, p_abort);
                }
                else
                    filesystem::g_open_temp (m_file, p_abort);
            }

            inline void free ()
            {
                if (m_file.is_valid ()) m_file.release ();
                // delete file if not system temp
                if (!m_path.is_empty ()) uDeleteFile (m_path);
            }

            inline bool is_valid () const { return m_file.is_valid (); }

//            inline bool operator== (const archive_file_info &other) const { return is_valid () && m_archive_file_info == other; }
        };

        critical_section m_lock;
        pfc::map_t<GUID, entry_t> m_data;  // GUID is made of md5 from concat of canonical path to archive and file path in archive
        t_size m_next_entry;
    public:

        bool fetch (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort)
        {
            /*if (cfg::cache_size != 0) {
                insync (m_lock);

                auto n = m_data.find_item (archive_file_info (p_archive, p_file));
                if (n != pfc_infinite) {
                    p_out = new file_tempmem (m_data[n].m_archive_file_info.m_stats);
                    try {
                        file::g_transfer_file (m_data[n].m_file, p_out, p_abort);
                        p_out->seek (0, p_abort);
                        return true;
                    } catch (const std::exception &e) {
                        error_log () << "disk cache fetch exception:" << e.what ();
                        p_out.release ();
                    }
                }
            }*/
            return false;
        }

        void store (const file_ptr &p_in, const char *p_archive, const char *p_file, abort_callback &p_abort)
        {
            //if (cfg::cache_size != 0) {
            //    insync (m_lock);

            //    // setup cache on first call
            //    if (!m_data.get_size ()) m_data.set_size (cfg::cache_size);

            //    // do not store the same file twice
            //    if (m_data.find_item (archive_file_info (p_archive, p_file)) == pfc_infinite) {
            //        cache_entry &entry = m_data[m_next_entry];
            //        try {
            //            if (!entry.is_valid ()) // initialize entry on demand
            //                entry.init (p_file, p_abort);

            //            file::g_transfer_file (p_in, entry.m_file, p_abort);
            //            p_in->seek (0, p_abort);

            //            entry.m_archive_file_info.set (p_archive, p_file, p_in->get_stats (p_abort));
            //        } catch (const std::exception &e) {
            //            error_log () << "disk cache store exception:" << e.what ();
            //            entry.free ();
            //        }
            //        m_next_entry = (m_next_entry + 1) % cfg::cache_size;
            //    }
            //}
        }

        inline void print_stats (pfc::string_formatter & out) const
        {
        }
    };
}
#endif