#ifndef _FILE_STREAMS_H_
#define _FILE_STREAMS_H_

#include "7zip\Common\FileStreams.h"

class foo_in_stream:
    public IInStream,
    public IStreamGetSize,
    public CMyUnknownImp
{
public:
    foo_in_stream (file_ptr const &p_stream, abort_callback &p_abort) : m_stream (p_stream), m_abort (p_abort) {}

    MY_UNKNOWN_IMP2(IInStream, IStreamGetSize)

    STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
    STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);

    STDMETHOD(GetSize)(UInt64 *size);
private:
    abort_callback &m_abort;
    file_ptr	    m_stream;
};

class foo_out_stream:
    public IOutStream,
    public CMyUnknownImp
{
public:
    foo_out_stream (file_ptr &p_stream, abort_callback &p_abort) : m_stream (p_stream), m_abort (p_abort) {}

    MY_UNKNOWN_IMP1(IOutStream)

    STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
    STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);
    STDMETHOD(SetSize)(UInt64 newSize);

private:
    abort_callback &m_abort;
    file_ptr        m_stream;
};

#endif