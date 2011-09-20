#include "stdafx.h"

#include "Common\StringConvert.h"
#include "7zip\Archive\IArchive.h"
#include "7z_dll.h"
#include "config.h"
#include "utils.h"
#include "boost\smart_ptr\scoped_array.hpp"

namespace unpack_7z
{
    namespace dll
    {    
	    DEFINE_GUID (CLSID_CFormat7z, 0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);

        typedef UINT32 (WINAPI * create_object_func_t)(const GUID *clsID, const GUID *interfaceID, void **outObject);

        class library {
            HMODULE m_module;

        public:
            library () : m_module (nullptr) {}
            ~library() { free (); }

            bool load (const pfc::string8 &path) {
                HMODULE new_module = uLoadLibrary (path);
                if (!new_module) return false;
                if (!free ()) return false;
                m_module = new_module;
                return true;
            }

            bool is_loaded () const { return m_module != nullptr; }

            bool free () {
                if (m_module == nullptr) return true;
                if (!FreeLibrary (m_module)) return false;
                m_module = nullptr;
                return true;
            }

            create_object_func_t get_proc () const { return (create_object_func_t)GetProcAddress (m_module, "CreateObject"); }
        };

	    create_object_func_t get_func ()
	    {
            static library m_dll;

            if (m_dll.is_loaded ()) {
                create_object_func_t func = m_dll.get_proc ();
                if (func)
                    return func;
                else {
                    if (!m_dll.free ())
                        throw exception_dll_unload ();
                }
            }

		    pfc::string8 path;

		    if (cfg::using_custom_dll) {
                if (cfg::dll_path.is_empty ())
                    error_log () << "Custom 7z.dll location is not specified";
                else if (!filesystem::g_exists (cfg::dll_path, abort_callback_dummy ()))
                    error_log () << "File \"" << cfg::dll_path << "\" not found";

                if (!m_dll.load (cfg::dll_path))
                    error_log () << "Couldn't load " << cfg::dll_path;
                else
                    path = cfg::dll_path;
		    }
		    else {
                pfc::string8 triedPaths;

                auto try_folder = [&] (const char *folder) {
                    path = folder;
                    if (!path.ends_with ('\\'))
                        path.add_char ('\\');
                    path += "7z.dll";
                    triedPaths += path;
                    triedPaths.add_char ('\n');

                    m_dll.load (path);
                };

                // 1. search for 7-zip installation folder
                HKEY key;
                if (RegOpenKeyEx (HKEY_CURRENT_USER, L"Software\\7-Zip", 0, KEY_READ, &key) == ERROR_SUCCESS) {
                    DWORD value_size = 0, dummy = 0;
                    LONG res = RegQueryValueEx (key, TEXT ("Path"), nullptr, &dummy, nullptr, &value_size);
                    if (res == ERROR_SUCCESS || res == ERROR_MORE_DATA) {
                        boost::scoped_array<TCHAR> tmp (new TCHAR[value_size + 1]);
                        res = RegQueryValueEx (key, TEXT ("Path"), nullptr, &dummy, (LPBYTE)tmp.get (), &value_size);
                        if (res == ERROR_SUCCESS)
                            try_folder (pfc::stringcvt::string_ansi_from_wide (tmp.get ()));
                    }

                    RegCloseKey (key);
                }

                if (!m_dll.is_loaded ()) // 2. look in component's installation folder
                    try_folder (pfc::string_directory (core_api::get_my_full_path ()));

                if (!m_dll.is_loaded ())
                    error_log () << "Couldn't load 7z.dll. Looked in:\n" << triedPaths;
		    }

		    if (m_dll.is_loaded ()) {
			    debug_log () << "Loaded \"" << path << "\"";

			    create_object_func_t func = m_dll.get_proc ();
			    if (func == nullptr)
				    throw exception_dll_func_not_found ();
                else
                    return func;
		    }
            else
                throw exception_dll_not_loaded ();
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