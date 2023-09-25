#pragma once

#include <vlr-util/util.includes.h>
#include <vlr-util/ActionOnDestruction.h>

VLR_NAMESPACE_BEGIN(vlr)

VLR_NAMESPACE_BEGIN(win32)

class AutoCloseRegKey : public CActionOnDestruction<LSTATUS>
{
	using BaseClass = CActionOnDestruction<LSTATUS>;

public:
	AutoCloseRegKey(HKEY& hKey)
		: BaseClass{ [&] { return ::RegCloseKey(hKey); } }
	{}
};

VLR_NAMESPACE_END //( win32 )

VLR_NAMESPACE_END //( vlr )
