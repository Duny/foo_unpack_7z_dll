#pragma once

// foobar2000 includes
#include "foobar2000\ATLHelpers\ATLHelpers.h"

// 7-zip includes
#include "Common\MyInitGuid.h"
#include "Common\StringConvert.h"
#include "Common\IntToString.h"

#include "Windows\PropVariant.h"
#include "Windows\PropVariantConversions.h"
#include "Windows\Registry.h"
#include "Windows\DLL.h"
#include "Windows\FileDir.h"
#include "Windows\FileName.h"
#include "Windows\FileFind.h"

#include "7zip\Common\FileStreams.h"
#include "7zip\Archive\IArchive.h"
#include "7zip\IPassword.h"
#include "7zip\ICoder.h"
#include "7zip\MyVersion.h"

#include "7zArchive.h"
#include "FileStreams.h"
#include "ExtractCallback.h" 

// helpers
#include "file_buffer.h"

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

class show_debug_message : public pfc::string_formatter
{
public:
	~show_debug_message () {
			console::formatter () << COMPONENT_NAME << ": " << get_ptr ();
	}
};