#include "stdafx.h"

#include "7zip\Archive\IArchive.h"
#include "extract_callback.h"

STDMETHODIMP foo_extract_callback::GetStream (UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode)
{
    *outStream = 0;
    m_out_file_stream.Release ();

    if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
        return S_OK;

    m_out_stream_spec = new foo_out_stream (m_stream, m_abort);

    CMyComPtr<ISequentialOutStream> outStreamLoc (m_out_stream_spec);
   
    m_out_file_stream = outStreamLoc;
    *outStream = outStreamLoc.Detach ();
    
    return S_OK;
}

STDMETHODIMP foo_extract_callback::SetOperationResult (Int32 operationResult)
{
    m_out_file_stream.Release ();

    return S_OK;
}