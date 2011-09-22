#ifndef _FOO_UNPACK_7Z_DLL_DISK_CACHE_H_
#define _FOO_UNPACK_7Z_DLL_DISK_CACHE_H_

#include "archive.h"

namespace unpack_7z
{
    namespace disk_cache
    {
        const t_uint32 max_cache_size = 20;

        // do not reimplement; instantiate using static_api_ptr_t<manager>
        class NOVTABLE manager : public service_base {
        public:
            // false then file not found in cache
            virtual bool fetch (const char *p_archive, const char *p_file, file_ptr &p_out, abort_callback &p_abort) = 0;
            virtual void store (const char *p_archive, const char *p_file, const file_ptr &p_in, const t_filetimestamp &timestamp, abort_callback &p_abort) = 0;

            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(manager)
        };

        // fetches file from cache or unpacks file from the archive if file is not cached
        // function allocates space for memory file and returns 
        // throws exceptions on errors
        bool fetch_or_unpack (const char *p_archive, const char *p_file, file_ptr &p_out, abort_callback &p_abort);
        //
        bool fetch_or_unpack (const unpack_7z::archive &p_archive, const char *p_file, file_ptr &p_out, abort_callback &p_abort);

        // {309B1505-2792-4BC4-A4CB-6BC4768DD296}
        __declspec(selectany) const GUID manager::class_guid = 
        { 0x309b1505, 0x2792, 0x4bc4, { 0xa4, 0xcb, 0x6b, 0xc4, 0x76, 0x8d, 0xd2, 0x96 } };
    }
}
#endif