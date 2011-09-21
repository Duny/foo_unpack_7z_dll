#ifndef _FOO_UNPACK_7Z_DLL_EXTRACT_CALLBACK_H_
#define _FOO_UNPACK_7Z_DLL_EXTRACT_CALLBACK_H_

#include "file_streams.h"

namespace unpack_7z
{
    class extract_callback : public IArchiveExtractCallback, public CMyUnknownImp
    {
    public:
        extract_callback (file_ptr &p_stream, abort_callback &p_abort)
            : m_stream (new streams::out (p_stream, p_abort)) { }

        MY_UNKNOWN_IMP1(IArchiveOpenCallback)


        // IProgress
	    STDMETHOD(SetTotal)(UInt64 size) override { return S_OK; }
        STDMETHOD(SetCompleted)(const UInt64 *completeValue) override { return S_OK; }

        // IArchiveExtractCallback
        STDMETHOD(PrepareOperation)(Int32 askExtractMode) override { return S_OK; }
        STDMETHOD(SetOperationResult)(Int32 resultEOperationResult) override { return S_OK; };

        STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode) override
        {
            *outStream = 0;

            if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
                return S_OK;

            *outStream = m_stream.Detach ();

            return S_OK;
        }

    private:
        CMyComPtr<ISequentialOutStream> m_stream; 
    };
}
#endif