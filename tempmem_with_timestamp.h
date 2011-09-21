#ifndef _FOO_UNPACK_7Z_DLL_TEMPMEM_WITH_TIMESTAMP_H_
#define _FOO_UNPACK_7Z_DLL_TEMPMEM_WITH_TIMESTAMP_H_

namespace unpack_7z
{
    class tempmem_with_timestamp : public file
    {
	    file_ptr m_file_mem;
	    t_filetimestamp m_timestamp;

    public:
	    tempmem_with_timestamp (const t_filetimestamp &timestamp) : m_timestamp (timestamp)
        { filesystem::g_open_tempmem (m_file_mem, abort_callback_dummy ()); }

	    t_size read (void *p_buffer, t_size p_bytes, abort_callback &p_abort) override 
        { return m_file_mem->read (p_buffer, p_bytes, p_abort); }
	    void write (const void *p_buffer, t_size p_bytes, abort_callback &p_abort) override 
        { m_file_mem->write (p_buffer, p_bytes, p_abort); }

	    t_filesize get_size (abort_callback &p_abort) override 
        { return m_file_mem->get_size (p_abort); }
	    t_filesize get_position (abort_callback &p_abort) override 
        { return m_file_mem->get_position (p_abort); }
	    void resize (t_filesize p_size, abort_callback &p_abort) override 
        { m_file_mem->resize (p_size, p_abort); }
	    void seek (t_filesize p_position, abort_callback &p_abort) override 
        { m_file_mem->seek (p_position, p_abort); }
	
	    void on_idle (abort_callback &) override {}
	    t_filetimestamp get_timestamp (abort_callback &) override { return m_timestamp; }
	    void reopen (abort_callback &p_abort) override { m_file_mem->reopen (p_abort); }

	    bool is_in_memory () override { return true; }
	    bool can_seek () override { return true; }
	    bool get_content_type (pfc::string_base &) override { return false; }
	    bool is_remote () override { return false; }
    };
}
#endif