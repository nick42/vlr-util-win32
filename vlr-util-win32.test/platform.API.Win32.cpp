#include "pch.h"

#include <versionhelpers.h>

#include <vlr-util/ActionOnDestruction.h>

#include "vlr-util-win32/platform.API.Win32.h"

TEST(Win32, IsWow64Process2)
{
	// Note: This preamble-mess curtosy of MS's infinite wisdom WRT Windows version information.

	static auto OSVersionInfo_Win10_1709 = []() {
		OSVERSIONINFOEX osVersionInfo{};
		osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		osVersionInfo.dwMajorVersion = 10;
		osVersionInfo.dwMinorVersion = 0;
		osVersionInfo.dwBuildNumber = 16299;
		return osVersionInfo;
	}();
	static const DWORD dwTypeMask = VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER;
	static const DWORDLONG dwlConditionMask = []() {
		DWORDLONG dwlValue = 0;
		dwlValue = VerSetConditionMask(dwlValue, VER_MAJORVERSION, VER_GREATER_EQUAL);
		dwlValue = VerSetConditionMask(dwlValue, VER_MINORVERSION, VER_GREATER_EQUAL);
		dwlValue = VerSetConditionMask(dwlValue, VER_BUILDNUMBER, VER_GREATER_EQUAL);
		return dwlValue;
	}();
	if (VerifyVersionInfoW(&OSVersionInfo_Win10_1709, dwTypeMask, dwlConditionMask))
	{
		return;
	}

	BOOL bResult{};

	auto& oFunction_IsWow64Process2 = vlr::win32::platform::API::CWin32::GetSharedInstance().GetFunction_IsWow64Process2();
	ASSERT_EQ(oFunction_IsWow64Process2.m_srLoadResult, S_OK);
	ASSERT_NE(oFunction_IsWow64Process2.m_fFunction, nullptr);

	HANDLE hProcessToken = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
	ASSERT_NE(hProcessToken, nullptr);
	auto oOnDestroy_CloseProcessToken = vlr::MakeActionOnDestruction([&] {
		CloseHandle(hProcessToken);
	});

	USHORT nProcessMachine{};
	USHORT nNativeMachine{};
	bResult = oFunction_IsWow64Process2.m_fFunction(hProcessToken, &nProcessMachine, &nNativeMachine);
	EXPECT_EQ(bResult, TRUE);
}
