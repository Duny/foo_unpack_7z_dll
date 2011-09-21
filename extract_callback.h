#ifndef _FOO_UNPACK_7Z_DLL_EXTRACT_CALLBACK_H_
#define _FOO_UNPACK_7Z_DLL_EXTRACT_CALLBACK_H_

#include "file_streams.h"

namespace unpack_7z
{
    class extract_callback : public IArchiveExtractCallback, public CMyUnknownImp
    {
    public:
        MY_UNKNOWN_IMP1(IArchiveOpenCallback)

        // IProgress
	    STDMETHOD(SetTotal)(UInt64 size) override { return S_OK; }
        STDMETHOD(SetCompleted)(const UInt64 *completeValue) override { return S_OK; }

        // IArchiveExtractCallback
        STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode) override;
        STDMETHOD(PrepareOperation)(Int32 askExtractMode) override { return S_OK; }
        STDMETHOD(SetOperationResult)(Int32 resultEOperationResult) override;

        extract_callback (file_ptr &p_stream, abort_callback &p_abort) : m_abort (p_abort), m_stream (p_stream) {}

    private:
        abort_callback &m_abort;
        file_ptr        m_stream;

        streams::out *m_out_stream_spec;
        CMyComPtr<ISequentialOutStream> m_out_file_stream; 
    };
}
#endif