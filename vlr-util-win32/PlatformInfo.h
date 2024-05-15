#pragma once

#include <mutex>

#include <vlr-util/util.Result.h>

namespace vlr {

namespace win32 {

class CPlatformInfo
{
public:
	static auto& GetSharedInstance()
	{
		static auto theInstance = CPlatformInfo{};
		return theInstance;
	}

protected:
	mutable std::mutex m_mutexDataAccess;

	SResult m_srPopulateResult_PlatformIs_Win64;
	bool m_bPlatformIs_Win64 = false;
	SResult PopulateValue_PlatformIs_Win64();

public:
	bool GetValue_PlatformIs_Win64();

};

} // namespace win32

} // namespace vlr
