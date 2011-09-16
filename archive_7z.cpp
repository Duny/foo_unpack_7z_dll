#include "stdafx.h"

#include "Windows\PropVariant.h"
#include "Windows\PropVariantConversions.h"

#include "archive_7z.h"
#include "extract_callback.h"
#include "tempmem_with_timestamp.h"

typedef archive_items::iterator items_iterator;
typedef archive_items::const_iterator const_items_iterator;

class item_path_equal
{
	const char *m_file;
public:
	item_path_equal (const char *path) : m_file (path) {}

	bool operator () (const file_in_archive &p_item) {
		return pfc::stricmp_ascii (p_item.m_path, m_file) == 0;
	}
};

void archive_7z::open (file_ptr const &p_file, abort_callback &p_abort)
{
	close ();

	if (!CreateArchiveObject ((void **)&m_archive))
		throw std::exception ("Could't init 7z.dll");

	m_timestamp = p_file->get_timestamp (p_abort);

    foo_in_stream *fileSpec = new foo_in_stream (p_file, p_abort);
    CMyComPtr<IInStream> file = fileSpec;

    if (m_archive->Open (file, 0, NULL) != S_OK)
        throw exception_io_unsupported_format ();

	list_archive ();
}

void archive_7z::open (const char *p_archive, abort_callback &p_abort)
{
	file_ptr file;

	filesystem::g_open (file, p_archive, filesystem::open_mode_read, p_abort);
	open (file, p_abort);
}

void archive_7z::close ()
{
	if (m_archive)
		m_archive->Close ();
		
	m_items.clear ();
	m_timestamp = 0;
}

const t_filestats& archive_7z::get_stats (const char *p_file)
{
	items_iterator pos = std::find_if (m_items.begin (), m_items.end (), item_path_equal (p_file));
	if (pos != m_items.end ())
		return (*pos).m_stats;
	else
		throw exception_io_not_found ();
}

void archive_7z::list (item_callback &p_callback) {
	for (size_t i = 0, n = m_items.size (); i < n; i++)
		if (!p_callback.on_item (m_items[i]))
			break;
}

void archive_7z::get_reader (t_size i, file_ptr &p_out, abort_callback &p_abort)
{
	p_out = new service_impl_t<tempmem_with_timestamp> (m_items[i].m_stats);

    CMyComPtr<IArchiveExtractCallback> extract_callback (new foo_extract_callback (p_out, p_abort));
	
    HRESULT result = m_archive->Extract (&i, 1, false, extract_callback);
	if (result != S_OK) {
		show_error_message () << "Error extracting \"" << m_items[i].m_path << "\"";
		throw exception_io_data ();
	}
    
    p_out->reopen (p_abort);
}

void archive_7z::get_reader (const pfc::string8 &p_file, file_ptr &p_out, abort_callback &p_abort)
{
	const_items_iterator begin = m_items.begin (), end = m_items.end ();
	const_items_iterator pos = std::find_if (begin, end, item_path_equal (p_file));
	if (pos != end)
		get_reader (pos - begin, p_out, p_abort);
	else
		throw exception_io_not_found ();
}

void archive_7z::list_archive ()
{
    UInt32 num_items = 0;
	if (m_archive->GetNumberOfItems (&num_items) != S_OK) {
		show_error_message () << "Could't get number of items in the archive";
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

	/*file_ptr m_last_opened_file;
	pfc::string8 m_last_opened_file_path;
	critical_section m_sync;*/

	/*file_ptr file;
		archive_7z archive;
	
		const pfc::string8_fast &file_name = p_file;

		DWORD start = GetTickCount ();

		if (file_name == m_last_opened_file_path) {
			insync (m_sync);

			filesystem::g_open_tempmem (p_out, p_abort);
			m_last_opened_file->reopen (p_abort);
			file::g_transfer_file (m_last_opened_file, p_out, p_abort);
		}
		else {
			insync (m_sync);

			filesystem::g_open (file, p_archive, filesystem::open_mode_read, p_abort);
			archive.open (file, p_abort);

			const archive_items &items = archive.items ();
			
			t_size i = 0, n = items.size ();
			for (; i < n; i++) 
				if (items[i].m_path == file_name)
					break;
			
			if (i == n)
				throw exception_io_not_found ();
			else {
				archive.get_reader (i, p_out, p_abort);

				if (m_last_opened_file.is_valid ())
					m_last_opened_file.release ();
				filesystem::g_open_tempmem (m_last_opened_file, p_abort);

				file::g_transfer_file (p_out, m_last_opened_file, p_abort);
				m_last_opened_file_path = file_name;

				p_out->reopen (p_abort);
			}
		}*/