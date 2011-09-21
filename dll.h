#ifndef _FOO_UNPACK_7Z_DLL_DLL_H_
#define _FOO_UNPACK_7Z_DLL_DLL_H_

namespace unpack_7z
{
    namespace dll
    {
        PFC_DECLARE_EXCEPTION (exception_dll_7z, pfc::exception, COMPONENT_NAME);
        PFC_DECLARE_EXCEPTION (exception_dll_not_loaded, exception_dll_7z, COMPONENT_NAME ": couldn't load 7z.dll");
        PFC_DECLARE_EXCEPTION (exception_dll_func_not_found, exception_dll_7z, COMPONENT_NAME ": couldn't get \"CreateObject\" function address");
        PFC_DECLARE_EXCEPTION (exception_dll_create_class_object, exception_dll_7z, COMPONENT_NAME ": couldn't create CLSID_CFormat7z class object");
        PFC_DECLARE_EXCEPTION (exception_dll_unload, exception_dll_7z, COMPONENT_NAME ": couldn't unload dll");

        CMyComPtr<IInArchive> create_archive_object ();
    }
}
#endif