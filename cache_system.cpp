#include "stdafx.h"

#include "cache_system.h"
#include "archive.h"
#include "archive_info_cache.h"
#include "file_cache.h"

namespace unpack_7z
{
    class cache_system_impl : public cache_system, archive_changed_callback
    {
        // member variables
        file_cache         m_file_cache;
        archive_info_cache m_archive_info_cache;
        

        // overrides
        void on_changed (const pfc::string_base &p_archive) override
        {
            // If archive has been updated we need to free cached files from that archive
            // So archive files will be reextracted on next demand
            m_file_cache.clear_by_archive_name (p_archive);
        }

        // cache_system overrides
        t_filestats get_stats_in_archive (const char *p_archive, const char *p_file, abort_callback &p_abort) override
        {
            return m_archive_info_cache.get_file_stats (file_path_canonical (p_archive), p_file, p_abort);
        }
        
        void open_archive (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort) override
        {
            file_path_canonical path (p_archive);
            if (!m_file_cache.fetch (p_out, path, p_file, p_abort)) {
                archive::file_list files;
                m_archive_info_cache.get_file_list (path, files, p_abort);
                auto index = files.find_item (p_file);
                if (index == pfc_infinite) throw exception_arch_file_not_found ();
                extract_internal (p_out, unpack_7z::archive (path, p_abort, false), files, index, p_abort);
            }
        }

        void archive_list (foobar2000_io::archive *owner, const char *p_archive, const file_ptr &p_reader, archive_callback &p_out, bool p_want_readers) override
        {
            file_path_canonical path (p_archive);

            archive::file_list files;
            m_archive_info_cache.get_file_list (path, files, p_out);

            if (p_want_readers) { 
                // 6810162 6888128 6826243
                unpack_7z::archive a (path, p_reader, p_out, false);
                for (t_size i = 0, max = files.get_size (); i < max; i++) {
                    file_ptr temp;
                    extract_internal (temp, a, files, i, p_out);
                    if (!p_out.on_entry (owner, files[i].m_unpack_path, files[i].m_stats, temp))
                        return;
                }
                
                //archive_list_multithread (owner, p_archive, files, p_out, /*pfc::getOptimalWorkerThreadCount ()*/4);
            }
            else { // special case for fast listing
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

        void cache_free () override
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

        // Multithreaded extract
        typedef function<DWORD (const char *p_archive, t_size p_file_index)> t_thread_func;
        struct t_thread_data
        {
            const char *m_archive;
            t_size m_file_index;
            t_thread_func m_callback;
            inline void set (const char *p_archive, t_size p_index, const t_thread_func &p_func)
            {
                m_archive = p_archive, m_file_index = p_index, m_callback = p_func;
            }
        };
        static DWORD WINAPI ThreadProc (LPVOID lpParameter)
        {
            t_thread_data *data = static_cast<t_thread_data*>(lpParameter);
            return data->m_callback (data->m_archive, data->m_file_index);
        }

        void archive_list_multithread (foobar2000_io::archive *owner, const char *p_archive, archive::file_list_cref p_info, archive_callback &p_out, t_size max_threads = 1)
        {
            if (max_threads > MAXIMUM_WAIT_OBJECTS) max_threads = MAXIMUM_WAIT_OBJECTS;
            
            pfc::array_t<file_ptr> files_out;
            t_size file_max = p_info.get_size (), file_num = 0;
            files_out.set_size (file_max);

            pfc::array_t<t_thread_data> thread_data;
            thread_data.set_size (file_max);

            pfc::list_t<HANDLE> working_set;

            auto worker = [&] (const char *p_archive, t_size p_file_index) -> DWORD {
                extract_internal (files_out[p_file_index], unpack_7z::archive (p_archive, file_ptr (), p_out, false), p_info, p_file_index, p_out);
                return p_file_index;
            };

            auto start_new_thread = [&thread_data, worker, p_archive] (t_size file_index) -> HANDLE {
                DWORD dummy;
                t_thread_data & data = thread_data[file_index];
                data.set (p_archive, file_index, worker);
                auto h = CreateThread (nullptr, 0, ThreadProc, (LPVOID)&data, 0, &dummy);
                if (h == NULL) throw exception_win32 (GetLastError ());
                return h;
            };

            for (t_size i = 0; i < max_threads && file_num < file_max; i++, file_num++)
                working_set.add_item (start_new_thread (file_num));

            BOOST_SCOPE_EXIT( (&working_set) )
            {
                working_set.for_each ([](HANDLE h) { CloseHandle (h); });
            } BOOST_SCOPE_EXIT_END;

            while (working_set.get_size () > 0) {
                //uSleepSeconds (0.01f, true);
                // Test WaitForMultipleObjects with bWaitAll = TRUE
                auto finished = WaitForMultipleObjects (working_set.get_size (), working_set.get_ptr (), FALSE, INFINITE);
                if (finished >= WAIT_OBJECT_0 && finished <= (WAIT_OBJECT_0 + working_set.get_size () + 1)) {
                    auto thread_idx = finished - WAIT_OBJECT_0;
                    DWORD file_index;
                    if (GetExitCodeThread (working_set[thread_idx], &file_index) == 0) throw exception_win32 (GetLastError ());
                    CloseHandle (working_set[thread_idx]);

                    if (p_out.on_entry (owner, p_info[file_index].m_unpack_path, p_info[file_index].m_stats, files_out[file_index]))
                        files_out[file_index].release ();
                    else
                        return;

                    if (file_num < file_max) {
                        working_set[thread_idx] = start_new_thread (file_num);
                        file_num++;
                    }
                    else
                        working_set.remove_by_idx (thread_idx);
                }
                else
                    throw exception_win32 (GetLastError ());
            }
        }

    public:
        cache_system_impl () : m_archive_info_cache (*this) {}
    };

    namespace { service_factory_single_t<cache_system_impl> g_factory; }
}