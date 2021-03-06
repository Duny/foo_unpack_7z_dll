#include "stdafx.h"
#include "cache_system.h"

namespace unpack_7z
{
    class archive_type_7z : public archive_impl
    {
        const char * get_archive_type () override { return _7Z_EXT; }

        bool supports_content_types () override { return false; }

        void check_is_our_type (const char *path) {
            if (pfc::stricmp_ascii (pfc::string_extension (path), _7Z_EXT) != 0)
                throw exception_io_data ();
        }

        t_filestats get_stats_in_archive (const char *p_archive, const char *p_file, abort_callback &p_abort) override
        {
            check_is_our_type (p_archive);
            operation_timing_timer timer (pfc::string_formatter () << "get_stats_in_archive(" << pfc::string_filename_ext (p_archive) << ", " << p_file << ")", TRUE);
            auto ret = static_api_ptr_t<cache_system>()->get_stats_in_archive (p_archive, p_file, p_abort);
            return timer.stop (), ret;
        }

        void open_archive (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort) override
        {
            check_is_our_type (p_archive);
            operation_timing_timer timer (pfc::string_formatter () << "open_archive(" << pfc::string_filename_ext (p_archive) << ", " << p_file << ")", TRUE);
            static_api_ptr_t<cache_system>()->open_archive (p_out, p_archive, p_file, p_abort);
            timer.stop ();
        }

        void archive_list (const char *p_archive, const file_ptr &p_reader, archive_callback &p_out, bool p_want_readers) override
        {
            check_is_our_type (p_archive);
            operation_timing_timer timer (pfc::string_formatter () << "archive_list(" << pfc::string_filename_ext (p_archive), TRUE);
            static_api_ptr_t<cache_system>()->archive_list (this, p_archive, p_reader, p_out, p_want_readers);
            timer.stop ();
        }
    };


    namespace { archive_factory_t<archive_type_7z> g_factory; }
}