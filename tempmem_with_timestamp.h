#pragma once

class tempmem_with_timestamp : public file
{
	file_ptr m_file_mem;
	t_filestats m_stats;

public:
	tempmem_with_timestamp (const t_filestats & stats) : m_stats (stats) {
		filesystem::g_open_tempmem (m_file_mem, abort_callback_dummy ());
	}

	virtual t_size read (void *p_buffer, t_size p_bytes, abort_callback &p_abort) { return m_file_mem->read (p_buffer, p_bytes, p_abort); }
	virtual void write (const void *p_buffer, t_size p_bytes, abort_callback &p_abort) { m_file_mem->write (p_buffer, p_bytes, p_abort); }

	virtual t_filesize get_size (abort_callback &p_abort) { return m_file_mem->get_size (p_abort); }
	virtual t_filesize get_position (abort_callback &p_abort) { return m_file_mem->get_position (p_abort); }
	virtual void resize (t_filesize p_size, abort_callback &p_abort) { m_file_mem->resize (p_size, p_abort); }
	virtual void seek (t_filesize p_position, abort_callback &p_abort) { m_file_mem->seek (p_position, p_abort); }
	
	virtual void on_idle (abort_callback &) {}
	virtual t_filetimestamp get_timestamp (abort_callback &) { return m_stats.m_timestamp; }
	virtual void reopen (abort_callback &p_abort) { m_file_mem->reopen (p_abort); }

	virtual bool is_in_memory () { return true; }
	virtual bool can_seek () { return true; }
	virtual bool get_content_type (pfc::string_base &) { return false; }
	virtual bool is_remote () { return false; }
};
