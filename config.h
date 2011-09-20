#ifndef _CFG_VARS_H_
#define _CFG_VARS_H_

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
            extern const __declspec(selectany) bool debug_log = false;
            extern const __declspec(selectany) bool using_custom_dll = false;
            extern const __declspec(selectany) t_uint32 disk_cache_size = 5;
        };

        void reset (); // reset config to defaults
    }
}
#endif