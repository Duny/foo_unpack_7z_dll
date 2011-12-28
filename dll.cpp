#include "stdafx.h"
#include "dll.h"

namespace unpack_7z
{
    namespace dll
    {    
	    DEFINE_GUID (CLSID_CFormat7z, 0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);

        typedef UINT32 (WINAPI * create_object_func_t)(const GUID *clsID, const GUID *interfaceID, void **outObject);

        class library {
            HMODULE m_module;

        public:
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
            static critical_section m_section;
            static library g_dll;
            static create_object_func_t g_proc = nullptr;

            insync (m_section);
            if (g_dll.is_loaded ()) {
                if (g_proc) return g_proc;
                else if (!g_dll.free ()) throw exception_dll_unload ();
                g_proc = nullptr;
            }

		    pfc::string8 path, triedPaths;

            auto try_folder = [&] (const char *folder) {
                path = folder;
                if (!path.ends_with ('\\'))
                    path.add_char ('\\');
                path += "7z.dll";
                triedPaths += path;
                triedPaths.add_char ('\n');
                g_dll.load (path);
            };

            // 1. look in component's installation folder
            try_folder (pfc::string_directory (core_api::get_my_full_path ()));

            // 2. search for 7-zip installation folder
            HKEY key;
            if (RegOpenKeyEx (HKEY_CURRENT_USER, L"Software\\7-Zip", 0, KEY_READ, &key) == ERROR_SUCCESS) {
                DWORD value_size = 0, dummy = 0;
                LONG res = RegQueryValueEx (key, TEXT ("Path"), nullptr, &dummy, nullptr, &value_size);
                if (res == ERROR_SUCCESS || res == ERROR_MORE_DATA) {
                    pfc::array_t<TCHAR> tmp; tmp.set_size (value_size + 1);
                    res = RegQueryValueEx (key, TEXT ("Path"), nullptr, &dummy, (LPBYTE)tmp.get_ptr (), &value_size);
                    if (res == ERROR_SUCCESS) 
                        try_folder (pfc::stringcvt::string_ansi_from_wide (tmp.get_ptr ()));
                }
                RegCloseKey (key);
            }
            
		    if (g_dll.is_loaded ()) {
			    debug_log () << "Loaded \"" << path << "\"";
			    g_proc = g_dll.get_proc ();
			    if (g_proc == nullptr)
				    throw exception_dll_func_not_found ();
                else
                    return g_proc;
		    }
            else {
                error_log () << "Couldn't load 7z.dll. Tried:\n" << triedPaths;
                throw exception_dll_not_loaded ();
            }
	    }

        CMyComPtr<IInArchive> create_archive_object ()
        {
	        create_object_func_t func = get_func ();

            CMyComPtr<IInArchive> out;
	        if (func (&CLSID_CFormat7z, &IID_IInArchive, (void**)&out) != S_OK)
		        throw exception_dll_create_class_object ();
            return out;
        }
    }
}