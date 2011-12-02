#include "stdafx.h"

#include "cache_system.h"
#include "archive.h"
#include "archive_info_cache.h"

namespace unpack_7z
{
    class file_cache
    {
        // information about location of file being cached
        struct archive_file_info
        {
            pfc::string8 m_archive; // archive full path
            pfc::string8 m_file; // path from inside the archive, e.i. "some folder\somefile.mp3"
            t_filestats  m_stats;

            archive_file_info () {}
            // constructor used for entry compare when searching by the list_t::find () function
            archive_file_info (const char *p_archive, const char *p_file) : m_archive (p_archive), m_file (p_file) {}

            inline void set (const char *p_archive, const char *p_file, const t_filestats &p_stats)
            {
                m_archive = p_archive;
                m_file = p_file;
                m_stats = p_stats;
            }

            inline bool operator== (const archive_file_info &other) const
            { return !other.m_archive.is_empty () && stricmp_utf8 (other.m_archive, m_archive) == 0 && stricmp_utf8 (other.m_file, m_file) == 0; }
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

        critical_section m_lock;
        pfc::list_t<cache_entry> m_data;
        t_size m_next_entry;
    public:

        bool fetch (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort)
        {
            if (cfg::cache_size != 0) {
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
                    cache_entry &entry = m_data[m_next_entry];
                    try {
                        if (!entry.is_valid ()) // initialize entry on demand
                            entry.init (p_file, p_abort);

                        file::g_transfer_file (p_in, entry.m_file, p_abort);
                        p_in->seek (0, p_abort);

                        entry.m_archive_file_info.set (p_archive, p_file, p_in->get_stats (p_abort));
                    } catch (const std::exception &e) {
                        error_log () << "disk cache store exception:" << e.what ();
                        entry.free ();
                    }
                    m_next_entry = (m_next_entry + 1) % cfg::cache_size;
                }
            }
        }

        inline void print_stats () const
        {
        }
    };


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
            // optimization for multiple listings of the same archive
            if (!p_want_readers) {
                pfc::list_t<archive::file_info> files;
                m_archive_info_cache.get_file_list (p_archive, files, p_out);

                file_ptr dummy;
                for (t_size i = 0, max = files.get_size (); i < max; i++)
                    if (!p_out.on_entry (owner, files[i].m_unpack_path, files[i].m_stats, dummy))
                        break;
            }
            else {
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