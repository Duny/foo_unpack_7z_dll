#include "stdafx.h"

namespace unpack_7z
{
    operation_timing_timer::operation_timing_timer (const char * p_operation, BOOL start) :
        operation (p_operation), 
        timer (start)
    {
        return;
    }

    operation_timing_timer::~operation_timing_timer ()
    {
        timer.Stop ();
        debug_log () << operation << " took " << out_time_smart (timer.Elapsedus ());
    }
    
    pfc::string8 operation_timing_timer::out_time_smart (double t)
    {
        if (t < 1000.0)
        {
            return pfc::string_formatter () << pfc::format_float (t, 0, 0) << " us";
        }
        else if (t < 1000000.0)
        {
            return pfc::string_formatter () << pfc::format_float (t / 1000.0, 0, 1) << " ms";
        }
        else
            return pfc::string_formatter () << pfc::format_float (t / 1000.0 / 1000.0, 0, 3) << " s";
    }

    void operation_timing_timer::stop ()
    {
        timer.Stop ();
    }
}