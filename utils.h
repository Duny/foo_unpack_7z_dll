#ifndef _FOO_UNPACK_7Z_DLL_UTILS_H_
#define _FOO_UNPACK_7Z_DLL_UTILS_H_

#include "config.h"

namespace unpack_7z
{
    class error_log : public pfc::string_formatter
    {
    public:
        ~error_log () { console::formatter () << COMPONENT_NAME << " error: " << get_ptr (); }
    };

    class debug_log : public pfc::string_formatter
    {
    public:
        ~debug_log () { if (cfg::debug_log) console::formatter () << COMPONENT_NAME << ": " << get_ptr (); }
    };
}
#endif