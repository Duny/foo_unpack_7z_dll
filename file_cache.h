#ifndef _FOO_UNPACK_7Z_DLL_FILE_CACHE_H_
#define _FOO_UNPACK_7Z_DLL_FILE_CACHE_H_

namespace unpack_7z
{
    class file_cache : public cfg_var
    {
        struct entry
        {
            pfc::string8 m_archive;
            t_filestats  m_stats; // Stats of file from archive
            pfc::string8 m_path; // Physical path of cache entry file. Empty if default system temp folder was used
            file_ptr     m_file; // Cache entry file handle

            ~entry () { free (); }

            void init (const char *p_archive, const char *p_file, const t_filestats &p_stats, abort_callback &p_abort)
            {
                free ();
                if (cfg::use_sys_tmp_for_cache)
                    filesystem::g_open_temp (m_file, p_abort);
                else {
                    generate_temp_location_for_file (m_path, cfg::custom_cache_path, "tmp", pfc::string_formatter () << ReadTimeStampCounter ());
                    filesystem::g_open_write_new (m_file, m_path, p_abort);
                }
                m_archive = p_archive;
                m_stats = p_stats;
            }

            void free ()
            {
                if (m_file.is_valid ()) m_file.release ();
                // Delete file if not system temp (tmp files are deleted by OS)
                if (!m_path.is_empty ()) { uDeleteFile (m_path); m_path.reset (); }
            }
        };

        typedef pfc::map_t<t_uint64, entry> t_hash_map;
        typedef t_hash_map::t_key t_key; typedef t_hash_map::t_value t_value;


        // cfg_var overrides
        void get_data_raw (stream_writer *p_stream, abort_callback &p_abort) // Called for storing configuration
        {
            insync (m_lock);

            stream_writer_formatter<> out (*p_stream, p_abort);
            if (cfg::keep_cache_at_exit && cfg::use_sys_tmp_for_cache != true) {
                out << m_data.get_count ();
                m_data.enumerate ([&](const t_key &key, const t_value &val) {
                    if (!val.m_path.is_empty ())
                        out << key << val.m_archive << val.m_stats << val.m_path;
                });
            }
            else
                out << static_cast<decltype(m_data.get_count ())>(0);
        }

	    void set_data_raw (stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort) // Called on startup for reading from disk
        {
            insync (m_lock);

            stream_reader_formatter<> in (*p_stream, p_abort);
            decltype(m_data.get_count ()) count; in >> count;
            while (count --> 0) {
                t_key key; in >> key;
                t_value & val = m_data.find_or_add (key);
                in >> val.m_archive >> val.m_stats >> val.m_path;
                try {
                    filesystem::g_open (val.m_file, val.m_path, filesystem::open_mode_write_existing, p_abort);
                    if (!val.m_file.is_valid ()) throw 0; // Signalize that item must be deleted from data
                    m_size += val.m_stats.m_size;
                } catch (...) { m_data.remove (key); }
            }
	    }

        // Helpers
        t_key make_key (const char *p_archive, const char *p_file) const 
        { 
            return hash (p_archive, hash (p_file));
        }

        t_value * alloc_entry (const char *p_archive, const char *p_file, t_filesize file_size, bool & is_new)
        {
            t_filesize max_size = cfg::file_cache_max; max_size <<= 20;
            if (file_size > max_size) return nullptr;

            if (m_size > max_size) free_space (m_size - max_size);
            if (m_size + file_size > max_size) free_space (file_size - (max_size - m_size));
            return &m_data.find_or_add_ex (make_key (p_archive, p_file), is_new);
        }

