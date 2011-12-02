#include "stdafx.h"

DECLARE_COMPONENT_VERSION ("7z Archive Unpacker", "3.1 beta",
"Unpacker for 7-Zip archives.\n"
"Requires 7z.dll (32 bit) to work.\n"
"Get it here http://7-zip.org/download.html\n\n"
"(c) 2009-2011 Efimenko Dmitry (majorquake3@gmail.com)");
VALIDATE_COMPONENT_FILENAME (COMPONENT_NAME ".dll");


namespace unpack_7z
{
    namespace cfg
    {
        cfg_bool   debug_log (guid_inline<0x4bbcc43d, 0xb10, 0x4bd4, 0x98, 0xba, 0x58, 0x80, 0x6a, 0x9d, 0x61, 0x5e>::guid, defaults::debug_log);

        cfg_bool   dll_path_custom (guid_inline<0x52b27d56, 0xa744, 0x4d67, 0xb4, 0x8f, 0x65, 0xac, 0x8a, 0xeb, 0x60, 0x65>::guid, defaults::dll_path_custom);
        cfg_string dll_path (guid_inline<0x292200c6, 0xfe9a, 0x4174, 0xaa, 0xf7, 0xb8, 0xe3, 0x9f, 0x41, 0x92, 0x9f>::guid, "");

        cfg_bool   cache_location_custom (guid_inline<0x28f84525, 0xdc60, 0x4bab, 0xab, 0xf3, 0x91, 0x32, 0xe5, 0x58, 0xbf, 0x26>::guid, defaults::cache_location_custom);
        cfg_string cache_location (guid_inline<0xda0dbea6, 0x5f7, 0x4f0e, 0x8f, 0x9f, 0xfd, 0xec, 0xab, 0xd0, 0xaf, 0xb7>::guid, "");
        cfg_uint   cache_size (guid_inline<0x6652e210, 0x3b5e, 0x4256, 0x81, 0xb7, 0x59, 0xba, 0xef, 0x2b, 0x35, 0x67>::guid, defaults::cache_size);
        cfg_uint   file_cache_size_max (guid_inline<0xceb9c5d6, 0x569b, 0x484f, 0x90, 0xbe, 0x55, 0x59, 0xba, 0xe0, 0x1d, 0xf7>::guid, defaults::file_cache_size_max);

        cfg_uint   archive_history_max (guid_inline<0x507889c1, 0x3794, 0x4e03, 0x8a, 0xa5, 0xfb, 0x2, 0x6f, 0x6, 0x60, 0x38>::guid, defaults::archive_history_max);
    }
}