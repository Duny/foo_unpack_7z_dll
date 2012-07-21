#ifndef _FOO_UNPACK_7Z_DLL_UTILS_H_
#define _FOO_UNPACK_7Z_DLL_UTILS_H_

namespace unpack_7z
{
    using boost::tuple;
    using boost::tie;
    using boost::make_tuple;
    using boost::function;

    //
    // Plugin configuration
    namespace cfg
    {
        extern cfg_bool   debug_log;

        extern cfg_bool   use_sys_tmp_for_cache; // Using custom directory for disk cache (default is system temp folder)
        extern cfg_bool   keep_cache_at_exit; // Don't delete cache files, they will be reused on next foobar2000 launch. Work only with use_sys_tmp_for_cache = false
        extern cfg_string custom_cache_path;
        extern cfg_uint   file_cache_max; // Max allowed disk space use (in Mb)

        extern cfg_uint   archive_history_max; // Max number of archives remembered

        namespace defaults 
        {
            enum
            {
                debug_log = false,

                use_sys_tmp_for_cache = true,
                keep_cache_at_exit = false,
                file_cache_max = 0,

                archive_history_max = 1 // There must be at least one position (for current processing archive)
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
    struct create_guid : public GUID
	{
		create_guid (t_uint32 p_data1, t_uint16 p_data2, t_uint16 p_data3, t_uint8 p_data41, t_uint8 p_data42, t_uint8 p_data43, t_uint8 p_data44, t_uint8 p_data45, t_uint8 p_data46, t_uint8 p_data47, t_uint8 p_data48) 
		{
			Data1 = p_data1;
			Data2 = p_data2;
			Data3 = p_data3;
			Data4[0] = p_data41;
			Data4[1] = p_data42;
			Data4[2] = p_data43;
			Data4[3] = p_data44;
			Data4[4] = p_data45;
			Data4[5] = p_data46;
			Data4[6] = p_data47;
			Data4[7] = p_data48;
		}
	};

    inline t_uint64 hash (const char *str, t_uint64 h = 0)
    {
        while (*str) h = 31 * h + *str++;
        return h;
    }

    // Console logging helpers
    struct error_log : public pfc::string_formatter
    {
        ~error_log () { if (!is_empty()) console::formatter () << "Error(7z.dll unpacker):" << get_ptr (); }
    };

    struct debug_log : public pfc::string_formatter
    {
        ~debug_log () { if (cfg::debug_log && !is_empty ()) console::formatter () << "Debug(7z.dll unpacker):" << get_ptr (); }
    };

    // timer class
    class operation_timing_timer
    {
        pfc::string8_fast operation;
        CPerfTimer timer;
        pfc::string8 out_time_smart (double t);
    public:
        operation_timing_timer (const char * p_operation, BOOL start = TRUE);
        ~operation_timing_timer ();
        void stop ();
    };

    template<class T>
    inline void run_in_separate_thread (const T & func)
    {
        class thread_dynamic {
        public:
            PFC_DECLARE_EXCEPTION (exception_creation, pfc::exception, "Could not create thread");

            thread_dynamic (const T & func, int priority) : func (func), thread (INVALID_HANDLE_VALUE)
            {
                thread = CreateThread (NULL, 0, g_entry, reinterpret_cast<void*>(this), CREATE_SUSPENDED, NULL);
                if (thread == NULL) throw exception_creation ();
                SetThreadPriority (thread, priority);
                ResumeThread (thread);
            }

        private:
            // Must be instantiated with operator new
            ~thread_dynamic () { CloseHandle (thread); }

            void threadProc () { func (); delete this; }

            static DWORD CALLBACK g_entry (void* p_instance) { return reinterpret_cast<thread_dynamic*>(p_instance)->entry (); }
            unsigned entry () {
                try { threadProc (); }
                catch (...) {}
                return 0;
            }

            T func;
            HANDLE thread;

            PFC_CLASS_NOT_COPYABLE_EX (thread_dynamic)
        };

        new thread_dynamic (func, THREAD_PRIORITY_BELOW_NORMAL);
    }

    // wrapper of memory file with custom timestamp
    class tempmem_with_timestamp : public file
    {
        file_ptr m_file_mem;
        t_filestats m_stats;

    public:
        tempmem_with_timestamp (const t_filestats &stats) : m_stats (stats)
        { filesystem::g_open_tempmem (m_file_mem, abort_callback_dummy ()); }

        t_size read (void *p_buffer, t_size p_bytes, abort_callback &p_abort) override 
        { p_abort.check (); return m_file_mem->read (p_buffer, p_bytes, p_abort); }
        void write (const void *p_buffer, t_size p_bytes, abort_callback &p_abort) override 
        { p_abort.check (); m_file_mem->write (p_buffer, p_bytes, p_abort); }

        t_filesize get_size (abort_callback &p_abort) override 
        { return m_stats.m_size; }
        t_filesize get_position (abort_callback &p_abort) override 
        { p_abort.check (); return m_file_mem->get_position (p_abort); }
        void resize (t_filesize p_size, abort_callback &p_abort) override 
        { p_abort.check (); m_file_mem->resize (p_size, p_abort); m_stats.m_size = p_size; }
        void seek (t_filesize p_position, abort_callback &p_abort) override 
        { p_abort.check (); m_file_mem->seek (p_position, p_abort); }

        void on_idle (abort_callback &) override {}
        t_filetimestamp get_timestamp (abort_callback &p_abort) override { p_abort.check (); return m_stats.m_timestamp; }
        void reopen (abort_callback &p_abort) override { p_abort.check (); m_file_mem->reopen (p_abort); }

        bool is_in_memory () override { return true; }
        bool can_seek () override { return true; }
        bool get_content_type (pfc::string_base &) override { return false; }
        bool is_remote () override { return false; }
    };

    typedef service_impl_t<tempmem_with_timestamp> file_tempmem;
}

FB2K_STREAM_READER_OVERLOAD(t_filestats) { return stream >> value.m_size >> value.m_timestamp; }
FB2K_STREAM_WRITER_OVERLOAD(t_filestats) { return stream << value.m_size << value.m_timestamp; }  

#endif