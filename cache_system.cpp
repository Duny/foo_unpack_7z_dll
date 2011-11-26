#include "stdafx.h"

namespace unpack_7z
{
    class file_cache
    {
        // information about location of file being cached
        struct archive_file_info
        {
            pfc::string8 m_archive; // archive full path
            pfc::string8 m_file; // path from inside the archive, e.i. "some folder\somefile.mp3"
            t_filetimestamp m_timestamp;

            archive_file_info () {}
            archive_file_info (const char *p_archive, const char *p_file) : m_archive (p_archive), m_file (p_file) {}

            inline void set (const char *p_archive, const char *p_file, t_filetimestamp p_timestamp)
            {
                m_archive = p_archive;
                m_file = p_file;
                m_timestamp = p_timestamp;
            }

            inline bool operator== (const archive_file_info &other) const
            { return stricmp_utf8 (other.m_archive, m_archive) == 0 && stricmp_utf8 (other.m_file, m_file) == 0; }
        };

        struct cache_entry
        {
            archive_file_info m_archive_file_info;

            // Physical path of cache entry file. Empty if default system temp folder was used for cache
            pfc::string8 m_path;
            file_ptr m_file; // cache entry file

            ~cache_entry () { free (); }

            inline void init (const char *p_file, abort_callback &p_abort)
            {
                if (cfg::cache_location_custom) {
                    generate_temp_location_for_file (m_path, cfg::cache_location, "tmp", p_file);
                    filesystem::g_open_write_new (m_file, m_path, p_abort);
                    /*pfc::string8_fast full_path;
                    filesystem::g_get_canonical_path (m_path, full_path);
                    console::formatter () << full_path;
                    filesystem::g_open (m_file, pfc::string_formatter () << "tempfile://D:\\Downloads\\123.tmp", filesystem::open_mode_write_new, p_abort);*/
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

            inline bool operator== (const archive_file_info &other) const { return is_valid () && m_archive_file_info == other; }
        };


        bool fetch (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort) const
        {
            if (cfg::cache_size != 0) {
                insync (m_lock);

                auto n = m_data.find_item (archive_file_info (p_archive, p_file));
                if (n != pfc_infinite) {
                    p_out = new file_tempmem (m_data[n].m_archive_file_info.m_timestamp);
                    try {
                        file::g_transfer_file (m_data[n].m_file, p_out, p_abort);
                        p_out->seek (0, p_abort);
                        return true;
                    } catch (const std::exception &e) {
                        error_log () << "disk cache fetch exception:" << e.what ();
                        p_out.release ();
                    }
                }
            }
            return false;
        }

        void store (const file_ptr &p_in, const char *p_archive, const char *p_file, abort_callback &p_abort)
        {
            if (cfg::cache_size != 0) {
                insync (m_lock);

                // setup cache on first call
                if (!m_data.get_size ()) m_data.set_size (cfg::cache_size);

                // do not store the same file twice
                if (m_data.find_item (archive_file_info (p_archive, p_file)) == pfc_infinite) {
                    auto entry_index = m_next_entry % cfg::cache_size;
                    cache_entry &entry = m_data[entry_index];

                    m_next_entry = (m_next_entry + 1) % cfg::cache_size;

                    try {
                        if (!entry.is_valid ()) // initialize entry on demand
                            entry.init (p_file, p_abort);

                        file::g_transfer_file (p_in, entry.m_file, p_abort);
                        p_in->seek (0, p_abort);

                        entry.m_archive_file_info.set (p_archive, p_file, p_in->get_timestamp (p_abort));
                    } catch (const std::exception &e) {
                        error_log () << "disk cache store exception:" << e.what ();
                        entry.free ();
                    }
                }
            }
        }

        mutable critical_section m_lock;
        pfc::list_t<cache_entry> m_data;
        t_size m_next_entry;
    public:

        inline void fetch_or_unpack (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort)
        {
            if (!fetch (p_out, p_archive, p_file, p_abort))
                fetch_or_unpack (p_out, unpack_7z::archive (p_archive, p_abort), p_file, p_abort);
        }

        inline void fetch_or_unpack (file_ptr &p_out, const unpack_7z::archive &p_archive, const char *p_file, abort_callback &p_abort)
        {
            if (!fetch (p_out, p_archive.get_path (), p_file, p_abort)) {
                p_out = new file_tempmem (p_archive.get_stats (p_file).m_timestamp);
                p_archive.extract_file (p_file, p_out, p_abort);
                store (p_out, p_archive.get_path (), p_file, p_abort);
            }
        }
    };


    class cache_system_impl : public cache_system
    {
        // member variables
        file_cache m_file_cache;


        // cache_system overrides
        void extract (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort) override
        {
            m_file_cache.fetch_or_unpack (p_out, p_archive, p_file, p_abort);
        }
    };

    namespace { service_factory_single_t<cache_system_impl> g_factory; }
}