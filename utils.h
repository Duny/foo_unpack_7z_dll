#ifndef _FOO_UNPACK_7Z_DLL_UTILS_H_
#define _FOO_UNPACK_7Z_DLL_UTILS_H_

namespace unpack_7z
{
    //
    // Plugin configuration
    namespace cfg
    {
        extern cfg_bool   debug_log;

        extern cfg_bool   use_sys_tmp_for_cache; // Using custom directory for disk cache (default is system temp folder)
        extern cfg_string custom_cache_path;
        extern cfg_uint   file_cache_max; // Max allowed disk space use (in Mb)

        extern cfg_uint   archive_history_max; // Max number of archives remembered

        namespace defaults 
        {
            enum
            {
                debug_log = false,

                use_sys_tmp_for_cache = true,
                file_cache_max = 100, // 100 Mb
                archive_history_max = 500
            };
        };
    }


#define COMMAND_ID_HANDLER_SIMPLE(id, func) \
	if(uMsg == WM_COMMAND && id == LOWORD(wParam)) \
	{ \
		bHandled = TRUE; \
		lResult = 0; \
        func (); \
	    return TRUE; \
	}

#define COMMAND_HANDLER_SIMPLE(id, code, func) \
    if(uMsg == WM_COMMAND && id == LOWORD(wParam) && code == HIWORD(wParam)) \
    { \
        bHandled = TRUE; \
        lResult = 0; \
        func (); \
        return TRUE; \
    }


    // helper to get "inline" GUID definitions
    // some_func (guid_inline<0xbfeaa7ea, 0x6810, 0x41c6, 0x82, 0x6, 0x12, 0x95, 0x5a, 0x89, 0xdf, 0x49>::guid);
    template <t_uint32 d1, t_uint16 d2, t_uint16 d3, t_uint8 d4, t_uint8 d5, t_uint8 d6, t_uint8 d7, t_uint8 d8, t_uint8 d9, t_uint8 d10, t_uint8 d11>
    struct guid_inline { static const GUID guid; };

    template <t_uint32 d1, t_uint16 d2, t_uint16 d3, t_uint8 d4, t_uint8 d5, t_uint8 d6, t_uint8 d7, t_uint8 d8, t_uint8 d9, t_uint8 d10, t_uint8 d11>
    __declspec (selectany) const GUID guid_inline<d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11>::guid = { d1, d2, d3, { d4, d5, d6, d7, d8, d9, d10, d11 } };


    inline t_uint64 hash (const char *str)
    {
        t_uint64 h = 0;
        while (*str) {
            /*unsigned c = 0;
            auto d = pfc::utf8_decode_char (str, c);
            if (d > 0) {
                h = 31 * h + uCharLower (c);
                str += d;
            }
            else break;*/
            h = 31 * h + *str++;
        }
        return h;
    }

    // Console logging helpers
    struct error_log : public pfc::string_formatter
    {
        ~error_log () { if (!is_empty()) console::formatter () << "Error(7z.dll unpacker):" << get_ptr (); }
    };

    struct debug_log : public pfc::string_formatter
    {
        ~debug_log () { if (!is_empty() && cfg::debug_log) console::formatter () << "Debug(7z.dll unpacker):" << get_ptr (); }
    };

    // timer class
    class operation_timer
    {
        pfc::string8_fast m_message;
        CPerfTimer m_timer;
    public:
        operation_timer (const char *p_message) : m_message (p_message), m_timer (TRUE) {}
        ~operation_timer () { m_timer.Stop (); console::formatter () << m_message << " took " << (int)m_timer.Elapsedus () << " microseconds\n"; }
    };

    // helper for creating GUID from any text using md5
    class GUID_from_text_md5 : public GUID
	{
	public:
		GUID_from_text_md5 (const char * text)
        {
            stream_writer_hasher_md5 hasher_md5;
            hasher_md5.write_string_raw (text, abort_callback_dummy ());
            GUID g = hasher_md5.resultGuid ();
            this->Data1 = g.Data1;
            this->Data2 = g.Data2;
            this->Data3 = g.Data3;
            t_size n = 8;
            while (n --> 0) this->Data4[n] = g.Data4[n];
        }
	};

    // wrapper of memory file with custom timestamp
    class tempmem_with_timestamp : public file
    {
        file_ptr m_file_mem;
        t_filestats m_stats;

    public:
        tempmem_with_timestamp (const t_filestats &stats) : m_stats (stats)
        { filesystem::g_open_tempmem (m_file_mem, abort_callback_dummy ()); }

        t_size read (void *p_buffer, t_size p_bytes, abort_callback &p_abort) override 
        { return m_file_mem->read (p_buffer, p_bytes, p_abort); }
        void write (const void *p_buffer, t_size p_bytes, abort_callback &p_abort) override 
        { m_file_mem->write (p_buffer, p_bytes, p_abort); }

        t_filesize get_size (abort_callback &p_abort) override 
        { return m_stats.m_size; }
        t_filesize get_position (abort_callback &p_abort) override 
        { return m_file_mem->get_position (p_abort); }
        void resize (t_filesize p_size, abort_callback &p_abort) override 
        { m_file_mem->resize (p_size, p_abort); m_stats.m_size = p_size; }
        void seek (t_filesize p_position, abort_callback &p_abort) override 
        { m_file_mem->seek (p_position, p_abort); }

        void on_idle (abort_callback &) override {}
        t_filetimestamp get_timestamp (abort_callback &) override { return m_stats.m_timestamp; }
        void reopen (abort_callback &p_abort) override { m_file_mem->reopen (p_abort); }

        bool is_in_memory () override { return true; }
        bool can_seek () override { return true; }
        bool get_content_type (pfc::string_base &) override { return false; }
        bool is_remote () override { return false; }
    };

    typedef service_impl_t<tempmem_with_timestamp> file_tempmem;
}
#endif