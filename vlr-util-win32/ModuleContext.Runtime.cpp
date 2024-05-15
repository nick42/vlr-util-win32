#include "pch.h"
#include "ModuleContext.Runtime.h"

#include "PlatformInfo.h"

namespace vlr {

namespace ModuleContext {

namespace Runtime {

bool NativePlatformIs_64bit()
{
	return vlr::win32::CPlatformInfo::GetSharedInstance().GetValue_PlatformIs_Win64();
}

} // namespace Runtime

} // namespace ModuleContext

} // namespace vlr
