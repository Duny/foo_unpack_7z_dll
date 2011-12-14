#include "stdafx.h"

DECLARE_COMPONENT_VERSION ("7z Archive Unpacker", "3.2 beta 3",
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

        cfg_bool   use_sys_tmp_for_cache (guid_inline<0x28f84525, 0xdc60, 0x4bab, 0xab, 0xf3, 0x91, 0x32, 0xe5, 0x58, 0xbf, 0x26>::guid, defaults::use_sys_tmp_for_cache);
        cfg_bool   keep_cache_at_exit (guid_inline<0x603e6f84, 0x70ee, 0x4e9a, 0xab,0x0a, 0xa9, 0xd2, 0xf9, 0x07, 0x59, 0xdd>::guid, defaults::keep_cache_at_exit);
        cfg_string custom_cache_path (guid_inline<0xda0dbea6, 0x5f7, 0x4f0e, 0x8f, 0x9f, 0xfd, 0xec, 0xab, 0xd0, 0xaf, 0xb7>::guid, "");
        cfg_uint   file_cache_max (guid_inline<0xceb9c5d6, 0x569b, 0x484f, 0x90, 0xbe, 0x55, 0x59, 0xba, 0xe0, 0x1d, 0xf7>::guid, defaults::file_cache_max);
        
        cfg_uint   archive_history_max (guid_inline<0x507889c1, 0x3794, 0x4e03, 0x8a, 0xa5, 0xfb, 0x2, 0x6f, 0x6, 0x60, 0x38>::guid, defaults::archive_history_max);
    }
}