
class CFooExtractCallback : public IArchiveExtractCallback, public CMyUnknownImp
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

    CFooExtractCallback (file_ptr &p_stream, abort_callback &p_abort) : m_abort (p_abort), m_stream (p_stream) {}

private:
    abort_callback &m_abort;
    file_ptr        m_stream;

    FooOutStream *m_outFileStreamSpec;
    CMyComPtr<ISequentialOutStream> m_outFileStream; 
};
