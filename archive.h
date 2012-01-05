#ifndef _FOO_UNPACK_7Z_DLL_ARCHIVE_H_
#define _FOO_UNPACK_7Z_DLL_ARCHIVE_H_

namespace unpack_7z
{
    PFC_DECLARE_EXCEPTION (exception_arch_7z, pfc::exception, COMPONENT_NAME);
    PFC_DECLARE_EXCEPTION (exception_arch_close, exception_arch_7z, "couldn't close archive");
    PFC_DECLARE_EXCEPTION (exception_arch_open, exception_arch_7z, "archive open error");
    PFC_DECLARE_EXCEPTION (exception_arch_extract_error, exception_arch_7z, "error extracting file");
    PFC_DECLARE_EXCEPTION (exception_arch_file_not_found, exception_arch_7z, "file not found in archive");
    PFC_DECLARE_EXCEPTION (exception_arch_num_items_error, exception_arch_7z, "couldn't get number of items in the archive");

    class archive : boost::noncopyable
    {
    public:
	    archive () {}
	    archive (const char *p_archive, abort_callback &p_abort, bool read_file_list = true) { open (p_archive, p_abort, read_file_list); }
        archive (const char *p_archive, const file_ptr &p_file, abort_callback &p_abort, bool read_file_list = true) { open (p_archive, p_file, p_abort, read_file_list); }
	    ~archive () { close (); }

        // If p_file is not valid, then p_archive is opened
        void open (const char *p_archive, const file_ptr &p_file, abort_callback &p_abort, bool read_file_list = true);
        void open (const char *p_archive, abort_callback &p_abort, bool read_file_list = true) { open (p_archive, file_ptr (), p_abort, read_file_list); }
        void close ();

        t_filestats get_stats () const { return m_stats; }
        const char *get_path () const { return m_path.get_ptr (); }


        struct file_info
        {
            pfc::string8 m_path; // path inside archive (dir\file.mp3)
            pfc::string8 m_unpack_path;
            t_filestats  m_stats;

            file_info () {}
            // constructor used for compare when searching by the list_t::find () function
            file_info (const char *p_path) : m_path (p_path) {}

            bool operator== (const file_info &other) const { return stricmp_utf8 (other.m_path, m_path) == 0; }
        };
        typedef pfc::list_t<archive::file_info> file_list;
        typedef file_list const & file_list_cref;

        file_list_cref get_list () const { return m_items; }

        void get_file_list ();

	    void extract_file (const file_ptr &p_out, const char *p_file, abort_callback &p_abort) const;
        void extract_file (const file_ptr &p_out, t_size index, abort_callback &p_abort) const;

    private:
        CMyComPtr<IInStream> m_stream;
	    CMyComPtr<IInArchive> m_archive;

        pfc::string8 m_path; // archive path

        // for some reason foobar uses timestamp of archive itself instead of timestamps of files from archive
        // each extracted file must have timestamp of m_timestamp
	    t_filestats m_stats;

        // archive files 
	    file_list m_items;
    };
}

FB2K_STREAM_READER_OVERLOAD(unpack_7z::archive::file_info) { return stream >> value.m_path >> value.m_unpack_path >> value.m_stats.m_size >> value.m_stats.m_timestamp; }
FB2K_STREAM_WRITER_OVERLOAD(unpack_7z::archive::file_info) { return stream << value.m_path << value.m_unpack_path << value.m_stats.m_size << value.m_stats.m_timestamp; }

FB2K_STREAM_READER_OVERLOAD(unpack_7z::archive::file_list) { t_size n; stream >> n; unpack_7z::archive::file_info info; while (n --> 0) { stream >> info; value.add_item (info); } return stream; }
FB2K_STREAM_WRITER_OVERLOAD(unpack_7z::archive::file_list) { auto n = value.get_size (); stream << n; for (t_size i = 0; i < n; i++) stream << value[i]; return stream; }

#endif