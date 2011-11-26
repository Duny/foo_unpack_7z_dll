#pragma once

#define COMPONENT_NAME "foo_unpack_7z_dll"


// crt includes
#include <vector>
#include <algorithm>

// foobar2000 includes
#include "foobar2000/ATLHelpers/ATLHelpers.h"

// 7-zip includes
#include "Common/MyInitGuid.h"
#include "Common/MyCom.h"
#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConversions.h"
#include "Common/StringConvert.h"

// boost includes
#include "boost/function.hpp"

// plugin includes
#include "resource.h"
#include "utils.h"
#include "archive.h"
#include "cache_system.h"

#define _7Z_EXT "7z"