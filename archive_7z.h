#ifndef _ARCHIVE_7Z_H_
#define _ARCHIVE_7Z_H_

#include "Common\MyCom.h"
#include "7zip\Archive\IArchive.h"

class archive_7z
{
	friend class archive_7z_cached;
public:
	struct file_desc_t {
		t_filestats  m_stats;
		pfc::string8 m_path;
	};
	typedef pfc::array_t<file_desc_t> t_arch_items;

	~archive_7z () { Close (); }

    void Open (const char*, file_ptr const &, abort_callback &);
    void Close () { if (m_archive) m_archive->Close (); }

	const t_arch_items & items () const { return m_items; }

    void GetFileReader (t_size i, file_ptr &p_out, abort_callback &p_abort);
	void GetFileReader (const pfc::string8 &path, file_ptr &p_out, abort_callback &p_abort);

private:
	CMyComPtr<IInArchive> m_archive;
	pfc::string8_fast m_path;
	t_filetimestamp m_timestamp;
	t_arch_items m_items;

    void GetFileList ();
};

class archive_7z_cached
{
	enum { cache_size = 10 };

	static archive_7z m_cache[cache_size];
	static t_size m_cache_pos;
	static critical_section m_section;

	c_insync m_sync;
public:
	archive_7z_cached () : m_sync (m_section) {}

	const archive_7z & Open (const char*, file_ptr const &, abort_callback &);
};

#endif