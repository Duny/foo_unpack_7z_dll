#ifndef _FOO_UNPACK_7Z_DLL_DISK_CACHE_H_
#define _FOO_UNPACK_7Z_DLL_DISK_CACHE_H_

namespace unpack_7z
{
    class NOVTABLE cache_system : public service_base {
    public:
        enum { cache_max_files = 20 };

        virtual t_filestats get_stats_in_archive (const char *p_archive, const char *p_file, abort_callback &p_abort) = 0;
        virtual void open_archive (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort) = 0;
        virtual void archive_list (foobar2000_io::archive *owner, const char *p_archive, const file_ptr &p_reader, archive_callback &p_out, bool p_want_readers) = 0;

        virtual void set_history_size_max (t_uint32 new_size) = 0;

        virtual void print_stats () = 0;

        FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(cache_system)
    };

    // {B9B29ADB-8842-45E9-8666-22750EE214E5}
    __declspec(selectany) const GUID cache_system::class_guid = 
    { 0xb9b29adb, 0x8842, 0x45e9, { 0x86, 0x66, 0x22, 0x75, 0xe, 0xe2, 0x14, 0xe5 } };
}
#endif