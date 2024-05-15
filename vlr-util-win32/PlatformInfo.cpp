#include "pch.h"
#include "PlatformInfo.h"

#include <vlr-util/ActionOnDestruction.h>
#include <vlr-util/ModuleContext.Compilation.h>

namespace vlr {

namespace win32 {

SResult CPlatformInfo::PopulateValue_PlatformIs_Win64()
{
	auto slDataAccess = std::scoped_lock{ m_mutexDataAccess };

	if constexpr (vlr::ModuleContext::Compilation::IsBuildPlatform_Win64())
	{
		m_bPlatformIs_Win64 = true;
		m_srPopulateResult_PlatformIs_Win64 = S_OK;
		return S_OK;
	}

	HANDLE hProcessToken = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(hProcessToken);
	auto oOnDestroy_CloseProcessToken = vlr::MakeActionOnDestruction([&] {
		CloseHandle(hProcessToken);
	});

	BOOL bWow64Process{};
	BOOL bResult = IsWow64Process(hProcessToken, &bWow64Process);
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(bResult);
	if (bWow64Process)
	{
		m_bPlatformIs_Win64 = true;
		m_srPopulateResult_PlatformIs_Win64 = S_OK;
		return S_OK;
	}

	// TODO? Add support for IsWow64Process2

	m_bPlatformIs_Win64 = false;
	m_srPopulateResult_PlatformIs_Win64 = S_OK;

	return S_OK;
}

bool CPlatformInfo::GetValue_PlatformIs_Win64()
{
	if (m_srPopulateResult_PlatformIs_Win64.isSet())
	{
		return m_bPlatformIs_Win64;
	}

	PopulateValue_PlatformIs_Win64();

	return m_bPlatformIs_Win64;
}

} // namespace win32

} // namespace vlr
