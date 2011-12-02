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
            return m_archive_info_cache.get_file_stats (p_archive, p_file, p_abort);
        }
        
        void open_archive (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort) override
        {
            if (!m_file_cache.fetch (p_out, p_archive, p_file, p_abort)) {
                archive::file_list files;
                bool files_valid = m_archive_info_cache.query_file_list (p_archive, files);

                unpack_7z::archive a;
                a.open (p_archive, p_abort, !files_valid);

                if (!files_valid) {
                    files = a.get_info ();
                    m_archive_info_cache.add_entry (a);
                }

                auto index = files.find_item (p_file);
                if (index == pfc_infinite) throw exception_arch_file_not_found ();
                extract_internal (p_out, a, files, index, p_abort);
            }
        }

        void archive_list (foobar2000_io::archive *owner, const char *p_archive, const file_ptr &p_reader, archive_callback &p_out, bool p_want_readers) override
        {
            if (p_want_readers) {
                archive::file_list files;
                bool files_valid = m_archive_info_cache.query_file_list (p_archive, files);
                
                unpack_7z::archive a;
                p_reader.is_empty () ? a.open (p_archive, p_out, !files_valid) : a.open (p_archive, p_reader, p_out, !files_valid);

                if (!files_valid) {
                    files = a.get_info ();
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
                m_archive_info_cache.get_file_list (p_archive, files, p_out);

                file_ptr dummy;
                for (t_size i = 0, max = files.get_size (); i < max; i++)
                    if (!p_out.on_entry (owner, files[i].m_unpack_path, files[i].m_stats, dummy))
                        return;
            }
        }

        void print_stats () const override
        {
            m_file_cache.print_stats ();
            m_archive_info_cache.print_stats ();
        }


        // helpers
        void extract_internal (file_ptr &p_out, const unpack_7z::archive &a, archive::file_list_cref p_info, t_size index, abort_callback &p_abort)
        {
            if (!m_file_cache.fetch (p_out, a.get_path (), p_info[index].m_file_path, p_abort)) {
                p_out = new file_tempmem (p_info[index].m_stats);
                a.extract_file (p_out, index, p_abort);
                m_file_cache.store (p_out, a.get_path (), p_info[index].m_file_path, p_abort);
            }
        }
    };

    namespace { service_factory_single_t<cache_system_impl> g_factory; }
}