#include "7z_open.h"

namespace C7Z
{
    DEFINE_GUID(CLSID_CFormatZip,
      0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00);

    DEFINE_GUID(CLSID_CFormatBZip2,
      0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x02, 0x00, 0x00);

    DEFINE_GUID(CLSID_CFormatRar,
      0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x03, 0x00, 0x00);

    DEFINE_GUID(CLSID_CFormatArj,
      0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x04, 0x00, 0x00);

    DEFINE_GUID(CLSID_CFormatZ,
      0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x05, 0x00, 0x00);

    DEFINE_GUID(CLSID_CFormatLzh,
      0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x06, 0x00, 0x00);

    DEFINE_GUID(CLSID_CFormat7z,
      0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);

    DEFINE_GUID(CLSID_CFormatCab,
      0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x08, 0x00, 0x00);

    DEFINE_GUID(CLSID_CFormatNsis,
      0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x09, 0x00, 0x00);

    DEFINE_GUID(CLSID_CFormatLzma,
      0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0A, 0x00, 0x00);

    DEFINE_GUID(CLSID_CFormatIso,
      0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xE7, 0x00, 0x00);

    DEFINE_GUID(CLSID_CFormatTar,
      0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xEE, 0x00, 0x00);

    DEFINE_GUID(CLSID_CFormatGZip,
      0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xEF, 0x00, 0x00);

    struct SupportedArchiveTypes_t
    {
        const char *ext;
        const GUID guid;
    };
    static SupportedArchiveTypes_t supported_unpack_archives[] = 
    {
        { "7z",    CLSID_CFormat7z },
        { "rar",   CLSID_CFormatRar },
        { "zip",   CLSID_CFormatZip },
        { "arj",   CLSID_CFormatArj },
        { "z",     CLSID_CFormatZ },
        { "cab",   CLSID_CFormatCab },
        { "bz2",   CLSID_CFormatBZip2 },
        { "bzip2", CLSID_CFormatBZip2 },
        { "iso",   CLSID_CFormatIso },
        { "tar",   CLSID_CFormatTar },
        { "gz",    CLSID_CFormatGZip },
        { "gzip",  CLSID_CFormatGZip }
    };
}

using namespace NWindows;

#define kDllName "7z.dll"

typedef UINT32 (WINAPI * CreateObjectFunc)(
    const GUID *clsID,
    const GUID *interfaceID,
    void **outObject);

class CArchiveExtractCallback : public IArchiveExtractCallback, public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP1(IArchiveOpenCallback)

    // IProgress
    STDMETHOD(SetTotal)(UInt64 size);
    STDMETHOD(SetCompleted)(const UInt64 *completeValue);

    // IArchiveExtractCallback
    STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode);
    STDMETHOD(PrepareOperation)(Int32 askExtractMode);
    STDMETHOD(SetOperationResult)(Int32 resultEOperationResult);

    CArchiveExtractCallback (file_ptr &_stream, abort_callback &_p_abort) : p_abort (_p_abort), stream (_stream) {}

private:
    abort_callback &p_abort;
    file_ptr       &stream;

    COutFileStream *_outFileStreamSpec;
    CMyComPtr<ISequentialOutStream> _outFileStream; 
};


