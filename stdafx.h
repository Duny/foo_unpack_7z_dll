#pragma once

#include <vector>
#include <algorithm>

// foobar2000 includes
#include "foobar2000\ATLHelpers\ATLHelpers.h"

// 7-zip includes
#include "Common\MyInitGuid.h"

#include "resource.h"

#define COMPONENT_NAME "foo_unpack_7z_dll"

enum dll_location_mode { 
	dll_location_default,
	dll_location_custom
};
 
extern cfg_bool cfg_quiet_mode, cfg_debug_messages;
extern cfg_uint cfg_dll_location_mode;
extern cfg_string cfg_dll_custom_path;

bool CreateArchiveObject (void **out);

class show_error_message : public pfc::string_formatter
{
public:
	~show_error_message () {
		if (cfg_quiet_mode)
			console::formatter () << COMPONENT_NAME << " error: " << get_ptr ();
		else
			popup_message::g_show (get_ptr (), COMPONENT_NAME, popup_message::icon_error);
	}
};

class debug_log : public pfc::string_formatter
{
public:
	~debug_log () {
			if (cfg_debug_messages) console::formatter () << COMPONENT_NAME << ": " << get_ptr ();
	}
};