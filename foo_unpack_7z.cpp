#include "stdafx.h"

class SevenZipArchiveImpl : public archive_impl
{
protected:

	SevenZipArchiveImpl () {}
   ~SevenZipArchiveImpl () {} 

    virtual const char *get_archive_type () { return "7z"; }

    virtual bool supports_content_types () { return false; }

	virtual t_filestats get_stats_in_archive (const char *p_archive, const char *p_file, abort_callback &p_abort)
	{
		pfc::string_extension ext (p_archive);
		if (strcmp (ext, get_archive_type ()) != 0)
			throw exception_io_data ();
#ifdef _DEBUG 
        console::formatter () << "get_stats_in_archive " << p_archive << " " << p_file;
#endif
		file_ptr file;
		SevenZipArchive archive;

		filesystem::g_open (file, p_archive, filesystem::open_mode_read, p_abort);

		archive.Open (file, p_abort);
		return archive.GetFileStats (p_file);
	}

	virtual void open_archive (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort)
	{
		pfc::string_extension ext (p_archive);
		if (strcmp (ext, get_archive_type ()) != 0)
			throw exception_io_data ();
#ifdef _DEBUG
        console::formatter() << "open_archive " << p_archive << " " << p_file;
#endif
		file_ptr file;
		SevenZipArchive archive; 

		filesystem::g_open (file, p_archive, filesystem::open_mode_read, p_abort);

		archive.Open (file, p_abort);
		archive.GetFileReader (p_file, p_out, p_abort);
	}

public:
	virtual void archive_list (const char *path, const file_ptr &p_reader, archive_callback &p_out, bool p_want_readers)
	{
#ifdef _DEBUG
        console::formatter() << "archive_list " << path << "; want_readers=" << p_want_readers;
#endif
		pfc::string_extension ext (path);
		if (_stricmp (ext, get_archive_type ()) != 0)
			throw exception_io_data ();

		{
			t_filestats stats;
			bool is_writeable;

			filesystem::g_get_stats (path, stats, is_writeable, p_out);
			console::formatter() << "timestamp: " << stats.m_timestamp;
		}

		file_ptr p_file = p_reader;
		if (p_file.is_empty ())
			filesystem::g_open (p_file, path, filesystem::open_mode_read, p_out);

		SevenZipArchive archive;
		if (!archive.Open (p_file, p_out))
			throw exception_io_data ();

		pfc::string8 m_url;
		for (t_size i = 0, n = archive.NumFiles (); i < n; i++) {
			make_unpack_path (m_url, path, archive.GetFilePath (i));
			
			file_ptr temp;
			if (p_want_readers)
				archive.GetFileReader (i, temp, p_out);

            if (!p_out.on_entry (this, m_url, archive.GetFileStats (i), temp)) 
                break;
		}
	}
};

static archive_factory_t<SevenZipArchiveImpl> g_archive_sevenzip;

DECLARE_COMPONENT_VERSION( "7z Unpacker", "2.1", 
    "Unpacker for 7z archives.\n"
	"Requires 7z.dll (32 bit) to work\n"
    "Get it here http://7-zip.org/download.html\n\n"
    "(c) 2011 Dmitry Duny Efimenko"
);