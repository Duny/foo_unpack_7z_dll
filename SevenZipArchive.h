class SevenZipArchive
{
public:
    SevenZipArchive ();
   ~SevenZipArchive ();

    bool Open (file_ptr const &, abort_callback &);
    void Close ();

	t_size NumFiles () const;

	const pfc::string8& GetFilePath (t_size i) const;
	const t_filestats& GetFileStats (t_size i) const;
	const t_filestats& GetFileStats (const pfc::string8 &path) const;
	void GetFileReader (t_size i, file_ptr &p_out, abort_callback &p_abort);
	void GetFileReader (const pfc::string8 &path, file_ptr &p_out, abort_callback &p_abort);

private:
	CMyComPtr<IInArchive> m_archive;

	struct ArchiveFileDesc {
		t_filestats  m_stats;
		pfc::string8 m_path;
	};
	pfc::array_t<ArchiveFileDesc> m_file_list;

	t_filetimestamp m_archive_tstamp;

    bool GetFileList ();
};