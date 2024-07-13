#pragma once

#include <vlr-util/util.Result.h>
#include <vlr-util/util.std_aliases.h>

#include "DynamicLoadInfo_Library.h"

namespace vlr {

namespace win32 {

class CDynamicLoadInfo_Function
{
public:
	vlr::tstring m_sFunctionName;
	std::vector<CDynamicLoadInfo_Library> m_vecLibraryLoadInfo;
};

} // namespace win32

} // namespace vlr
