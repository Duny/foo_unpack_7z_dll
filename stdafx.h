#pragma once

#define COMPONENT_NAME "foo_unpack_7z_dll"


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
#include <boost/function.hpp>
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/scope_exit.hpp>

// plugin includes
#include "perf_timer.h"
#include "utils.h"

#define _7Z_EXT "7z"