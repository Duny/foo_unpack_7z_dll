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
                } catch (...) { return S_FALSE; }
                return S_OK;
            }

            // IInStream
            STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) override
            {
                if (seekOrigin >= 3) return STG_E_INVALIDFUNCTION;
                try {
                    m_stream->seek_ex (offset, (file::t_seek_mode)seekOrigin, m_abort);
                    if (newPosition) *newPosition = m_stream->get_position (m_abort);
                } catch (...) { return S_FALSE; }
                return S_OK;
            }

            // IStreamGetSize
            STDMETHOD(GetSize)(UInt64 *size) override
            {
                try {
                    if (size) *size = m_stream->get_size (m_abort);
                } catch (...) { return S_FALSE; }
                return S_OK;
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
                } catch (...) { return S_FALSE; }
                return S_OK;
            }

            // IOutStream
            STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) override
            {
                if (seekOrigin >= 3) return STG_E_INVALIDFUNCTION;
                try {
                    m_stream->seek_ex (offset, (file::t_seek_mode)seekOrigin, m_abort);
                    if (newPosition) *newPosition = m_stream->get_position (m_abort);
                } catch (...) { return S_FALSE; }
                return S_OK;
            }

            STDMETHOD(SetSize)(UInt64 newSize) override
            {
                try {
                    m_stream->resize (newSize, m_abort);
                } catch (...) { return S_FALSE; }
                return S_OK;
            }
        };
    }
}
#endif