#ifndef _ARCHIVE_7Z_H_
#define _ARCHIVE_7Z_H_

struct file_in_archive {
	t_filestats  m_stats;
	pfc::string8 m_path;
};

class archive_7z
{
public:
	archive_7z () {}
	archive_7z (const char *p_archive, abort_callback &p_abort) { open (p_archive, p_abort); }

	~archive_7z () { close (); }

    void open (const file_ptr &p_file, abort_callback &p_abort);
	void open (const char *p_file, abort_callback &p_abort);
    void close ();

	const t_filestats& get_stats (const char *p_file);

	template <class Func> void list (Func f) {
        for (size_t i = 0, n = m_items.size (); i < n; i++)
		    if (!f (m_items[i]))
			    break;
    }

	void get_reader (const pfc::string8 &p_file, file_ptr &p_out, abort_callback &p_abort);

private:
	archive_7z (const archive_7z &);
	archive_7z& operator= (const archive_7z &);

	CMyComPtr<IInArchive> m_archive;
	t_filetimestamp m_timestamp;

    typedef std::vector<file_in_archive> archive_items;
	archive_items m_items;

	void get_reader_internal (t_size i, file_ptr &p_out, abort_callback &p_abort);
    void list_archive ();
};

#endif