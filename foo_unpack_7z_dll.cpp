#include "stdafx.h"
#include "archive_7z.h"

DECLARE_COMPONENT_VERSION
(
    "7z Archive Unpacker", // NAME
    "2.1", // VERSION
    "Unpacker for 7-Zip archives.\n" // ABOUT
	"Requires 7z.dll (32 bit) to work.\n"
    "Get it here http://7-zip.org/download.html\n\n"
	"(c) 2011 Dmitry Duny Efimenko"
);
VALIDATE_COMPONENT_FILENAME (COMPONENT_NAME ".dll");

class archive_type_7z : public archive_impl
{
	static const char *archive_ext;

	file_ptr m_last_opened_file;
	pfc::string8 m_last_opened_file_path;
	critical_section m_sync;

	virtual const char * get_archive_type () { return archive_ext; }

    virtual bool supports_content_types () { return false; }

	virtual t_filestats get_stats_in_archive (const char *p_archive, const char *p_file, abort_callback &p_abort)
	{
		if (_stricmp (pfc::string_extension (p_archive), archive_ext) != 0)
			throw exception_io_data ();

		if (cfg_debug_messages)
			show_debug_message () << "get_stats_in_archive(\"" << pfc::string_filename (p_archive) << "\", \"" << p_file << "\")";

		file_ptr file;
		archive_7z archive;
		
		filesystem::g_open (file, p_archive, filesystem::open_mode_read, p_abort);
		archive.Open (p_archive, file, p_abort);

		const archive_7z::t_arch_items &items = archive.items ();
		const pfc::string8_fast &file_name = p_file;
		for (t_size i = 0, n = items.get_size (); i < n; i++) {
			if (items[i].m_path == file_name)
				return items[i].m_stats;
		}
		throw exception_io_not_found ();
	}

	virtual void open_archive (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort)
	{
		if (_stricmp (pfc::string_extension (p_archive), archive_ext) != 0)
			throw exception_io_data ();

		file_ptr file;
		archive_7z archive;
	
		const pfc::string8_fast &file_name = p_file;

		DWORD start = GetTickCount ();
		if (file_name == m_last_opened_file_path) {
			insync (m_sync);

			filesystem::g_open_tempmem (p_out, p_abort);
			m_last_opened_file->reopen (p_abort);
			file::g_transfer_file (m_last_opened_file, p_out, p_abort);
		}
		else {
			insync (m_sync);

			filesystem::g_open (file, p_archive, filesystem::open_mode_read, p_abort);
			archive.Open (p_archive, file, p_abort);

			const archive_7z::t_arch_items &items = archive.items ();
			
			t_size i = 0, n = items.get_size ();
			for (; i < n; i++) 
				if (items[i].m_path == file_name)
					break;
			
			if (i == n)
				throw exception_io_not_found ();
			else {
				archive.GetFileReader (i, p_out, p_abort);

				if (m_last_opened_file.is_valid ())
					m_last_opened_file.release ();
				filesystem::g_open_tempmem (m_last_opened_file, p_abort);

				file::g_transfer_file (p_out, m_last_opened_file, p_abort);
				m_last_opened_file_path = file_name;

				p_out->reopen (p_abort);
			}
		}
		DWORD end = GetTickCount ();
		
		if (cfg_debug_messages)
			show_debug_message () << "open_archive(\"" << pfc::string_filename (p_archive) << "\", \"" << p_file << "\")" <<
				" took " << (t_int32)(end - start) << " ms\n";
	}

	virtual void archive_list (const char *p_archive, const file_ptr &p_reader, archive_callback &p_out, bool p_want_readers)
	{
		if (_stricmp (pfc::string_extension (p_archive), archive_ext) != 0)
			throw exception_io_data ();

		file_ptr p_file = p_reader;
		if (p_file.is_empty ())
			filesystem::g_open (p_file, p_archive, filesystem::open_mode_read, p_out);

		archive_7z archive;
		archive.Open (p_archive, p_file, p_out);
		const archive_7z::t_arch_items &items = archive.items ();

		DWORD start = GetTickCount ();

		pfc::string8 m_url;
		for (t_size i = 0, n = items.get_size (); i < n; i++) {
			make_unpack_path (m_url, p_archive, items[i].m_path);
			
			file_ptr temp;
			if (p_want_readers)
				archive.GetFileReader (i, temp, p_out);

            if (!p_out.on_entry (this, m_url, items[i].m_stats, temp)) 
                break;
		}

		DWORD end = GetTickCount ();
		if (cfg_debug_messages)
			show_debug_message () << "archive_list(\"" << pfc::string_filename (p_archive) << "\", " << p_want_readers << ")" <<
				" took " << (t_int32)(end - start) << " ms\n";
	}
};
const char *archive_type_7z::archive_ext = "7z";

static archive_factory_t<archive_type_7z> g_archive_7z;