STDMETHODIMP CArchiveExtractCallback::SetTotal(UInt64 /* size */)
{
    p_abort.check ();
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetCompleted(const UInt64 * /* completeValue */)
{
    p_abort.check ();
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::GetStream (UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode)
{
    p_abort.check ();

    *outStream = 0;
    _outFileStream.Release ();

    if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
        return S_OK;

    _outFileStreamSpec = new COutFileStream (stream, p_abort);

    CMyComPtr<ISequentialOutStream> outStreamLoc (_outFileStreamSpec);
   
    _outFileStream = outStreamLoc;
    *outStream = outStreamLoc.Detach ();
    
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::PrepareOperation (Int32 askExtractMode)
{
    p_abort.check ();

    switch (askExtractMode)
    {
        case NArchive::NExtract::NAskMode::kExtract:  return S_OK; break;
        case NArchive::NExtract::NAskMode::kSkip:     return S_OK; break;
    };

    return S_FALSE;
}

STDMETHODIMP CArchiveExtractCallback::SetOperationResult (Int32 operationResult)
{
    p_abort.check ();

    _outFileStream.Release();
    return S_OK;
}


namespace C7Z 
{
    static NWindows::NDLL::CLibrary library;
    static CreateObjectFunc createObjectFunc = NULL;

    static bool Load7z ()
    {
        if (library.IsLoaded () && createObjectFunc != NULL) return true;

        createObjectFunc = NULL;
        library.Free ();

        if (!library.Load (TEXT (kDllName))) return false;
            
        createObjectFunc = (CreateObjectFunc)library.GetProcAddress ("CreateObject");
        return (createObjectFunc != 0);
    }

    static int FindGUIDIndex (pfc::string8 ext)
    {
        size_t i, n = sizeof (supported_unpack_archives) / sizeof (supported_unpack_archives[0]);
        for (i = 0; i < n; i++) {
            if (!_stricmp (ext.get_ptr (), supported_unpack_archives[i].ext))
                return (int)i;
        }
        return -1;
    }

    bool IsSupportedType (const char *ext)
    {
        return FindGUIDIndex (ext) != -1;
    }

    c7z_open::c7z_open (const char *archive_type) 
    {
        if (!Load7z ()) throw std::exception ("Cannot load 7z.dll");

        int GUID_index = FindGUIDIndex (archive_type);
        if (GUID_index == -1) throw std::exception ("Archive type not supported");

        if (createObjectFunc (&supported_unpack_archives[GUID_index].guid, &IID_IInArchive, (void **)&archive7z) != S_OK)
            throw std::exception ("Can not get class object");
        //console::formatter() << "created archive reader \"" << supported_unpack_archives[GUID_index].ext << "\"";
    }

    void c7z_open::open_archive (file_ptr const &p_file, abort_callback &p_abort)
    {
        if (p_file.is_empty ()) throw exception_7z_no_open ();

        CInFileStream *fileSpec = new CInFileStream (p_file, p_abort);
        CMyComPtr<IInStream> file = fileSpec;

        HRESULT res = archive7z->Open (file, 0, NULL);
        if (res != S_OK) {
            //console::formatter() << "open_archive returned " << res;
            throw std::exception ("Can not open archive");
        }

        read_file_list ();
    }
    
    void c7z_open::close ()
    {
        archive7z->Close ();
    }

    void c7z_open::read_file_list ()
    {
        c7z_file_info      file;
        NCOM::CPropVariant prop;

        UInt32 numItems = 0;
        if (archive7z->GetNumberOfItems (&numItems) != S_OK)
            throw std::exception ("Can not get number of files in archive");

        files.force_reset ();
        files.prealloc (numItems);

        for (UInt32 i = 0; i < numItems; i++) {
            // Get name of file
            archive7z->GetProperty (i, kpidPath, &prop);
            if (prop.vt != VT_BSTR) continue;
            {
                UString file_name = ConvertPropVariantToString (prop);
                int i, n = file_name.Length ();

                file.name.reset ();
                file.name.prealloc (n);
                for (i = 0; i < n; i++)
                    file.name.add_char (file_name[i]);
            }
            //file.name = UnicodeStringToMultiByte (ConvertPropVariantToString (prop));

            // Get uncompressed size of file
            file.stats.m_size = 0;
            archive7z->GetProperty (i, kpidSize, &prop);
            if (prop.vt != VT_EMPTY)
                file.stats.m_size = ConvertPropVariantToUInt64 (prop);

            // Get file time
            file.stats.m_timestamp = 0;
            archive7z->GetProperty (i, kpidMTime, &prop);
            if (prop.vt == VT_FILETIME)
                file.stats.m_timestamp = prop.uhVal.QuadPart;

            files.append_single (file);
        }
    }

    t_filestats c7z_open::get_stats_in_archive (const char *p_file, abort_callback &p_abort)
    {
        size_t num_files = files.get_size (), i = 0;
        for (; i < num_files; i++) {
            p_abort.check ();
            if (!strcmp (files[i].name.toString (), p_file)) {
                return files[i].stats;
            }
        }
   
        throw exception_7z_file_not_found ();
    }

    void c7z_open::open_file_in_archive (file_ptr &p_out, pfc::string8 file, abort_callback &p_abort)
    {
        filesystem::g_open_tempmem (p_out, p_abort);

        CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback (p_out, p_abort);
        CMyComPtr<IArchiveExtractCallback> extractCallback (extractCallbackSpec);
        
        size_t num_files = files.get_size (), i = 0;
        for (; i < num_files; i++) {
            p_abort.check ();
            if (file == files[i].name) 
                break;    
        }

        if (i == num_files) 
            throw exception_7z_file_not_found (); 

        t_filesize m_size = files[i].stats.m_size;
        p_out->resize (m_size, p_abort);
        size_t index[] = { i };
        DWORD start = GetTickCount ();
        HRESULT result = archive7z->Extract (index, 1, false, extractCallback);
        if (result != S_OK) 
            throw std::exception ("Can not open file in archive"); 
        
        DWORD end = GetTickCount ();
        p_out->reopen (p_abort);
        //console::formatter() << "decoding " << files[i].name << " took " << (t_int32)(end - start) << " ms\n";
    }
}