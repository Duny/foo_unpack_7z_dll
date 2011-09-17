#include "stdafx.h"
#include "archive_7z.h"
#include <iostream>

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
	static const char *g_ext_7z;

	virtual const char * get_archive_type () { return g_ext_7z; }

    virtual bool supports_content_types () { return false; }

	void check_is_our_type (const char *path) {
		if (_stricmp (pfc::string_extension (path), g_ext_7z) != 0)
			throw exception_io_data ();
	}

	virtual t_filestats get_stats_in_archive (const char *p_archive, const char *p_file, abort_callback &p_abort)
	{
		check_is_our_type (p_archive);

		debug_log () << "get_stats_in_archive(\"" << pfc::string_filename (p_archive) << "\", \"" << p_file << "\")";

		return archive_7z (p_archive, p_abort).get_stats (p_file);
	}

	virtual void open_archive (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort)
	{
		check_is_our_type (p_archive);

		DWORD start = GetTickCount ();

		archive_7z (p_archive, p_abort).get_reader (p_file, p_out, p_abort);

		DWORD end = GetTickCount ();
		debug_log () << "open_archive(\"" << pfc::string_filename (p_archive) << "\", \"" << p_file << "\")"
			<< " took " << (t_int32)(end - start) << " ms\n";
	}

	virtual void archive_list (const char *p_archive, const file_ptr &p_reader, archive_callback &p_out, bool p_want_readers)
	{
		check_is_our_type (p_archive);

		class process_item : public item_callback
		{
			archive_impl *m_owner;
			archive_7z &m_archive;
			const char *m_archive_path;
			archive_callback &m_callback;
			bool m_want_readers;

		public:
			process_item (
				archive_impl *owner,
				archive_7z &archive,
				const char *archive_path,
				archive_callback &callback,
				bool want_readers
			)
			: m_owner (owner), m_archive (archive), m_archive_path (archive_path),
				m_callback (callback), m_want_readers (want_readers) {}

			virtual bool on_item (const file_in_archive &p_file) {
				pfc::string8_fast m_url;
				m_owner->make_unpack_path (m_url, m_archive_path, p_file.m_path);
			
				file_ptr temp;
				if (m_want_readers)
					m_archive.get_reader (p_file.m_path, temp, m_callback);

				return m_callback.on_entry (m_owner, m_url, p_file.m_stats, temp);
			}
		};

		DWORD start = GetTickCount ();

		archive_7z archive;
		p_reader.is_empty () ? archive.open (p_archive, p_out) : archive.open (p_reader, p_out);
		archive.list (process_item (this, archive, p_archive, p_out, p_want_readers));

		DWORD end = GetTickCount ();
		debug_log () << "archive_list(\"" << pfc::string_filename (p_archive) << "\", " << p_want_readers << ")"
			<< " took " << (t_int32)(end - start) << " ms\n";
	}
};
const char *archive_type_7z::g_ext_7z = "7z";

static archive_factory_t<archive_type_7z> g_archive_7z;