#ifndef _DISK_CACHE_H_
#define _DISK_CACHE_H_

namespace unpack_7z
{
    namespace disk_cache
    {
        // do not reimplement; instantiate using static_api_ptr_t<manager>
        class NOVTABLE manager : public service_base {
        public:
            virtual void func1 () = 0;
            FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(manager)
        }; 
    }
}
#endif