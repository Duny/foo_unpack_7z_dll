#include "stdafx.h"

#include "archive.h"
#include "dll.h"
#include "extract_callback.h"

namespace unpack_7z
{
    void archive::open (const char *p_archive, file_ptr const &p_file, abort_callback &p_abort, bool read_file_list)
    {
        CMyComPtr<IInArchive> new_archive (dll::create_archive_object ());
        CMyComPtr<IInStream> new_stream (new file_streams::in (p_file, p_abort));

        if (new_archive->Open (new_stream, 0, NULL) != S_OK)
            throw exception_arch_open ();

        this->close ();

        m_archive = new_archive;
        m_stream = new_stream;
        m_path = p_archive;

        m_timestamp = p_file->get_timestamp (p_abort);
        if (read_file_list)
            get_file_list ();
    }

    void archive::open (const char *p_archive, abort_callback &p_abort, bool read_file_list)
    {
	    file_ptr file;

	    filesystem::g_open (file, p_archive, filesystem::open_mode_read, p_abort);
	    this->open (p_archive, file, p_abort, read_file_list);
    }

    void archive::close ()
    {
        m_stream.Release ();

	    if (m_archive) {
		    if (m_archive->Close () != S_OK)
                throw exception_arch_close ();
            m_archive.Release ();
        } 
		
	    m_items.remove_all ();
	    m_timestamp = filetimestamp_invalid;
        m_path.reset ();
    }

    void archive::extract_file (const file_ptr &p_out, t_size i, abort_callback &p_abort) const
    {
        HRESULT res = m_archive->Extract (&i, 1, FALSE, CMyComPtr<IArchiveExtractCallback> (new extract_callback (p_out, p_abort)));
	    if (res != S_OK)
		    throw exception_arch_extract_error ();

        p_out->seek (0, p_abort);
    }

    // m_items must be clear before calling this
    void archive::get_file_list ()
    {
        if (m_items.get_size ()) return; // do not read twice

        UInt32 num_items = 0;
	    if (m_archive->GetNumberOfItems (&num_items) != S_OK)
		    throw exception_arch_num_items_error ();

	    file_info dummy;
	    NWindows::NCOM::CPropVariant prop;

        m_items.set_size (num_items);
        while (num_items --> 0) {
            m_archive->GetProperty (num_items, kpidPath, &prop);
            if (prop.vt != VT_BSTR) continue;
		    dummy.m_path = pfc::stringcvt::string_utf8_from_os (ConvertPropVariantToString (prop));

            m_archive->GetProperty (num_items, kpidSize, &prop);
            if (prop.vt != VT_UI8) continue;
            dummy.m_stats.m_size = ConvertPropVariantToUInt64 (prop);
		    dummy.m_stats.m_timestamp = m_timestamp;

            // precalculate unpack path
            archive_impl::g_make_unpack_path (dummy.m_unpack_path, m_path, dummy.m_path, _7Z_EXT);

            m_items[num_items] = dummy;
        }
    }
}