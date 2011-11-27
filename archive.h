#ifndef _FOO_UNPACK_7Z_DLL_ARCHIVE_H_
#define _FOO_UNPACK_7Z_DLL_ARCHIVE_H_

namespace unpack_7z
{
    PFC_DECLARE_EXCEPTION (exception_arch_7z, pfc::exception, COMPONENT_NAME);
    PFC_DECLARE_EXCEPTION (exception_arch_close, exception_arch_7z, COMPONENT_NAME ": couldn't close archive");
    PFC_DECLARE_EXCEPTION (exception_arch_open, exception_arch_7z, COMPONENT_NAME ": archive open error");
    PFC_DECLARE_EXCEPTION (exception_arch_extract_error, exception_arch_7z, COMPONENT_NAME ": error extracting file");
    PFC_DECLARE_EXCEPTION (exception_arch_file_not_found, exception_arch_7z, COMPONENT_NAME ": file not found in archive");
    PFC_DECLARE_EXCEPTION (exception_arch_num_items_error, exception_arch_7z, COMPONENT_NAME ": couldn't get number of items in the archive");

    class archive : boost::noncopyable
    {
    public:
	    archive () {}
	    archive (const char *p_archive, abort_callback &p_abort, bool read_file_list = true) { open (p_archive, p_abort, read_file_list); }
	    ~archive () { close (); }

        void open (const file_ptr &p_file, abort_callback &p_abort, bool read_file_list = true);
	    void open (const char *p_file, abort_callback &p_abort, bool read_file_list = true);
        void close ();

        inline const pfc::string_base & get_path () const { return m_path; }


        struct file_info
        {
            pfc::string8 m_file_path, m_unpack_path;
            t_filestats  m_stats;

            file_info () {}
            // constructor used for compare when searching by the list_t::find () function
            file_info (const char *p_path) : m_file_path (p_path) {}

            inline bool operator== (const file_info &other) const { return stricmp_utf8 (other.m_file_path, m_file_path) == 0; }
        };


	    inline const t_filestats & get_stats (const char *p_file) const
        {
            auto n = m_items.find_item (file_info (p_file));
            if (n != pfc_infinite)
                return m_items[n].m_stats;
            else
                throw exception_arch_file_not_found ();
        }

        inline const pfc::list_base_const_t<archive::file_info> & get_info () const { return m_items; }

	    void extract_file (const file_ptr &p_out, const char *p_file, abort_callback &p_abort) const;
        
    private:
        void extract_file (const file_ptr &p_out, t_size i, abort_callback &p_abort) const;
        void get_file_list ();

        CMyComPtr<IInStream> m_stream;
	    CMyComPtr<IInArchive> m_archive;

        pfc::string8 m_path; // archive path

        // for some reasons foobar uses timestamp of archive itself instead of timestamps of files from archive
        // each extracted file must have timestamp of m_timestamp
	    t_filetimestamp m_timestamp; // archive timestamp

        // archive files 
	    pfc::list_t<file_info> m_items;
    };
}
#endif