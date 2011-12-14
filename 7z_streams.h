#ifndef _FOO_UNPACK_7Z_DLL_STREAMS_H_
#define _FOO_UNPACK_7Z_DLL_STREAMS_H_

namespace unpack_7z
{
    namespace file_streams
    {
        inline HRESULT execute_safe (const function<void ()> & code)
        {
            try { code (); return S_OK; } catch (...) { return S_FALSE; }
        }

        class in: 
            public IInStream,
            public IStreamGetSize,
            public CMyUnknownImp
        {
            file_ptr        m_file;
            abort_callback &m_abort;

        public:
            in (file_ptr const &p_file, abort_callback &p_abort) : m_file (p_file), m_abort (p_abort) {}

            MY_UNKNOWN_IMP2(IInStream, IStreamGetSize)

            // ISequentialInStream
            STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize) override
            {
                return execute_safe ([&] () {
                    UInt32 realProcessedSize = m_file->read (data, size, m_abort);
                    if (processedSize) *processedSize = realProcessedSize;
                });
            }

            // IInStream
            STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) override
            {
                return execute_safe ([&] () {
                    m_file->seek_ex (offset, (file::t_seek_mode)seekOrigin, m_abort);
                    if (newPosition) *newPosition = m_file->get_position (m_abort);
                });
            }

            // IStreamGetSize
            STDMETHOD(GetSize)(UInt64 *size) override
            {
                return execute_safe ([&] () { if (size) *size = m_file->get_size (m_abort); });
            }
        };

        class out:
            public IOutStream,
            public CMyUnknownImp
        {
            file_ptr        m_file;
            abort_callback &m_abort;

        public:
            out (const file_ptr &p_file, abort_callback &p_abort) : m_file (p_file), m_abort (p_abort) {}

            MY_UNKNOWN_IMP1(IOutStream)

            // ISequentialOutStream
            STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize) override
            {
                return execute_safe ([&] () {
                    m_file->write (data, size, m_abort);
                    if (processedSize) *processedSize = size;
                });
            }

            // IOutStream
            STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) override
            {
                return execute_safe ([&] () {
                    m_file->seek_ex (offset, (file::t_seek_mode)seekOrigin, m_abort);
                    if (newPosition) *newPosition = m_file->get_position (m_abort);
                });
            }

            STDMETHOD(SetSize)(UInt64 newSize) override
            {
                return execute_safe ([&] () { m_file->resize (newSize, m_abort); });
            }
        };
    }
}
#endif