#ifndef _FOO_UNPACK_7Z_DLL_ARCHIVE_H_
#define _FOO_UNPACK_7Z_DLL_ARCHIVE_H_

namespace unpack_7z
{
    struct file_in_archive
    {
	    t_filestats  m_stats;
	    pfc::string8 m_path;
    };

    PFC_DECLARE_EXCEPTION (exception_arch_7z, pfc::exception, COMPONENT_NAME);
    PFC_DECLARE_EXCEPTION (exception_arch_close, exception_arch_7z, COMPONENT_NAME ": couldn't close archive");
    PFC_DECLARE_EXCEPTION (exception_arch_open, exception_arch_7z, COMPONENT_NAME ": archive open error");
    PFC_DECLARE_EXCEPTION (exception_arch_file_not_found, exception_arch_7z, COMPONENT_NAME ": file not found in archive");

    class archive
    {
    public:
	    archive () {}
	    archive (const char *p_archive, abort_callback &p_abort) { open (p_archive, p_abort); }
	    ~archive () { close (); }

        void open (const file_ptr &p_file, abort_callback &p_abort);
	    void open (const char *p_file, abort_callback &p_abort);
        void close ();

	    const t_filestats& get_stats (const char *p_file) const;

	    template <class Func> void list (Func f) const
        {
            for (decltype(m_items.size ()) n = m_items.size (), i = 0; i < n; i++)
                if (!f (m_items[i]))
                    break;
        }

	    void get_reader (const char *p_file, file_ptr &p_out, abort_callback &p_abort);

    private:
	    archive (const archive &);
	    archive& operator= (const archive &);

        CMyComPtr<IInStream> m_stream;
	    CMyComPtr<IInArchive> m_archive;

        pfc::string8 m_path;
	    t_filetimestamp m_timestamp;

	    std::vector<file_in_archive> m_items;

	    void get_reader_internal (t_size i, file_ptr &p_out, abort_callback &p_abort);
        void list_archive ();
    };
}
#endif