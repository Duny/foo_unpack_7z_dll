#include "stdafx.h"

#include "cache_system.h"
#include "archive.h"
#include "archive_info_cache.h"
#include "file_cache.h"

namespace unpack_7z
{
    class cache_system_impl : public cache_system
    {
        // member variables
        file_cache         m_file_cache;
        archive_info_cache m_archive_info_cache;


        // cache_system overrides
        t_filestats get_stats_in_archive (const char *p_archive, const char *p_file, abort_callback &p_abort) override
        {
            pfc::string8 path_canonical;
            filesystem::g_get_canonical_path (p_archive, path_canonical);
            return m_archive_info_cache.get_file_stats (path_canonical, p_file, p_abort);
        }
        
        void open_archive (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort) override
        {
            pfc::string8 path_canonical;
            filesystem::g_get_canonical_path (p_archive, path_canonical);

            if (pfc::stricmp_ascii (p_file, "\\front.jpg") == 0) {
                int i = 0;
            }

            if (!m_file_cache.fetch (p_out, path_canonical, p_file, p_abort)) {
                archive::file_list files;
                bool files_valid = m_archive_info_cache.query_file_list (path_canonical, files);

                unpack_7z::archive a;
                a.open (path_canonical, p_abort, !files_valid);

                if (!files_valid) {
                    files = a.get_list ();
                    m_archive_info_cache.add_entry (a);
                }

                auto index = files.find_item (p_file);
                if (index == pfc_infinite) throw exception_arch_file_not_found ();
                extract_internal (p_out, a, files, index, p_abort);
            }
        }

        void archive_list (foobar2000_io::archive *owner, const char *p_archive, const file_ptr &p_reader, archive_callback &p_out, bool p_want_readers) override
        {
            pfc::string8 path_canonical;
            filesystem::g_get_canonical_path (p_archive, path_canonical);

            if (p_want_readers) {
                archive::file_list files;
                bool files_valid = m_archive_info_cache.query_file_list (path_canonical, files);
                
                unpack_7z::archive a;
                a.open (path_canonical, p_reader, p_out, !files_valid);

                if (!files_valid) {
                    files = a.get_list ();
                    m_archive_info_cache.add_entry (a);
                }

                for (t_size i = 0, max = files.get_size (); i < max; i++) {
                    file_ptr temp;
                    extract_internal (temp, a, files, i, p_out);
                    if (!p_out.on_entry (owner, files[i].m_unpack_path, files[i].m_stats, temp))
                        return;
                }
            }
            else { // special case for fast listing
                pfc::list_t<archive::file_info> files;
                m_archive_info_cache.get_file_list (path_canonical, files, p_out);

                file_ptr dummy;
                for (t_size i = 0, max = files.get_size (); i < max; i++)
                    if (!p_out.on_entry (owner, files[i].m_unpack_path, files[i].m_stats, dummy))
                        return;
            }
        }

        void set_history_size_max (t_uint32 new_size) override
        {
            m_archive_info_cache.set_max_size (new_size);
        }

        void set_cache_size_max (t_uint32 new_size) override
        {
            m_file_cache.set_max_size (new_size);
        }

        void cache_free ()
        {
            m_file_cache.clear ();
        }

        void remove_dead_history_items () override
        {
            m_archive_info_cache.remove_dead_items ();
        }

        void history_clear () override
        {
            m_archive_info_cache.clear ();
        }

        void print_stats () override
        {
            m_file_cache.print_stats ();
            m_archive_info_cache.print_stats ();
        }

        // helpers
        void extract_internal (file_ptr &p_out, const unpack_7z::archive &a, archive::file_list_cref p_info, t_size index, abort_callback &p_abort)
        {
            if (!m_file_cache.fetch (p_out, a.get_path (), p_info[index].m_path, p_abort)) {
                p_out = new file_tempmem (p_info[index].m_stats);
                a.extract_file (p_out, index, p_abort);
                m_file_cache.store (p_out, a.get_path (), p_info[index].m_path, p_abort);
            }
        }
    };

    namespace { service_factory_single_t<cache_system_impl> g_factory; }
}