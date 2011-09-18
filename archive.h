#ifndef _ARCHIVE_7Z_H_
#define _ARCHIVE_7Z_H_

namespace unpack_7z
{
    struct file_in_archive
    {
	    t_filestats  m_stats;
	    pfc::string8 m_path;
    };

    class archive
    {
    public:
	    archive () {}
	    archive (const char *p_archive, abort_callback &p_abort) { open (p_archive, p_abort); }
	    ~archive () { close (); }

        void open (const file_ptr &p_file, abort_callback &p_abort);
	    void open (const char *p_file, abort_callback &p_abort);
        void close ();

	    const t_filestats& get_stats (const char *p_file);

	    template <class Func> void list (Func f) { std::find_if (m_items.begin (), m_items.end (), f); }

	    void get_reader (const pfc::string8 &p_file, file_ptr &p_out, abort_callback &p_abort);

    private:
	    archive (const archive &);
	    archive& operator= (const archive &);

	    CMyComPtr<IInArchive> m_archive;
	    t_filetimestamp m_timestamp;

	    std::vector<file_in_archive> m_items;

	    void get_reader_internal (t_size i, file_ptr &p_out, abort_callback &p_abort);
        void list_archive ();
    };
}
#endif