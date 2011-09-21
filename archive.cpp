#include "stdafx.h"

#include "Windows\PropVariant.h"
#include "Windows\PropVariantConversions.h"

#include "archive.h"
#include "dll.h"
#include "extract_callback.h"
#include "tempmem_with_timestamp.h"
#include "disk_cache.h"
#include "utils.h"

namespace unpack_7z
{
    void archive::open (file_ptr const &p_file, abort_callback &p_abort)
    {
        static_api_ptr_t<disk_cache::manager> api;

        try {
            CMyComPtr<IInArchive> archive_new (dll::create_archive_object ());
            static_api_ptr_t<disk_cache::manager> api;
            CMyComPtr<IInStream> stream_new (new streams::in (p_file, p_abort));

            if (archive_new->Open (stream_new, 0, NULL) != S_OK)
                throw exception_arch_open ();

            close ();

            m_archive = archive_new;
            m_stream = stream_new;
        }
        catch (...) {
            throw;
        }

        m_timestamp = p_file->get_timestamp (p_abort);
        list_archive ();
    }

    void archive::open (const char *p_archive, abort_callback &p_abort)
    {
	    file_ptr file;

	    filesystem::g_open (file, p_archive, filesystem::open_mode_read, p_abort);
	    open (file, p_abort);
    }

    void archive::close ()
    {
        m_stream.Release ();

	    if (m_archive) {
		    if (m_archive->Close () != S_OK)
                throw exception_arch_close ();
            m_archive.Release ();
        } 
		
	    m_items.clear ();
	    m_timestamp = 0;
    }

    const t_filestats& archive::get_stats (const char *p_file) const
    {
	    auto pos = std::find_if (m_items.begin (), m_items.end (), [p_file] (const file_in_archive &p_item) { return pfc::stricmp_ascii (p_item.m_path, p_file) == 0; });
	    if (pos != m_items.end ())
		    return (*pos).m_stats;
	    else
		    throw exception_arch_file_not_found ();
    }

    void archive::get_reader_internal (t_size i, file_ptr &p_out, abort_callback &p_abort)
    {
	    p_out = new service_impl_t<tempmem_with_timestamp> (m_items[i].m_stats);

        CMyComPtr<IArchiveExtractCallback> archive_extract_callback (new extract_callback (p_out, p_abort));
	
        HRESULT result = m_archive->Extract (&i, 1, false, archive_extract_callback);
	    if (result != S_OK) {
		    error_log () << "Error extracting \"" << m_items[i].m_path << "\"";
		    throw exception_io_data ();
	    }
    
        p_out->reopen (p_abort);
    }

    void archive::get_reader (const char *p_file, file_ptr &p_out, abort_callback &p_abort)
    {
	    auto begin = m_items.begin (), end = m_items.end ();
	    auto pos = std::find_if (begin, end, [p_file] (const file_in_archive &p_item) { return pfc::stricmp_ascii (p_item.m_path, p_file) == 0; });
	    if (pos != end)
		    get_reader_internal (pos - begin, p_out, p_abort);
	    else
		    throw exception_arch_file_not_found ();
    }

    void archive::list_archive ()
    {
        UInt32 num_items = 0;
	    if (m_archive->GetNumberOfItems (&num_items) != S_OK) {
		    error_log () << "Couldn't get number of items in the archive";
		    throw exception_io_data ();
	    }

        m_items.reserve (num_items);

	    file_in_archive file_desc;
	    NWindows::NCOM::CPropVariant prop;

        for (UInt32 i = 0; i < num_items; i++) {
            m_archive->GetProperty (i, kpidPath, &prop);
            if (prop.vt != VT_BSTR) continue;
		    file_desc.m_path = pfc::stringcvt::string_utf8_from_wide (ConvertPropVariantToString (prop));

            m_archive->GetProperty (i, kpidSize, &prop);
            if (prop.vt != VT_UI8) continue;
            file_desc.m_stats.m_size = ConvertPropVariantToUInt64 (prop);
		    file_desc.m_stats.m_timestamp = m_timestamp;

            m_items.push_back (file_desc);
        }
    }
}