#include "stdafx.h"

#include "Windows\Registry.h"
#include "Windows\DLL.h"
#include "Common\StringConvert.h"
#include "7zip\Archive\IArchive.h"

#include "7z_dll.h"

namespace unpack_7z
{
    namespace dll
    {
	    const UString dll_name (L"7z.dll");
	    const UString reg_path_7z (TEXT("Software") TEXT(STRING_PATH_SEPARATOR) TEXT("7-Zip"));

	    DEFINE_GUID (CLSID_CFormat7z, 0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);

	    typedef UINT32 (WINAPI * create_object_func_t)(const GUID *clsID, const GUID *interfaceID, void **outObject);

	    create_object_func_t get_func ()
	    {
            static NWindows::NDLL::CLibrary m_dll;
            static create_object_func_t m_func = nullptr;

		    if (m_func != nullptr) return m_func;

            m_dll.Free ();

		    UString path;

		    if (cfg_dll_location_mode == dll_location_default) {
			    UString triedPaths;

                auto try_folder = [&] (const UString &folder) {
                    path = folder;
                    if (path.Back () != '\\')
				        path += '\\';
                    path += dll_name;
                    triedPaths += path + '\n';

				    m_dll.Load (path);
                };

			    // 1. search for 7-zip installation folder
			    NWindows::NRegistry::CKey key;
			    if (key.Open (HKEY_CURRENT_USER, reg_path_7z, KEY_READ) == ERROR_SUCCESS) {
                    UString tmp;
				    if (key.QueryValue (TEXT ("Path"), tmp) == ERROR_SUCCESS && !tmp.IsEmpty ())
					    try_folder (tmp);
			    }

			    if (!m_dll.IsLoaded ()) // 2. look in component's installation folder
				    try_folder (GetUnicodeString (AString (pfc::string_directory (core_api::get_my_full_path ()))));

			    if (!m_dll.IsLoaded ())
				    show_error_message () << "Couldn't load 7z.dll\nLooked in:\n" << pfc::stringcvt::string_utf8_from_wide (triedPaths);
		    }
		    else { // dll_location_custom
			    if (cfg_dll_custom_path.is_empty ())
				    show_error_message () << "Custom 7z.dll location is not specified";
			    else if (!filesystem::g_exists (cfg_dll_custom_path, abort_callback_dummy ()))
				    show_error_message () << "File \"" << cfg_dll_custom_path << "\" not found";
			
			    if (!m_dll.Load (pfc::stringcvt::string_wide_from_utf8 (cfg_dll_custom_path)))
				    show_error_message () << "Couldn't load " << cfg_dll_custom_path;
			    else
				    path = GetUnicodeString (AString (cfg_dll_custom_path));
		    }

		    if (m_dll.IsLoaded ()) {
			    debug_log () << "Loaded \"" << pfc::stringcvt::string_utf8_from_wide (path) << "\"";

			    m_func = (create_object_func_t)m_dll.GetProc ("CreateObject");
			    if (m_func == nullptr)
				    throw exception_dll_func_not_found ();
                else
                    return m_func;
		    }
            else {
                m_func = nullptr;
                throw exception_dll_not_loaded ();
            }
	    }


        CMyComPtr<IInArchive> create_archive_object ()
        {
	        create_object_func_t func = get_func ();

            CMyComPtr<IInArchive> out;
	        if (func (&CLSID_CFormat7z, &IID_IInArchive, reinterpret_cast<void**>(&out)) != S_OK)
		        throw exception_dll_create_class_object ();
            return out;
        }
    }
}