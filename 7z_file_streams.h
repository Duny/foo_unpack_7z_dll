#ifndef _FOO_UNPACK_7Z_DLL_FILE_STREAMS_H_
#define _FOO_UNPACK_7Z_DLL_FILE_STREAMS_H_

namespace unpack_7z
{
    namespace file_streams
    {
        class in: 
            public IInStream,
            public IStreamGetSize,
            public CMyUnknownImp
        {
            file_ptr        m_stream;
            abort_callback &m_abort;

        public:
            in (file_ptr const &p_stream, abort_callback &p_abort) : m_stream (p_stream), m_abort (p_abort) {}

            MY_UNKNOWN_IMP2(IInStream, IStreamGetSize)

            // ISequentialInStream
            STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize) override
            {
                try {
                    UInt32 realProcessedSize = m_stream->read (data, size, m_abort);
                    if (processedSize) *processedSize = realProcessedSize;
                    return S_OK;
                } catch (...) {
                    return S_FALSE;
                }
            }

            // IInStream
            STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) override
            {
                try {
                    if (seekOrigin >= 3) return STG_E_INVALIDFUNCTION;
                    m_stream->seek_ex (offset, (file::t_seek_mode)seekOrigin, m_abort);
                    if (newPosition) *newPosition = m_stream->get_position (m_abort);
                    return S_OK;
                } catch (...) {
                    return S_FALSE;
                }
            }

            // IStreamGetSize
            STDMETHOD(GetSize)(UInt64 *size) override
            {
                try {
                    if (size) *size = m_stream->get_size (m_abort);
                    return S_OK;
                } catch (...) {
                    return S_FALSE;
                }
            }
        };

        class out:
            public IOutStream,
            public CMyUnknownImp
        {
            file_ptr        m_stream;
            abort_callback &m_abort;

        public:
            out (const file_ptr &p_stream, abort_callback &p_abort) : m_stream (p_stream), m_abort (p_abort) {}

            MY_UNKNOWN_IMP1(IOutStream)

            // ISequentialOutStream
            STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize) override
            {
                try {
                    m_stream->write (data, size, m_abort);
                    if (processedSize) *processedSize = size;
                    return S_OK;
                } catch (...) {
                    return S_FALSE;
                }
            }

            // IOutStream
            STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) override
            {
                if (seekOrigin >= 3) return STG_E_INVALIDFUNCTION;
                try {
                    m_stream->seek_ex (offset, (file::t_seek_mode)seekOrigin, m_abort);
                    if (newPosition) *newPosition = m_stream->get_position (m_abort);
                    return S_OK;
                } catch (...) {
                    return S_FALSE;
                }
            }

            STDMETHOD(SetSize)(UInt64 newSize) override
            {
                try {
                    m_stream->resize (newSize, m_abort);
                    return S_OK;
                } catch (...) {
                    return S_FALSE;
                }
            }
        };
    }
}
#endif