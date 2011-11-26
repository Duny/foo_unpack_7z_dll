#ifndef _FOO_UNPACK_7Z_DLL_DISK_CACHE_H_
#define _FOO_UNPACK_7Z_DLL_DISK_CACHE_H_

namespace unpack_7z
{
    class NOVTABLE cache_system : public service_base {
    public:
        enum { cache_max_files = 20 };


        virtual void extract (file_ptr &p_out, const char *p_archive, const char *p_file, abort_callback &p_abort) = 0;

        FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(cache_system)
    };

    // {B9B29ADB-8842-45E9-8666-22750EE214E5}
    __declspec(selectany) const GUID cache_system::class_guid = 
    { 0xb9b29adb, 0x8842, 0x45e9, { 0x86, 0x66, 0x22, 0x75, 0xe, 0xe2, 0x14, 0xe5 } };
}
#endif