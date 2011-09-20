#include "stdafx.h"

#include "config.h"

namespace unpack_7z
{
    namespace cfg
    {
        namespace guid
        {
            const GUID debug_log = { 0x4bbcc43d, 0xb10, 0x4bd4, { 0x98, 0xba, 0x58, 0x80, 0x6a, 0x9d, 0x61, 0x5e } };
            const GUID using_custom_dll = { 0x52b27d56, 0xa744, 0x4d67, { 0xb4, 0x8f, 0x65, 0xac, 0x8a, 0xeb, 0x60, 0x65 } };
            const GUID dll_path = { 0x292200c6, 0xfe9a, 0x4174, { 0xaa, 0xf7, 0xb8, 0xe3, 0x9f, 0x41, 0x92, 0x9f } };
            const GUID disk_cache_size = { 0x6652e210, 0x3b5e, 0x4256, { 0x81, 0xb7, 0x59, 0xba, 0xef, 0x2b, 0x35, 0x67 } };
        }

        cfg_bool   debug_log (guid::debug_log, defaults::debug_log);
        cfg_bool   using_custom_dll (guid::using_custom_dll, defaults::using_custom_dll);
        cfg_string dll_path (guid::dll_path, "");
        cfg_uint   disk_cache_size (guid::disk_cache_size, defaults::disk_cache_size);
    }
}