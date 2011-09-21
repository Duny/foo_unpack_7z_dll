#include "stdafx.h"

#include "disk_cache.h"
#include "config.h"
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
                    return pfc::stricmp_ascii (slot.archive_path, archive) == 0 && pfc::stricmp_ascii (slot.file_path, file) == 0;
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
                    p_out = new service_impl_t<tempmem_with_timestamp> (pos->timestamp);
                    file::g_transfer_file (pos->file, p_out, p_abort);
                    return true;
                }
                else
                    return false;
            }

            void store (const char *p_archive, const char *p_file, const file_ptr &file_new, abort_callback &p_abort)
            {
                if (cfg::disk_cache_size == 0)
                    return;

                insync (m_lock);

                auto pos = find_if (m_cache.begin (), m_cache.end (), cache_slot_equal (p_archive, p_file));
                if (pos == m_cache.end ()) {
                    cache_slot &slot = m_cache[m_next_slot];

                    if (slot.file.is_empty ())
                        filesystem::g_open_temp (slot.file, p_abort);

                    file::g_transfer_file (file_new, slot.file, p_abort);

                    slot.archive_path = p_archive;
                    slot.file_path = p_file;
                    slot.timestamp = file_new->get_timestamp (p_abort);

                    m_next_slot = (m_next_slot + 1) % cfg::disk_cache_size;
                }
            }

        public:
            manager_impl () : m_cache (cfg::disk_cache_size), m_next_slot (0) {}
        };
        static service_factory_single_t<manager_impl> g_manager_impl_factory;
    }
}