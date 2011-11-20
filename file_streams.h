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
        public:
            in (file_ptr const &p_stream, abort_callback &p_abort) : m_stream (p_stream), m_abort (p_abort) {}

            MY_UNKNOWN_IMP2(IInStream, IStreamGetSize)

            // ISequentialInStream
            STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize) override;
            // IInStream
            STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) override;
            // IStreamGetSize
            STDMETHOD(GetSize)(UInt64 *size) override;

        private:
            file_ptr m_stream;
            abort_callback &m_abort;
        };

        class out:
            public IOutStream,
            public CMyUnknownImp
        {
        public:
            out (const file_ptr &p_stream, abort_callback &p_abort) : m_stream (p_stream), m_abort (p_abort) {}

            MY_UNKNOWN_IMP1(IOutStream)

            // ISequentialOutStream
            STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize) override;
            // IOutStream
            STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) override;
            STDMETHOD(SetSize)(UInt64 newSize) override;

        private:
            file_ptr m_stream;
            abort_callback &m_abort;
        };
    }
}
#endif