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

    class archive
    {
    public:
	    archive () {}
	    archive (const char *p_archive, abort_callback &p_abort) { open (p_archive, p_abort); }
	    ~archive () { close (); }

        void open (const file_ptr &p_file, abort_callback &p_abort);
	    void open (const char *p_file, abort_callback &p_abort);
        void close ();

        const pfc::string8& get_path () const { return m_path; }

	    const t_filestats& get_stats (const char *p_file) const;

	    template <class Func> void list (Func f) const
        {
            for (decltype(m_items.size ()) n = m_items.size (), i = 0; i < n; i++)
                if (!f (m_items[i].m_path, m_items[i].m_stats))
                    break;
        }

	    void extract_file (const char *p_file, const file_ptr &p_out, abort_callback &p_abort) const;

    private:
	    archive (const archive &);
	    archive& operator= (const archive &);

        void extract_file (t_size i, const file_ptr &p_out, abort_callback &p_abort) const;
        void list_archive ();

        CMyComPtr<IInStream> m_stream;
	    CMyComPtr<IInArchive> m_archive;

        pfc::string8 m_path;
	    t_filetimestamp m_timestamp; // timestamp of archive itself

        struct file_in_archive
        {
            t_filestats  m_stats;
            pfc::string8 m_path;
        };
	    std::vector<file_in_archive> m_items;
    };
}
#endif