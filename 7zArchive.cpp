#include "stdafx.h"

void C7zArchive::Open (file_ptr const &p_file, abort_callback &p_abort)
{
	if (!CreateArchiveObject ((void **)&m_archive))
		throw std::exception ("Could't init 7z.dll");

	m_timestamp = p_file->get_timestamp (p_abort);

    FooInStream *fileSpec = new FooInStream (p_file, p_abort);
    CMyComPtr<IInStream> file = fileSpec;

    if (m_archive->Open (file, 0, NULL) != S_OK)
        throw exception_io_unsupported_format ();

	GetFileList ();
}

void C7zArchive::GetFileList ()
{
    UInt32 num_items = 0;
	if (m_archive->GetNumberOfItems (&num_items) != S_OK) {
		show_error_message () << "Could't get number of items in the archive";
		throw exception_io_data ();
	}

    m_items.prealloc (num_items);

	file_desc_t file_desc;
	NWindows::NCOM::CPropVariant prop;

    file_desc.m_stats.m_timestamp = m_timestamp;

    for (UInt32 i = 0; i < num_items; i++) {
        m_archive->GetProperty (i, kpidPath, &prop);
        if (prop.vt != VT_BSTR) continue;
		file_desc.m_path = pfc::stringcvt::string_utf8_from_wide (ConvertPropVariantToString (prop));

        m_archive->GetProperty (i, kpidSize, &prop);
        if (prop.vt != VT_UI8) continue;
        file_desc.m_stats.m_size = ConvertPropVariantToUInt64 (prop);

        m_items.append_single (file_desc);
    }
}

void C7zArchive::GetFileReader (t_size i, file_ptr &p_out, abort_callback &p_abort)
{
	p_out = new service_impl_t<file_buffer> (m_items[i].m_stats);

    CFooExtractCallback *extractCallbackSpec = new CFooExtractCallback (p_out, p_abort);
    CMyComPtr<IArchiveExtractCallback> extractCallback (extractCallbackSpec);

    size_t index[] = { i };
    //DWORD start = GetTickCount ();
	
    HRESULT result = m_archive->Extract (index, 1, false, extractCallback);
	if (result != S_OK) {
		show_error_message () << "Error extracting \"" << m_items[i].m_path << "\"";
		throw exception_io_data ();
	}
    //DWORD end = GetTickCount ();
    p_out->reopen (p_abort);
    //console::formatter() << "decoding " << m_file_list[i].m_path << " took " << (t_int32)(end - start) << " ms\n";
}