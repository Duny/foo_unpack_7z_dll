#include "stdafx.h"

DECLARE_COMPONENT_VERSION
(
    "7z Unpacker", // NAME
    "2.1", // VERSION
    "Unpacker for 7-Zip archives.\n" // ABOUT
	"Requires 7z.dll (32 bit) to work.\n"
    "Get it here http://7-zip.org/download.html\n\n"
    "(c) 2011 Dmitry Duny Efimenko"
);
VALIDATE_COMPONENT_FILENAME (COMPONENT_NAME ".dll");

class archive_type_7z : public archive_impl
{
	virtual const char * get_archive_type () { return "7z"; }

    virtual bool supports_content_types () { return false; }

	virtual t_filestats get_stats_in_archive (const char *p_archive, const char *p_file, abort_callback &p_abort)
	{
		pfc::string_extension ext (p_archive);
		if (strcmp (ext, get_archive_type ()) != 0)
			throw exception_io_data ();

		if (cfg_debug_messages)
			show_debug_message () << "get_stats_in_archive(\"" << p_archive << "\", \"" << p_file << "\")";

		file_ptr file;
		C7zArchive archive;

		filesystem::g_open (file, p_archive, filesystem::open_mode_read, p_abort);

		archive.Open (file, p_abort);

		const C7zArchive::t_arch_items &items = archive.items ();
		const pfc::string8_fast &file_name = p_file;
		for (t_size i = 0, n = items.get_size (); i < n; i++) {
			if (items[i].m_path == file_name)
				return items[i].m_stats;
		}
		throw exception_io_not_found ();
	}

	virtual void open_archive (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort)
	{
		pfc::string_extension ext (p_archive);
		if (strcmp (ext, get_archive_type ()) != 0)
			throw exception_io_data ();

		if (cfg_debug_messages)
			show_debug_message () << "open_archive(\"" << p_archive << "\", \"" << p_file << "\")";

		file_ptr file;
		C7zArchive archive; 

		filesystem::g_open (file, p_archive, filesystem::open_mode_read, p_abort);

		archive.Open (file, p_abort);

		const C7zArchive::t_arch_items &items = archive.items ();
		const pfc::string8_fast &file_name = p_file;
		for (t_size i = 0, n = items.get_size (); i < n; i++) {
			if (items[i].m_path == file_name)
				return archive.GetFileReader (i, p_out, p_abort);
		}
		throw exception_io_not_found ();
	}

	virtual void archive_list (const char *path, const file_ptr &p_reader, archive_callback &p_out, bool p_want_readers)
	{
		pfc::string_extension ext (path);
		if (strcmp (ext, get_archive_type ()) != 0)
			throw exception_io_data ();

		if (cfg_debug_messages)
			show_debug_message () << "archive_list(\"" << path << "\", " << p_want_readers << ")";

		file_ptr p_file = p_reader;
		if (p_file.is_empty ())
			filesystem::g_open (p_file, path, filesystem::open_mode_read, p_out);

		C7zArchive archive;
		archive.Open (p_file, p_out);
		const C7zArchive::t_arch_items &items = archive.items ();

		pfc::string8 m_url;
		for (t_size i = 0, n = items.get_size (); i < n; i++) {
			make_unpack_path (m_url, path, items[i].m_path);
			
			file_ptr temp;
			if (p_want_readers)
				archive.GetFileReader (i, temp, p_out);

            if (!p_out.on_entry (this, m_url, items[i].m_stats, temp)) 
                break;
		}
	}
};
static archive_factory_t<archive_type_7z> g_archive_7z;