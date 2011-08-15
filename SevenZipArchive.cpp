#include "stdafx.h"

DEFINE_GUID(CLSID_CFormat7z,
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);


typedef UINT32 (WINAPI * CreateObjectFunc)(
	const GUID *clsID,
	const GUID *interfaceID,
	void **outObject);


static NWindows::NDLL::CLibrary library;
static CreateObjectFunc createObjectFunc = NULL;

const wchar_t *DllName = L"7z.dll";
#define REG_PATH_7Z TEXT("Software") TEXT(STRING_PATH_SEPARATOR) TEXT("7-Zip")

static bool InitializeSevenZip ()
{
    if (library.IsLoaded () && createObjectFunc != NULL) return true;

    createObjectFunc = NULL;
    library.Free ();

	UString dll_path;

	// try to find 7-zip installation
	NWindows::NRegistry::CKey key;
	if (key.Open (HKEY_CURRENT_USER, REG_PATH_7Z, KEY_READ) == ERROR_SUCCESS) {
		if (key.QueryValue (TEXT ("Path"), dll_path) == ERROR_SUCCESS && !dll_path.IsEmpty ()) {
			if (dll_path[dll_path.Length () - 1] != '\\')
				dll_path += '\\';
			dll_path += DllName;
			library.Load (dll_path);
		}
	}

	if (!library.IsLoaded () && !library.Load (DllName)) {
		console::formatter() << "7z: could't load 7z.dll";
		return false;
	}
        
    createObjectFunc = (CreateObjectFunc)library.GetProc ("CreateObject");
	if (createObjectFunc == NULL)
		console::formatter() << "7z: could't get CreateObject proc address";

    return (createObjectFunc != 0);
}

SevenZipArchive::SevenZipArchive () 
{
}

SevenZipArchive::~SevenZipArchive ()
{
	Close ();
}

bool SevenZipArchive::Open (file_ptr const &p_file, abort_callback &p_abort)
{
	if (!InitializeSevenZip ())	return false; 

	if (createObjectFunc (&CLSID_CFormat7z, &IID_IInArchive, (void **)&m_archive) != S_OK) {
		console::formatter() << "7z: could't get class object";
		return false;
	}

	m_archive_tstamp = p_file->get_timestamp (p_abort);

    FooInStream *fileSpec = new FooInStream (p_file, p_abort);
    CMyComPtr<IInStream> file = fileSpec;

    if (m_archive->Open (file, 0, NULL) != S_OK)
        return false;

	return GetFileList ();
}

void SevenZipArchive::Close ()
{
    if (m_archive) m_archive->Close ();
}

bool SevenZipArchive::GetFileList ()
{
    ArchiveFileDesc file_desc;
	NWindows::NCOM::CPropVariant prop;

    UInt32 num_items = 0;
	if (m_archive->GetNumberOfItems (&num_items) != S_OK) {
		console::formatter() << "7z: Could't get number of items in the archive";
		return false;
	}

    m_file_list.force_reset ();
    m_file_list.prealloc (num_items);

    for (UInt32 i = 0; i < num_items; i++) {
		//
        // read file path
		//
        m_archive->GetProperty (i, kpidPath, &prop);
        if (prop.vt != VT_BSTR) continue;

        UString file_path = ConvertPropVariantToString (prop);
        int j, n = file_path.Length ();

        file_desc.m_path.reset ();
        file_desc.m_path.prealloc (n);
        for (j = 0; j < n; j++)
            file_desc.m_path.add_char (file_path[j]);

		//
        // Get file size
		//
        file_desc.m_stats.m_size = 0;

        m_archive->GetProperty (i, kpidSize, &prop);
        if (prop.vt != VT_UI8) continue;

        file_desc.m_stats.m_size = ConvertPropVariantToUInt64 (prop);

		//
        // Get file time
		//
        m_archive->GetProperty (i, kpidMTime, &prop);
		if (prop.vt == VT_EMPTY)
			file_desc.m_stats.m_timestamp = 0;
        else if (prop.vt == VT_FILETIME)
			file_desc.m_stats.m_timestamp = prop.uhVal.QuadPart;
		else 
			continue;

        m_file_list.append_single (file_desc);
    }
	
	return true;
}

t_size SevenZipArchive::NumFiles () const
{
	return !m_archive ? 0 : m_file_list.get_size ();
}

const pfc::string8& SevenZipArchive::GetFilePath (t_size i) const
{
	return m_file_list[i].m_path;
}

const t_filestats& SevenZipArchive::GetFileStats (t_size i) const
{
	return m_file_list[i].m_stats;
}

const t_filestats& SevenZipArchive::GetFileStats (const pfc::string8 &path) const
{
	for (t_size i = 0, n = m_file_list.get_size (); i < n; i++) {
		if (path == m_file_list[i].m_path)
			return m_file_list[i].m_stats;
	}
	throw exception_io_not_found ();
}

void SevenZipArchive::GetFileReader (t_size i, file_ptr &p_out, abort_callback &p_abort)
{
	p_out = new service_impl_t<file_buffer> (m_archive_tstamp, (t_size)m_file_list[i].m_stats.m_size);

    CFooExtractCallback *extractCallbackSpec = new CFooExtractCallback (p_out, p_abort);
    CMyComPtr<IArchiveExtractCallback> extractCallback (extractCallbackSpec);

    size_t index[] = { i };
    //DWORD start = GetTickCount ();
	
    HRESULT result = m_archive->Extract (index, 1, false, extractCallback);
	if (result != S_OK) {
		console::formatter () << "7z: error extracting \"" << m_file_list[i].m_path << "\"";
		throw exception_io_data ();
	}
    //DWORD end = GetTickCount ();
    p_out->reopen (p_abort);
    //console::formatter() << "decoding " << m_file_list[i].m_path << " took " << (t_int32)(end - start) << " ms\n";
}

void SevenZipArchive::GetFileReader (const pfc::string8 &path, file_ptr &p_out, abort_callback &p_abort)
{
	for (t_size i = 0, n = m_file_list.get_size (); i < n; i++) {
		if (m_file_list[i].m_path == path)
			return GetFileReader (i, p_out, p_abort);
	}
	throw exception_io_not_found ();
}