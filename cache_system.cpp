#include "stdafx.h"

FB2K_STREAM_READER_OVERLOAD(unpack_7z::archive::file_info) { return stream >> value.m_file_path >> value.m_unpack_path >> value.m_stats.m_size >> value.m_stats.m_timestamp; }
FB2K_STREAM_WRITER_OVERLOAD(unpack_7z::archive::file_info) { return stream << value.m_file_path << value.m_unpack_path << value.m_stats.m_size << value.m_stats.m_timestamp; }

FB2K_STREAM_READER_OVERLOAD(unpack_7z::archive::file_list) { t_size n; stream >> n; unpack_7z::archive::file_info info; while (n --> 0) { stream >> info; value.add_item (info); } return stream; }
FB2K_STREAM_WRITER_OVERLOAD(unpack_7z::archive::file_list) { auto n = value.get_size (); stream << n; for (t_size i = 0; i < n; i++) stream << value[i]; return stream; }


namespace unpack_7z
{
    class archive_info_cache : public cfg_var
    {
        struct entry_t
        {
            t_filetimestamp    m_timestamp; // archive timestamp
            archive::file_list m_files; // archive contents
        };

        // cfg_var overrides
        void get_data_raw (stream_writer *p_stream, abort_callback &p_abort) // called on shutdown for storing to disk
        {
            stream_writer_formatter<> out (*p_stream, p_abort);
            auto max_items = min (cfg::archive_history_max, m_data_size); // check max data size and truncate if needed
            out << max_items;
            for (pfc::map_t<GUID, entry_t>::const_iterator walk = m_data.first (); walk.is_valid () && max_items; ++walk, --max_items)
                out << walk->m_key << walk->m_value.m_timestamp << walk->m_value.m_files;
        }

	    void set_data_raw (stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort) // called on startup for reading from disk
        {
            stream_reader_formatter<> in (*p_stream, p_abort);
            t_uint32 count; in >> count;
            m_data_size = count;
            while (count --> 0) {
                GUID key; in >> key;
                entry_t &e = m_data.find_or_add (key);
                in >> e.m_timestamp >> e.m_files;
            }
	    }

        // helpers
        inline entry_t & find_or_add (const char *p_archive, abort_callback &p_abort)
        {
            t_filestats stats;
            bool is_new, dummy;

            GUID key = key_from_string (p_archive);
            entry_t & e = m_data.find_or_add_ex (key, is_new);
            filesystem::g_get_stats (p_archive, stats, dummy, p_abort);

            if (!is_new && e.m_timestamp != stats.m_timestamp)
                is_new = true;

            if (is_new) {
                if (m_data_size + 1 > cfg::archive_history_max)
                    remove_random_item ();
                
                entry_t & new_entry = m_data.find_or_add (key);
                unpack_7z::archive a (p_archive, p_abort);
                new_entry.m_files = a.get_info ();
                new_entry.m_timestamp = stats.m_timestamp;
                m_data_size++;
                return new_entry;
            }
            else
                return e;
        }

        inline void remove_random_item ()
        {
            if (m_data_size) {
                auto n = ReadTimeStampCounter () % m_data_size; // choose witch item to remove
                auto walk = m_data.first ();
                while (n --> 0) walk++; // go to it logical position
                m_data.remove (walk); // and delete it
                m_data_size--;
            }
        }

        inline GUID key_from_string (const char *p_str) const { return GUID_from_text_md5 (string_lower (p_str)); }


        // member variables
        pfc::map_t<GUID, entry_t> m_data; // GUID is made of md5 from canonical path to archive
        t_uint32                  m_data_size;
        mutable critical_section          m_lock; // synchronization for accessing m_data
            
    public:
        archive_info_cache () : cfg_var (guid_inline<0x8D96A7C4, 0x9855, 0x4076, 0xB9, 0xD7, 0x88, 0x82, 0x23, 0x50, 0xBF, 0xCA>::guid) {}

        inline t_filestats get_file_stats (const char *p_archive, const char *p_file, abort_callback &p_abort)
        {
            insync (m_lock);
            entry_t &e = find_or_add (p_archive, p_abort);
            auto n = e.m_files.find_item (p_file);
            if (n == pfc_infinite) throw exception_arch_file_not_found ();
            return e.m_files[n].m_stats;    
        }

        inline void get_file_list (const char *p_archive, archive::file_list &p_out, abort_callback &p_abort)
        {
            insync (m_lock);
            p_out = find_or_add (p_archive, p_abort).m_files;
        }

        inline bool query_file_list (const char *p_archive, archive::file_list &p_out)
        {
            insync (m_lock);
            entry_t e;
            bool res = m_data.query (key_from_string (p_archive), e);
            if (res) p_out = e.m_files;
            return res;
        }

        inline void add_entry (const unpack_7z::archive &p_archive)
        {
            insync (m_lock);

            bool is_new;
            entry_t & e = m_data.find_or_add_ex (key_from_string (p_archive.get_path ()), is_new);
            e.m_files = p_archive.get_info ();
            e.m_timestamp = p_archive.get_timestamp ();
            if (is_new) {
                if (m_data_size + 1 > cfg::archive_history_max)
                    remove_random_item ();
                else
                    m_data_size++;
            }
        }

        inline void print_stats () const
        {
            insync (m_lock);
            console::formatter() << "Archive info cache size: " << m_data.get_count ();;
        }
    };

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