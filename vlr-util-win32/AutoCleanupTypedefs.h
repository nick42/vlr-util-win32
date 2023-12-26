#pragma once

#include <vlr-util/util.includes.h>
#include <vlr-util/ActionOnDestruction.h>

namespace vlr {

namespace win32 {

class AutoCloseRegKey : public CActionOnDestruction<LSTATUS>
{
	using BaseClass = CActionOnDestruction<LSTATUS>;

public:
	AutoCloseRegKey(HKEY& hKey)
		: BaseClass{ [&] { return ::RegCloseKey(hKey); } }
	{}
};

} // namespace win32

} // namespace vlr
