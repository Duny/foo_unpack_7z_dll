// foo_unpack_7z.cpp : Defines the exported functions for the DLL application.
//
#include "7z_open.h"

using namespace C7Z;
class archive_7z_common : public archive_impl
{
public:

    archive_7z_common () {}
    virtual ~archive_7z_common () {}

    virtual bool supports_content_types ()
	{
		return false;
	}

	virtual t_filestats get_stats_in_archive (const char *p_archive, const char *p_file, abort_callback &p_abort)
	{
        pfc::string_extension ext (p_archive);

		if (!IsSupportedType (ext))
			throw exception_io_data ();

        //console::formatter () << "get_stats_in_archive " << p_archive << " " << p_file;
        
        c7z_open archive (ext);
        file_ptr m_file;

		filesystem::g_open (m_file, p_archive, filesystem::open_mode_read, p_abort);
        archive.open_archive (m_file, p_abort);

        return archive.get_stats_in_archive (p_file, p_abort);
	}

	virtual void open_archive (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort)
	{
        pfc::string_extension ext (p_archive);

		if (!IsSupportedType (ext))
			throw exception_io_data ();

        //console::formatter() << "open_archive " << p_archive << " " << p_file;

        c7z_open archive (ext);
        file_ptr m_file;

        filesystem::g_open (m_file, p_archive, filesystem::open_mode_read, p_abort);
        archive.open_archive (m_file, p_abort);
        archive.open_file_in_archive (p_out, p_file, p_abort);
	}

	virtual void archive_list (const char *path, const file_ptr &p_reader, archive_callback &p_out, bool p_want_readers)
	{
        pfc::string_extension ext (path);

		if (!IsSupportedType (ext))
			throw exception_io_data ();

        //console::formatter() << "archive_list " << path << "; want_readers: " << (p_want_readers ? "true" : "false");
      
        c7z_open archive (ext);
        file_ptr m_file = p_reader, m_reader;
        pfc::string8_fastalloc m_url;

        if (m_file.is_empty ())
            filesystem::g_open (m_file, path, filesystem::open_mode_read, p_out);

        archive.open_archive (m_file, p_out);

        t_size num_items = archive.files.get_size (), i;
        for (i = 0; i < num_items; i++) {
            m_reader.release ();
            if (p_want_readers)
                archive.open_file_in_archive (m_reader, archive.files[i].name, p_out);
            
            archive_impl::g_make_unpack_path (m_url, path, archive.files[i].name.toString (), ext);
            console::formatter() << "on_entry(\"" << m_url << "\")";
            if (!p_out.on_entry (this, m_url, archive.files[i].stats, m_reader)) 
                break;
        }
	}
};

#define DEFINE_ARCHIVE_TYPE(archive_class_name, ext) \
class archive_class_name : public archive_7z_common { \
    public: \
        archive_class_name () {} \
        virtual ~archive_class_name () {} \
        virtual const char *get_archive_type () { return ext; } }; \
        static archive_factory_t < archive_class_name >  g_archive_factory_##archive_class_name;  \

DEFINE_ARCHIVE_TYPE(archive_7z,   "7z")
DEFINE_ARCHIVE_TYPE(archive_zip,   "zip")
DEFINE_ARCHIVE_TYPE(archive_rar,   "rar")
//DEFINE_ARCHIVE_TYPE(archive_arj,   "arj")
//DEFINE_ARCHIVE_TYPE(archive_z,     "z")
DEFINE_ARCHIVE_TYPE(archive_cab,   "cab")
//DEFINE_ARCHIVE_TYPE(archive_bz2,   "bz2")
//DEFINE_ARCHIVE_TYPE(archive_bzip2, "bzip2")
//DEFINE_ARCHIVE_TYPE(archive_iso,   "iso")
//DEFINE_ARCHIVE_TYPE(archive_tar,   "tar")
//DEFINE_ARCHIVE_TYPE(archive_gz,    "gz")
//DEFINE_ARCHIVE_TYPE(archive_gzip,  "gzip")

DECLARE_COMPONENT_VERSION( "7zip unpacker", "1.0", 
    "Unpacker for 7z, zip, rar, cab and iso archives\n"
    "Based on Igor Pavlov's 7zip archiver 9.10\n"
    "7z.dll needed for the plugin's work\n"
    "Get it here http://7-zip.org/download.html\n"
    "\n"
    "(c) 2009 Dmitry Major(Duny) Efimenko"
);