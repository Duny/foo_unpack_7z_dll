#ifndef _FOO_UNPACK_7Z_DLL_CONFIG_H_
#define _FOO_UNPACK_7Z_DLL_CONFIG_H_

namespace unpack_7z
{
    namespace cfg
    {
        extern cfg_bool debug_log;
        extern cfg_bool using_custom_dll;
        extern cfg_string dll_path;
        extern cfg_uint disk_cache_size;

        namespace defaults 
        {
            enum
            {
                debug_log = false,
                using_custom_dll = false,
                disk_cache_size = 5
            };
        };

        void reset (); // reset configuration to defaults
    }
}
#endif