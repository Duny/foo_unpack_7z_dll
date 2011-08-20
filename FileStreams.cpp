#include "stdafx.h"

//=================================================
// CFooInFileStream
//=================================================
STDMETHODIMP FooInStream::Read (void *data, UInt32 size, UInt32 *processedSize)
{
	try {
		UInt32 realProcessedSize = m_stream->read (data, size, m_abort);
		if (processedSize) *processedSize = realProcessedSize;
		return S_OK;
	} catch (std::exception const &) {
		return S_FALSE;
	}
}

STDMETHODIMP FooInStream::Seek (Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
	try {
		if (seekOrigin >= 3) return STG_E_INVALIDFUNCTION;

		m_stream->seek_ex (offset, (file::t_seek_mode)seekOrigin, m_abort); // Dirty hack
		if (newPosition) *newPosition = m_stream->get_position (m_abort);
		return S_OK;
	} catch (std::exception const &) {
		return S_FALSE;
	}
} 

STDMETHODIMP FooInStream::GetSize(UInt64 *size)
{
	try {
		if (size) *size = m_stream->get_size (m_abort);
		return S_OK;
	} catch(std::exception const &) {
		return S_FALSE;
	}
}

//=================================================
// CFooOutFileStream
//=================================================
STDMETHODIMP FooOutStream::Write (const void *data, UInt32 size, UInt32 *processedSize)
{
	try {
		m_stream->write (data, size, m_abort);
		if (processedSize) *processedSize = size;
		return S_OK;
	} catch(std::exception const &) {
		return S_FALSE;
	}
}

STDMETHODIMP FooOutStream::Seek (Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
	try {
		if (seekOrigin >= 3) return STG_E_INVALIDFUNCTION;
		m_stream->seek_ex (offset, (file::t_seek_mode)seekOrigin, m_abort);
		if (newPosition) *newPosition = m_stream->get_position (m_abort);
		return S_OK;
	} catch(std::exception const &) {
		return S_FALSE;
	}
}

STDMETHODIMP FooOutStream::SetSize (UInt64 newSize)
{
	try {
		m_stream->resize (newSize, m_abort);
		return S_OK;
	} catch(std::exception const &) {
		return S_FALSE;
	}
}