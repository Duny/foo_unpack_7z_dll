#pragma once

class C7zArchive
{
public:
	struct file_desc_t {
		t_filestats  m_stats;
		pfc::string8 m_path;
	};
	typedef pfc::array_t<file_desc_t> t_arch_items;

	~C7zArchive () { Close (); }

    void Open (file_ptr const &, abort_callback &);
    void Close () { if (m_archive) m_archive->Close (); }

	const t_arch_items & items () const { return m_items; }

    void GetFileReader (t_size i, file_ptr &p_out, abort_callback &p_abort);
	void GetFileReader (const pfc::string8 &path, file_ptr &p_out, abort_callback &p_abort);

private:
	CMyComPtr<IInArchive> m_archive;
	t_filetimestamp m_timestamp;
	t_arch_items m_items;

    void GetFileList ();
};