#include "stdafx.h"

namespace unpack_7z
{
    class archive_type_7z : public archive_impl
    {
        const char * get_archive_type () override { return _7Z_EXT; }

        bool supports_content_types () override { return false; }

        inline void check_is_our_type (const char *path) {
            if (pfc::stricmp_ascii (pfc::string_extension (path), _7Z_EXT) != 0)
                throw exception_io_data ();
        }

        t_filestats get_stats_in_archive (const char *p_archive, const char *p_file, abort_callback &p_abort) override
        {
            check_is_our_type (p_archive);
            operation_timer timer (pfc::string_formatter () << "get_stats_in_archive(" << pfc::string_filename_ext (p_archive) << ", " << p_file << ")");

            return unpack_7z::archive (p_archive, p_abort).get_stats (p_file);
        }

        void open_archive (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort) override
        {
            check_is_our_type (p_archive);
            operation_timer timer (pfc::string_formatter () << "open_archive(" << pfc::string_filename_ext (p_archive) << ", " << p_file << ")");

            //disk_cache::fetch_or_unpack (p_archive, p_file, p_out, p_abort);
            static_api_ptr_t<cache_system>()->extract (p_out, p_archive, p_file, p_abort);
        }

        void archive_list (const char *p_archive, const file_ptr &p_reader, archive_callback &p_out, bool p_want_readers) override
        {
            check_is_our_type (p_archive);
            operation_timer timer (pfc::string_formatter () << "archive_list(" << pfc::string_filename_ext (p_archive));

            unpack_7z::archive archive;
            p_reader.is_empty () ? archive.open (p_archive, p_out) : archive.open (p_reader, p_out);

            archive.list ([&] (const pfc::string8 &p_file, const t_filestats &p_stats) -> bool
            {
                pfc::string8_fast m_url;
                make_unpack_path (m_url, p_archive, p_file);

                file_ptr temp;
                if (p_want_readers)
                    static_api_ptr_t<cache_system>()->extract (temp, p_archive, p_file, p_out);
                    //disk_cache::fetch_or_unpack (archive, p_file, temp, p_out);

                return p_out.on_entry (this, m_url, p_stats, temp);
            });
        }
    };
    static archive_factory_t<archive_type_7z> g_archive_7z;
}