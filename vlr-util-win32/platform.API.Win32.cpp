#include "pch.h"
#include "platform.API.Win32.h"

#include "platform.DynamicLoadProc.h"

namespace vlr {

namespace win32 {

namespace platform {

namespace API {

const Win32::F_IsWow64Process2& CWin32::GetFunction_IsWow64Process2()
{
	static const auto _tFailureValue = Win32::F_IsWow64Process2{};

	auto slDataAccess = std::scoped_lock{ m_mutexDataAccess };

	static constexpr vlr::tzstring_view svzFunctionName = _T("IsWow64Process2");
	static constexpr vlr::tzstring_view svzLibraryName = _T("Kernel32.dll");

	if (m_spIsWow64Process2)
	{
		return *m_spIsWow64Process2;
	}

	SResult sr;

	CDynamicLoadInfo_Function oLoadInfo;
	oLoadInfo.m_vecLibraryLoadInfo.push_back(CDynamicLoadInfo_Library{ svzLibraryName });
	oLoadInfo.m_sFunctionName = svzFunctionName.toStdString();

	// We want the typed function version in the map, so we create and pass in
	m_spIsWow64Process2 = std::make_shared<Win32::F_IsWow64Process2>();

	SPCDynamicLoadedFunctionBase spDynamicLoadFunction;
	sr = CDynamicLoadProc::GetSharedInstance().TryPopulateFunction(oLoadInfo, spDynamicLoadFunction, m_spIsWow64Process2);
	if (sr != SResult::Success)
	{
		return _tFailureValue;
	}
	// Note: If we succeeded, we should have the non-null value in our variable now
	VLR_ASSERT_NONZERO_OR_RETURN_FAILURE_VALUE(spDynamicLoadFunction);
	VLR_ASSERT_COMPARE_OR_RETURN_FAILURE_VALUE(spDynamicLoadFunction.get(), == , m_spIsWow64Process2.get());

	return *m_spIsWow64Process2;
}

} // namespace API

} // namespace platform

} // namespace win32

} // namespace vlr
