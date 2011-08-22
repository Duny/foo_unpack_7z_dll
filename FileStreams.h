#pragma once

class FooInStream:
    public IInStream,
    public IStreamGetSize,
    public CMyUnknownImp
{
public:
    FooInStream (file_ptr const &p_stream, abort_callback &p_abort) : m_stream (p_stream), m_abort (p_abort) {}

    MY_UNKNOWN_IMP2(IInStream, IStreamGetSize)

    STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
    STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);

    STDMETHOD(GetSize)(UInt64 *size);
private:
    abort_callback &m_abort;
    file_ptr	    m_stream;
};

class FooOutStream:
    public IOutStream,
    public CMyUnknownImp
{
public:
    FooOutStream (file_ptr &p_stream, abort_callback &p_abort) : m_stream (p_stream), m_abort (p_abort) {}

    MY_UNKNOWN_IMP1(IOutStream)

    STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
    STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);
    STDMETHOD(SetSize)(UInt64 newSize);

private:
    abort_callback &m_abort;
    file_ptr        m_stream;
};