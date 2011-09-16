#ifndef _ARCHIVE_7Z_H_
#define _ARCHIVE_7Z_H_

#include "Common\MyCom.h"
#include "7zip\Archive\IArchive.h"

struct file_in_archive {
	t_filestats  m_stats;
	pfc::string8 m_path;
};
typedef std::vector<file_in_archive> archive_items;

class item_callback
{
public:
	virtual bool on_item (const file_in_archive &p_file) = 0;
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

	void list (item_callback &p_callback);

	const archive_items & items () const { return m_items; }

	void get_reader (const pfc::string8 &p_file, file_ptr &p_out, abort_callback &p_abort);

private:
	archive_7z (const archive_7z &);
	archive_7z& operator= (const archive_7z &);

	CMyComPtr<IInArchive> m_archive;
	t_filetimestamp m_timestamp;
	archive_items m_items;

	void get_reader (t_size i, file_ptr &p_out, abort_callback &p_abort);
    void list_archive ();
};

#endif