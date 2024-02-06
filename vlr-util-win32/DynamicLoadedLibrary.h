#pragma once

#include <vlr-util/util.Result.h>
#include <vlr-util/util.std_aliases.h>

namespace vlr {

namespace win32 {

class CDynamicLoadedLibrary
{
public:
	vlr::tstring m_sLoadName;
	SResult m_srLoadResult;
	HMODULE m_hLibrary{};
};

} // namespace win32

} // namespace vlr
