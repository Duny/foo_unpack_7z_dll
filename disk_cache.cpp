#include "stdafx.h"

#include "disk_cache.h"

namespace unpack_7z
{
    namespace disk_cache
    {
        // {309B1505-2792-4BC4-A4CB-6BC4768DD296}
        const GUID manager::class_guid = 
        { 0x309b1505, 0x2792, 0x4bc4, { 0xa4, 0xcb, 0x6b, 0xc4, 0x76, 0x8d, 0xd2, 0x96 } };

        class manager_impl : public manager
        {
            void func1 ()
            {
            }
        };
        static service_factory_single_t<manager_impl> g_manager_impl_factory;
    }
}