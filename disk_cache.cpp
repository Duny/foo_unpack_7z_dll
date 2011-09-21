#include "stdafx.h"

#include "disk_cache.h"
#include "archive.h"
#include "config.h"
#include "utils.h"
#include "tempmem_with_timestamp.h"

namespace unpack_7z
{
    namespace disk_cache
    {
        using namespace std;
        class manager_impl : public manager
        {
            struct cache_slot
            {
                pfc::string8 archive_path;
                pfc::string8 file_path; // path from inside the archive, e.i. "some folder\somefile.mp3"
                file_ptr file;
                t_filetimestamp timestamp;
            };

            vector<cache_slot> m_cache;
            t_uint32 m_next_slot;

            critical_section m_lock;

            struct cache_slot_equal
            {
                cache_slot_equal (const char *p_archive, const char *p_file)
                    : archive (p_archive), file (p_file) {}

                bool operator () (const cache_slot &slot)
                {
                    return slot.file.is_valid () && pfc::stricmp_ascii (slot.archive_path, archive) == 0 && pfc::stricmp_ascii (slot.file_path, file) == 0;
                }

                const char *archive;
                const char *file;
            };

            bool fetch (const char *p_archive, const char *p_file, file_ptr &p_out, abort_callback &p_abort) override
            {
                if (cfg::disk_cache_size == 0)
                    return false;

                insync (m_lock);

                auto pos = find_if (m_cache.begin (), m_cache.end (), cache_slot_equal (p_archive, p_file));
                if (pos != m_cache.end ()) {
                    p_out = new file_tempmem (pos->timestamp);
                    try {
                        file::g_transfer_file (pos->file, p_out, p_abort);
                        return true;
                    } catch (const std::exception &e) {
                        p_out.release ();
                        error_log () << "disk cache fetch exception:" << e.what ();
                    }
                }

                return false;
            }

            void store (const char *p_archive, const char *p_file, const file_ptr &p_in, const t_filetimestamp &timestamp, abort_callback &p_abort)
            {
                if (cfg::disk_cache_size == 0)
                    return;

                insync (m_lock);

                auto pos = find_if (m_cache.begin (), m_cache.end (), cache_slot_equal (p_archive, p_file));
                if (pos == m_cache.end ()) {
                    cache_slot &slot = m_cache[m_next_slot];

                    m_next_slot = (m_next_slot + 1) % cfg::disk_cache_size;

                    try {
                        if (slot.file.is_empty ())
                            filesystem::g_open_temp (slot.file, p_abort);

                        file::g_transfer_file (p_in, slot.file, p_abort);
                    } catch (const std::exception &e) {
                        error_log () << "disk cache store exception:" << e.what ();
                        return;
                    }

                    slot.archive_path = p_archive;
                    slot.file_path = p_file;
                    slot.timestamp = timestamp;
                }
            }

        public:
            manager_impl () : m_cache (cfg::disk_cache_size), m_next_slot (0) {}
        };
        static service_factory_single_t<manager_impl> g_manager_impl_factory;

        bool fetch (const char *p_archive, const char *p_file, file_ptr &p_out, abort_callback &p_abort)
        {
            static_api_ptr_t<disk_cache::manager> api;
            if (api->fetch (p_archive, p_file, p_out, p_abort))
                return true;
            else
                return fetch (unpack_7z::archive (p_archive, p_abort), p_file, p_out, p_abort);
        }

        bool fetch (const unpack_7z::archive &p_archive, const char *p_file, file_ptr &p_out, abort_callback &p_abort)
        {
            const t_filestats &stats = p_archive.get_stats (p_file);

            static_api_ptr_t<disk_cache::manager> api;
            if (api->fetch (p_archive.get_path (), p_file, p_out, p_abort))
                return true;
            else {
                p_out = new file_tempmem (stats.m_timestamp);
                p_archive.extract_file (p_file, p_out, p_abort);
                api->store (p_archive.get_path (), p_file, p_out, stats.m_timestamp, p_abort);
                return true;
            }
        }
    }
}