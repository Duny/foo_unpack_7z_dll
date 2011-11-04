#include "stdafx.h"

#include "disk_cache.h"
#include "utils.h"

DECLARE_COMPONENT_VERSION
(
    "7z Archive Unpacker", // NAME
    "3.0.6", // VERSION
    "Unpacker for 7-Zip archives.\n" // ABOUT
	"Requires 7z.dll (32 bit) to work.\n"
    "Get it here http://7-zip.org/download.html\n\n"
	"(c) 2009-2011 Dmitry Duny Efimenko"
);
VALIDATE_COMPONENT_FILENAME (COMPONENT_NAME ".dll");

namespace unpack_7z
{
    class archive_type_7z : public archive_impl
    {
	    virtual const char * get_archive_type () { return _7Z_EXT; }

        virtual bool supports_content_types () { return false; }

	    void check_is_our_type (const char *path) {
		    if (_stricmp (pfc::string_extension (path), _7Z_EXT) != 0)
			    throw exception_io_data ();
	    }

	    virtual t_filestats get_stats_in_archive (const char *p_archive, const char *p_file, abort_callback &p_abort)
	    {
		    check_is_our_type (p_archive);

		    debug_log () << "get_stats_in_archive(" << pfc::string_filename_ext (p_archive) << ", " << p_file << ")";

		    return unpack_7z::archive (p_archive, p_abort).get_stats (p_file);
	    }

	    virtual void open_archive (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort)
	    {
		    check_is_our_type (p_archive);

		    DWORD start = GetTickCount ();
            debug_log () << "open_archive(" << pfc::string_filename_ext (p_archive) << ", " << p_file << ")";

            disk_cache::fetch_or_unpack (p_archive, p_file, p_out, p_abort);

		    DWORD end = GetTickCount ();
		    debug_log () << "open_archive(" << pfc::string_filename_ext (p_archive) << ") took " << (t_int32)(end - start) << " ms\n";
	    }

	    virtual void archive_list (const char *p_archive, const file_ptr &p_reader, archive_callback &p_out, bool p_want_readers)
	    {
		    check_is_our_type (p_archive);

		    DWORD start = GetTickCount ();
            debug_log () << "archive_list(" << pfc::string_filename_ext (p_archive) << ")";

		    unpack_7z::archive archive;
		    p_reader.is_empty () ? archive.open (p_archive, p_out) : archive.open (p_reader, p_out);

		    archive.list ([&] (const pfc::string8 &p_file, const t_filestats &p_stats) -> bool
            {
			    pfc::string8_fast m_url;
			    make_unpack_path (m_url, p_archive, p_file);
			
			    file_ptr temp;
			    if (p_want_readers)
                    disk_cache::fetch_or_unpack (archive, p_file, temp, p_out);

			    return p_out.on_entry (this, m_url, p_stats, temp);
            });

		    DWORD end = GetTickCount ();
		    debug_log () << "archive_list(" << pfc::string_filename_ext (p_archive) <<") took " << (t_int32)(end - start) << " ms\n";
	    }
    };
    static archive_factory_t<archive_type_7z> g_archive_7z;
}