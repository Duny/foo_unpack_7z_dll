#ifndef _FOO_UNPACK_7Z_DLL_UTILS_H_
#define _FOO_UNPACK_7Z_DLL_UTILS_H_

#include "config.h"

namespace unpack_7z
{
    class error_log : public pfc::string_formatter
    {
    public:
        ~error_log () { if (!is_empty()) console::formatter () << "Error("COMPONENT_NAME"):" << get_ptr (); }
    };

    class debug_log : public pfc::string_formatter
    {
    public:
        ~debug_log () { if (!is_empty() && cfg::debug_log) console::formatter () << "Debug("COMPONENT_NAME"):" << get_ptr (); }
    };
}
#endif