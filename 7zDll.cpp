#include "stdafx.h"

#define DLL_NAME (L"7z.dll")
#define REG_PATH_7Z TEXT("Software") TEXT(STRING_PATH_SEPARATOR) TEXT("7-Zip")

// type definitions 
typedef UINT32 (WINAPI * CreateObjectFunc)(const GUID *clsID, const GUID *interfaceID, void **outObject);

// global variables
DEFINE_GUID (CLSID_CFormat7z, 0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);

static NWindows::NDLL::CLibrary g_7zDll;
static CreateObjectFunc g_createObjectFunc = NULL;

static void InitLibrary ()
{
    if (g_7zDll.IsLoaded () && g_createObjectFunc != NULL) return;

	UString dll_path;

	if (cfg_dll_location_mode == dll_location_default) {
		UString triedPaths;

		// 1.
		// try to find 7-zip installation
		NWindows::NRegistry::CKey key;
		if (key.Open (HKEY_CURRENT_USER, REG_PATH_7Z, KEY_READ) == ERROR_SUCCESS) {
			if (key.QueryValue (TEXT ("Path"), dll_path) == ERROR_SUCCESS && !dll_path.IsEmpty ()) {
				if (dll_path[dll_path.Length () - 1] != '\\')
					dll_path += '\\';
				dll_path += DLL_NAME;

				g_7zDll.Load (dll_path);

				triedPaths = dll_path;
				triedPaths += '\n';
			}
		}

		if (!g_7zDll.IsLoaded ()) {
			// 2. look in foobar2000 installation folder
			dll_path = GetUnicodeString (AString (pfc::string_directory (core_api::get_my_full_path ())));
			if (dll_path[dll_path.Length () - 1] != '\\')
					dll_path += '\\';
				dll_path += DLL_NAME;

				g_7zDll.Load (dll_path);

				triedPaths += dll_path;
		}

		if (!g_7zDll.IsLoaded ())
			show_error_message () << "Could't load 7z.dll\nLooked in:\n" << pfc::stringcvt::string_utf8_from_wide (triedPaths);
	}
	else { // dll_location_custom
		if (cfg_dll_custom_path.is_empty ())
			show_error_message () << "Custom 7z.dll location is not specified";
		else if (!filesystem::g_exists (cfg_dll_custom_path, abort_callback_dummy ()))
			show_error_message () << "File \"" << cfg_dll_custom_path << "\" not found";
		else {
			if (!g_7zDll.Load (pfc::stringcvt::string_wide_from_utf8 (cfg_dll_custom_path)))
				show_error_message () << "Could't load " << cfg_dll_custom_path;
			else
				dll_path = GetUnicodeString (AString (cfg_dll_custom_path));
		}
	}

	if (g_7zDll.IsLoaded ()) {
		if (cfg_debug_messages)
			show_debug_message () << "Loaded \"" << pfc::stringcvt::string_utf8_from_wide (dll_path) << "\"";

		g_createObjectFunc = (CreateObjectFunc)g_7zDll.GetProc ("CreateObject");
		if (g_createObjectFunc == NULL)
			show_error_message () << "Could't get \"CreateObject\" function address";
	}
}

bool CreateArchiveObject (void **out)
{
	InitLibrary ();

	if (g_createObjectFunc == NULL)
		return false;

	if (g_createObjectFunc (&CLSID_CFormat7z, &IID_IInArchive, out) != S_OK) {
		show_error_message () << "Could't create CLSID_CFormat7z class object";
		return false;
	}

	return true;
}