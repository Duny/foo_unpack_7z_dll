#include "stdafx.h"

namespace unpack_7z
{
    namespace disk_cache
    {
        using namespace std;
        class manager_impl : public manager
        {
            struct cache_slot
            {
                pfc::string8 archive; // 7z archive full path
                pfc::string8 file_in_archive; // path from inside the archive, e.i. "some folder\somefile.mp3"

                pfc::string8 cache_path;
                file_ptr cache_file;
                t_filetimestamp timestamp;
            };

            vector<cache_slot> m_cache;
            t_uint32 m_next_slot;

            critical_section m_lock;

            struct cache_slot_equal
            {
                cache_slot_equal (const char *p_archive, const char *p_file) : archive (p_archive), file (p_file) {}

                bool operator () (const cache_slot &slot)
                {
                    return slot.cache_file.is_valid () && pfc::stricmp_ascii (slot.archive, archive) == 0
                        && pfc::stricmp_ascii (slot.file_in_archive, file) == 0;
                }

                const char *archive;
                const char *file;
            };

            bool fetch (const char *p_archive, const char *p_file, file_ptr &p_out, abort_callback &p_abort) override
            {
                if (cfg::cache_size != 0) {
                    insync (m_lock);

                    auto pos = find_if (m_cache.begin (), m_cache.end (), cache_slot_equal (p_archive, p_file));
                    if (pos != m_cache.end ()) {
                        p_out = new file_tempmem (pos->timestamp);
                        try {
                            file::g_transfer_file (pos->cache_file, p_out, p_abort);
                            p_out->seek (0, p_abort);
                            return true;
                        } catch (const std::exception &e) {
                            p_out.release ();
                            error_log () << "disk cache fetch exception:" << e.what ();
                            throw;
                        }
                    }
                }
                return false;
            }

            void store (const char *p_archive, const char *p_file, const file_ptr &p_in, abort_callback &p_abort) override
            {
                if (cfg::cache_size != 0) {
                    insync (m_lock);

                    // setup cache on first call
                    if (m_cache.empty ()) m_cache.resize (cfg::cache_size);

                    // do not store same file twice
                    auto pos = find_if (m_cache.begin (), m_cache.end (), cache_slot_equal (p_archive, p_file));
                    if (pos == m_cache.end ()) {
                        auto slot_index = m_next_slot % cfg::cache_size;
                        cache_slot &slot = m_cache[slot_index];

                        m_next_slot = (m_next_slot + 1) % cfg::cache_size;

                        try {
                            if (slot.cache_file.is_empty ())
                                open_cache_file (slot, p_file, p_abort);

                            file::g_transfer_file (p_in, slot.cache_file, p_abort);
                            p_in->seek (0, p_abort);

                            slot.timestamp = p_in->get_timestamp (p_abort);
                        } catch (const std::exception &e) {
                            error_log () << "disk cache store exception:" << e.what ();
                            slot.cache_file.release ();
                            return;
                        }

                        slot.archive = p_archive;
                        slot.file_in_archive = p_file;
                    }
                }
            }

            void restart () override
            {
                if (cfg::cache_size != m_cache.size ()) {
                    insync (m_lock);
                    m_cache.resize (cfg::cache_size);
                }
            }

            // helpers
            inline void open_cache_file (cache_slot &slot, const char *magic, abort_callback &p_abort)
            {
                if (cfg::cache_location_custom) {
                    console::formatter () << "cache location:" << cfg::cache_location;
                    generate_temp_location_for_file (slot.cache_path, cfg::cache_location, "tmp", magic);
                    filesystem::g_open_write_new (slot.cache_file, slot.cache_path, p_abort);
                }
                else
                    filesystem::g_open_temp (slot.cache_file, p_abort);
            }

        public:
            ~manager_impl ()
            {
                if (cfg::cache_location_custom) {
                    for (int n = m_cache.size () - 1; n >= 0; n--) {
                        if (m_cache[n].cache_file.is_valid ()) {
                            m_cache[n].cache_file.release ();
                            uDeleteFile (m_cache[n].cache_path);
                        }
                    }
                }
            }
        };
        static service_factory_single_t<manager_impl> g_factory;


        void fetch_or_unpack (const char *p_archive, const char *p_file, file_ptr &p_out, abort_callback &p_abort)
        {
            if (!static_api_ptr_t<disk_cache::manager>()->fetch (p_archive, p_file, p_out, p_abort))
                fetch_or_unpack (unpack_7z::archive (p_archive, p_abort), p_file, p_out, p_abort);
        }

        void fetch_or_unpack (const unpack_7z::archive &p_archive, const char *p_file, file_ptr &p_out, abort_callback &p_abort)
        {
            static_api_ptr_t<disk_cache::manager> api;
            if (!api->fetch (p_archive.get_path (), p_file, p_out, p_abort)) {
                p_out = new file_tempmem (p_archive.get_stats (p_file).m_timestamp);
                p_archive.extract_file (p_file, p_out, p_abort);
                api->store (p_archive.get_path (), p_file, p_out, p_abort);
            }
        }
    }
}