        void free_space (t_filesize size) // In bytes
        {
            typedef tuple<t_key, t_filesize> cache_file;
            auto cache_file_comparator = [] (const cache_file &left, const cache_file &right) -> int { return pfc::compare_t (left.get<1>(), right.get<1>()); };
                
            // Sort files by size (asc)
            pfc::list_t<cache_file> files;
            m_data.enumerate ([&](const t_key &key, const t_value &value) { files.add_item (make_tuple (key, value.m_stats.m_size)); });
            files.sort_t (cache_file_comparator);

            t_size i = 0, n = files.get_size ();
            for (; i < n; i++) // Search for the smallest possible file to delete from the beginning
                if (files[i].get<1> () >= size) break;

            t_filesize freed = 0;
            if (i != n) { // File was found
                m_data.remove (files[i].get<0>());
                freed = files[i].get<1> ();
            }
            else { // There is no such file in cache to accommodate our size
                // Very ugly solution:
                // Delete each file starting from the end, until we reach desired size
                while ((n --> 0) && (freed < size)) {
                    m_data.remove (files[n].get<0>());                    
                    freed += files[n].get<1> ();
                }
            }
            m_size -= freed;
        }
        
        // Member variables
        t_hash_map               m_data; // Key is made of hash () from canonical path to archive+file path in archive
        t_filesize               m_size; // Size in bytes of used space
        mutable critical_section m_lock;

    public:
        file_cache () : cfg_var (create_guid (0x99e8aad3, 0xa107, 0x4495, 0x84, 0xf4, 0x08, 0x43, 0xc9, 0x88, 0x63, 0xa1)) {}
        ~file_cache ()
        { //  Hack to prevent files from being deleted during entry::~entry ()
            m_data.enumerate ([&](const t_key&, t_value & val) { if (cfg::keep_cache_at_exit && !cfg::use_sys_tmp_for_cache) val.m_path.reset (); });
        }
        
        bool fetch (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort) const
        {
            insync (m_lock);

            const t_value *e;
            if (m_data.query_ptr (make_key (p_archive, p_file), e)) {
                p_out = new file_tempmem (e->m_stats);
                try {
                    file::g_transfer_file (e->m_file, p_out, p_abort);
                    p_out->seek (0, p_abort);
                    return true;
                } catch (const std::exception &e) {
                    error_log () << "Disk cache fetch exception \"" << e.what () << "\"";
                    p_out.release ();
                }
            }
            return false;
        }
        
        void store (const file_ptr &p_in, const char *p_archive, const char *p_file, abort_callback &p_abort)
        {
            insync (m_lock);            

            auto stats = p_in->get_stats (p_abort);
            bool new_item;
            t_value *e = alloc_entry (p_archive, p_file, stats.m_size, new_item);
            if (e && new_item) {
                try {
                    e->init (p_archive, p_file, stats, p_abort);
                    file::g_transfer_file (p_in, e->m_file, p_abort);
                    p_in->seek (0, p_abort);
                    m_size += stats.m_size;
                } catch (const std::exception &ex) {
                    error_log () << "Disk cache store exception \"" << ex.what () << "\"";
                    m_data.remove (make_key (p_archive, p_file));
                }
            }
        }

        void set_max_size (t_uint32 new_size) // In Mb
        {
            insync (m_lock);

            cfg::file_cache_max = new_size;

            t_filesize new_sizeB = new_size; new_sizeB <<= 20;
            if (m_size > new_sizeB) free_space (m_size - new_sizeB);
        } 

        void clear_by_archive_name (const pfc::string_base & p_archive)
        {
            t_hash_map new_data;
            insync (m_lock);
            for (auto walk = m_data.first (); walk.is_valid (); ++walk)
                if (walk->m_value.m_archive == p_archive)
                    m_data.remove (walk);
        }

        void clear ()
        {
            insync (m_lock);
            m_data.remove_all ();
            m_size = 0;
        }

        void print_stats () const
        {
            t_filesize max_size = cfg::file_cache_max; max_size <<= 20;
            console::formatter () << "File cache disk usage: " << pfc::format_file_size_short (m_size)
                << " in " << m_data.get_count ()  << " files out of " << pfc::format_file_size_short (max_size) << " max\n";
        }
    };
}
#endif