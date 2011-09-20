#include "stdafx.h"

#include "disk_cache.h"
#include "config.h"

namespace unpack_7z
{
    namespace disk_cache
    {
        class file_cached_imp : public file_cached
        {
            file_ptr m_file;

            virtual bool is_empty () const { return m_file.is_empty (); }

        public:
            file_cached_imp (const file_ptr &p_file) : m_file (p_file) {}
        };


        class manager_impl : public manager
        {
            struct cache_slot
            {
                pfc::string8 m_archive_path;
                pfc::string8 m_file_path; // path from inside the archive, e.i. "some folder\somefile.mp3"

                file_ptr m_file;
            };
            std::vector<cache_slot> m_cache;
            std::vector<t_uint32> m_readers_count;
            std::vector<HANDLE> m_not_reading_from_slot;

            critical_section m_cache_lock;

            HANDLE m_not_writing;

            bool m_successful_init;


            bool wait_for_finish_writing () const { return WaitForSingleObject (m_not_writing, INFINITE) == WAIT_OBJECT_0; }

            virtual file_cached_ptr find (const char *p_archive, const char *p_file)
            {
                if (m_successful_init && wait_for_finish_writing ()) {
                    insync (m_cache_lock);
                }

                return new service_impl_t<file_cached_imp>(file_ptr ());
            }

        public:
            manager_impl ()
            {
                m_successful_init = (m_not_writing = uCreateEvent (nullptr, TRUE, TRUE, nullptr)) != NULL;
                for (t_uint32 i = 0; i < cfg::disk_cache_size && m_successful_init; i++) {
                    HANDLE hEvent = uCreateEvent (nullptr, TRUE, TRUE, nullptr);
                    if (hEvent) {
                        m_not_reading_from_slot.push_back (hEvent);
                        m_readers_count.push_back (0);
                    }
                    else
                        m_successful_init = false;
                }
            }

            ~manager_impl ()
            {
                if (m_not_writing) CloseHandle (m_not_writing);

                for (t_uint32 i = 0, n = m_not_reading_from_slot.size (); i < n; i++)
                    CloseHandle (m_not_reading_from_slot[i]);
            }
        };
        static service_factory_single_t<manager_impl> g_manager_impl_factory;
    }
}