#ifndef _EXTRACT_CALLBACK_H_
#define _EXTRACT_CALLBACK_H_

#include "file_streams.h"

class foo_extract_callback : public IArchiveExtractCallback, public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP1(IArchiveOpenCallback)

    // IProgress
	STDMETHOD(SetTotal)(UInt64 size) { return S_OK; }
    STDMETHOD(SetCompleted)(const UInt64 *completeValue) { return S_OK; }

    // IArchiveExtractCallback
    STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode);
    STDMETHOD(PrepareOperation)(Int32 askExtractMode)  { return S_OK; }
    STDMETHOD(SetOperationResult)(Int32 resultEOperationResult);

    foo_extract_callback (file_ptr &p_stream, abort_callback &p_abort) : m_abort (p_abort), m_stream (p_stream) {}

private:
    abort_callback &m_abort;
    file_ptr        m_stream;

    foo_out_stream *m_out_stream_spec;
    CMyComPtr<ISequentialOutStream> m_out_file_stream; 
};

#endif