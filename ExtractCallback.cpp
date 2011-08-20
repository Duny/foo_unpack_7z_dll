#include "stdafx.h"

STDMETHODIMP CFooExtractCallback::GetStream (UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode)
{
    *outStream = 0;
    m_outFileStream.Release ();

    if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
        return S_OK;

    m_outFileStreamSpec = new FooOutStream (m_stream, m_abort);

    CMyComPtr<ISequentialOutStream> outStreamLoc (m_outFileStreamSpec);
   
    m_outFileStream = outStreamLoc;
    *outStream = outStreamLoc.Detach ();
    
    return S_OK;
}

STDMETHODIMP CFooExtractCallback::SetOperationResult (Int32 operationResult)
{
    m_outFileStream.Release ();

    return S_OK;
}