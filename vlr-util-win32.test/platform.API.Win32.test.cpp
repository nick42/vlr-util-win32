#include "pch.h"

#include <versionhelpers.h>

#include <vlr-util/ActionOnDestruction.h>

#include "vlr-util-win32/platform.API.Win32.h"

using namespace vlr;
using namespace vlr::win32;

struct TestPlatformAPI_Win32
	: public testing::Test
{
	bool VersionHasFunction_IsWow64Process2()
	{
		// Note: This mess curtosy of MS's infinite wisdom WRT Windows version information.
		// ... and only works if you have an appropriate manifest.

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
		bool bMeetsVersionRequirement = (VerifyVersionInfoW(&OSVersionInfo_Win10_1709, dwTypeMask, dwlConditionMask) != 0);

		return bMeetsVersionRequirement;
	}

	platform::API::CWin32 m_oPlatformAPI_Win32;
};

TEST_F(TestPlatformAPI_Win32, GetFunction_IsWow64Process2)
{
	if (!VersionHasFunction_IsWow64Process2())
	{
		return;
	}

	BOOL bResult{};

	auto& oFunction_IsWow64Process2 = m_oPlatformAPI_Win32.GetFunction_IsWow64Process2();
	ASSERT_EQ(oFunction_IsWow64Process2.m_srLoadResult, S_OK);
	ASSERT_NE(oFunction_IsWow64Process2.m_fFunction, nullptr);

	HANDLE hProcessToken = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
	ASSERT_NE(hProcessToken, nullptr);
	auto oOnDestroy_CloseProcessToken = MakeActionOnDestruction([&] {
		CloseHandle(hProcessToken);
	});

	USHORT nProcessMachine{};
	USHORT nNativeMachine{};
	bResult = oFunction_IsWow64Process2.m_fFunction(hProcessToken, &nProcessMachine, &nNativeMachine);
	EXPECT_EQ(bResult, TRUE);
}

TEST_F(TestPlatformAPI_Win32, TryLoadFunction)
{
	typedef BOOL (WINAPI *API_IsWow64Process)(
		/*[in]*/  HANDLE hProcess,
		/*[out]*/ PBOOL  Wow64Process);

	CDynamicLoadInfo_Function oLoadInfo;
	oLoadInfo.m_saFunctionName = "IsWow64Process";
	oLoadInfo.m_vecLibraryLoadInfo.emplace_back(_T("kernel32.dll"));

	SResult sr;

	SPCDynamicLoadedFunctionBase spDynamicLoadFunction;
	sr = m_oPlatformAPI_Win32.TryLoadFunction(oLoadInfo, spDynamicLoadFunction);
	ASSERT_EQ(sr, S_OK);
	ASSERT_EQ(spDynamicLoadFunction->m_srLoadResult, S_OK);
	ASSERT_NE(spDynamicLoadFunction, nullptr);
	ASSERT_NE(spDynamicLoadFunction->m_pvRawFunctionPointer, nullptr);

	auto fIsWow64Process = reinterpret_cast<API_IsWow64Process>(spDynamicLoadFunction->m_pvRawFunctionPointer);

	HANDLE hProcessToken = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
	ASSERT_NE(hProcessToken, nullptr);
	auto oOnDestroy_CloseProcessToken = vlr::MakeActionOnDestruction([&] {
		CloseHandle(hProcessToken);
	});

	BOOL bWow64Process{};
	BOOL bResult = fIsWow64Process(hProcessToken, &bWow64Process);
	EXPECT_EQ(bResult, TRUE);
}

TEST_F(TestPlatformAPI_Win32, TryLoadFunction_WithInvalidDll)
{
	typedef BOOL(WINAPI* API_IsWow64Process)(
		/*[in]*/  HANDLE hProcess,
		/*[out]*/ PBOOL  Wow64Process);

	CDynamicLoadInfo_Function oLoadInfo;
	oLoadInfo.m_saFunctionName = "IsWow64Process";
	oLoadInfo.m_vecLibraryLoadInfo.emplace_back(_T("invalid.dll"));
	oLoadInfo.m_vecLibraryLoadInfo.emplace_back(_T("kernel32.dll"));

	SResult sr;

	SPCDynamicLoadedFunctionBase spDynamicLoadFunction;
	sr = m_oPlatformAPI_Win32.TryLoadFunction(oLoadInfo, spDynamicLoadFunction);
	ASSERT_EQ(sr, S_OK);
	ASSERT_EQ(spDynamicLoadFunction->m_srLoadResult, S_OK);
	ASSERT_NE(spDynamicLoadFunction, nullptr);
	ASSERT_NE(spDynamicLoadFunction->m_pvRawFunctionPointer, nullptr);

	auto fIsWow64Process = reinterpret_cast<API_IsWow64Process>(spDynamicLoadFunction->m_pvRawFunctionPointer);

	HANDLE hProcessToken = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
	ASSERT_NE(hProcessToken, nullptr);
	auto oOnDestroy_CloseProcessToken = vlr::MakeActionOnDestruction([&] {
		CloseHandle(hProcessToken);
	});

	BOOL bWow64Process{};
	BOOL bResult = fIsWow64Process(hProcessToken, &bWow64Process);
	EXPECT_EQ(bResult, TRUE);
}

TEST_F(TestPlatformAPI_Win32, TryLoadFunction_ThenTypedLoad)
{
	if (!VersionHasFunction_IsWow64Process2())
	{
		return;
	}

	CDynamicLoadInfo_Function oLoadInfo;
	oLoadInfo.m_saFunctionName = "IsWow64Process2";
	oLoadInfo.m_vecLibraryLoadInfo.emplace_back(_T("kernel32.dll"));

	SResult sr;

	SPCDynamicLoadedFunctionBase spDynamicLoadFunction;
	sr = m_oPlatformAPI_Win32.TryLoadFunction(oLoadInfo, spDynamicLoadFunction);
	ASSERT_EQ(sr, S_OK);
	ASSERT_EQ(spDynamicLoadFunction->m_srLoadResult, S_OK);
	ASSERT_NE(spDynamicLoadFunction, nullptr);
	ASSERT_NE(spDynamicLoadFunction->m_pvRawFunctionPointer, nullptr);

	auto& oFunction_IsWow64Process2 = m_oPlatformAPI_Win32.GetFunction_IsWow64Process2();
	ASSERT_EQ(oFunction_IsWow64Process2.m_srLoadResult, S_OK);
	ASSERT_NE(oFunction_IsWow64Process2.m_fFunction, nullptr);

	HANDLE hProcessToken = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
	ASSERT_NE(hProcessToken, nullptr);
	auto oOnDestroy_CloseProcessToken = vlr::MakeActionOnDestruction([&] {
		CloseHandle(hProcessToken);
	});

	USHORT nProcessMachine{};
	USHORT nNativeMachine{};
	BOOL bResult = oFunction_IsWow64Process2.GetFunction()(hProcessToken, &nProcessMachine, &nNativeMachine);
	EXPECT_EQ(bResult, TRUE);
}
