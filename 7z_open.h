#ifndef __7Z_H
#define __7Z_H

#include <fstream>
#include <vector>
#include <time.h>

#include <foobar2000.h>

#include "MyInitGuid.h"
#include "StringConvert.h"
#include "IntToString.h"

#include "PropVariant.h"
#include "PropVariantConversions.h"
#include "DLL.h"
#include "FileDir.h"
#include "FileName.h"
#include "FileFind.h"

#include "FileStreams.h"
#include "IArchive.h"
#include "IPassword.h"
#include "MyVersion.h"

#define _PFC_DECLARE_EXCEPTION(NAME,BASECLASS,DEFAULTMSG)	\
class NAME : public BASECLASS {	\
public:	\
	NAME() : BASECLASS(DEFAULTMSG) {}	\
	NAME(const char * p_msg) : BASECLASS(p_msg) {}	\
	NAME(const NAME & p_source) : BASECLASS(p_source) {}	\
};

namespace C7Z
{
    PFC_DECLARE_EXCEPTION( exception_7z_no_create, exception_io_data, "7z could not be created" );
    _PFC_DECLARE_EXCEPTION( exception_7z_no_mem_alloc, std::bad_alloc, "Memory for 7z could not be allocated" );
    PFC_DECLARE_EXCEPTION( exception_7z_no_open, exception_io_data, "7z could not be opened" );
    PFC_DECLARE_EXCEPTION( exception_7z_bad_file, exception_io_data, "Invalid/Corrupt 7z" );
    PFC_DECLARE_EXCEPTION( exception_7z_dll_init_failed, exception_io_data, "7z library cannot be initilized" );
    PFC_DECLARE_EXCEPTION( exception_7z_unsupported_version, exception_io_data, "7z version not supported" );
    PFC_DECLARE_EXCEPTION( exception_7z_compress_failed, exception_io_data, "7z compression failed" );
    PFC_DECLARE_EXCEPTION( exception_7z_decompress_failed, exception_io_data, "7z decompression failed" );
    PFC_DECLARE_EXCEPTION( exception_7z_file_not_found, exception_io_data, "File not found in 7z" );

    bool IsSupportedType (const char *ext);

    struct c7z_file_info {
        t_filestats  stats;
        pfc::string8 name;
    };

    class c7z_open
    {
    public:
        c7z_open (const char *archive_type="7z");
        ~c7z_open () { close (); }

        void open_archive (file_ptr const &, abort_callback &);
        void close ();

        t_filestats get_stats_in_archive (const char *p_file, abort_callback &p_abort);
        void open_file_in_archive (file_ptr &p_out, pfc::string8 file, abort_callback &p_abort);

        pfc::array_t<c7z_file_info> files;

    private:
        void read_file_list ();

        CMyComPtr<IInArchive>  archive7z;
    };
}
#endif
