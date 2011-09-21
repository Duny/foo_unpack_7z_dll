#ifndef _FOO_UNPACK_7Z_DLL_DISK_CACHE_H_
#define _FOO_UNPACK_7Z_DLL_DISK_CACHE_H_

namespace unpack_7z
{
    namespace disk_cache
    {
        PFC_DECLARE_EXCEPTION (exception_disk_cache, pfc::exception, COMPONENT_NAME " disk cache error");
        PFC_DECLARE_EXCEPTION (exception_cache_init_failed, exception_disk_cache, "Couldn't initialize disk cache system");

        const t_uint32 max_cache_size = 20;


        class NOVTABLE file_cached : public service_base {
        public:
            virtual bool is_empty () const = 0;

            FB2K_MAKE_SERVICE_INTERFACE(file_cached, service_base)
        };
        typedef service_ptr_t<file_cached> file_cached_ptr;


        // do not reimplement; instantiate using static_api_ptr_t<manager>
        class NOVTABLE manager : public service_base {
        public:
            virtual file_cached_ptr find (const char *p_archive, const char *p_file) = 0;

            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(manager)
        };

        // {AE95D6B8-0AC3-453E-831E-CE3C12F36AE1}
        __declspec(selectany) const GUID file_cached::class_guid = 
        { 0xae95d6b8, 0xac3, 0x453e, { 0x83, 0x1e, 0xce, 0x3c, 0x12, 0xf3, 0x6a, 0xe1 } };

        // {309B1505-2792-4BC4-A4CB-6BC4768DD296}
        __declspec(selectany) const GUID manager::class_guid = 
        { 0x309b1505, 0x2792, 0x4bc4, { 0xa4, 0xcb, 0x6b, 0xc4, 0x76, 0x8d, 0xd2, 0x96 } };
    }
}
#endif