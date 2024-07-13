#include "pch.h"

#include "vlr-util-win32/platform.DynamicLoadProc.h"

using namespace vlr;
using namespace vlr::win32;

struct TestDynamicLoadProc
	: public testing::Test
{
	platform::CDynamicLoadProc m_oDynamicLoadProc;
};

TEST_F(TestDynamicLoadProc, LoadLibrary_PreLoaded_Success)
{
	CDynamicLoadInfo_Library oLoadInfo;
	oLoadInfo.withLibraryName(_T("kernel32.dll"));
	oLoadInfo.withExpectLibraryAlreadyLoaded(true);

	const CDynamicLoadedLibrary& oDynamicLoadedLibrary = m_oDynamicLoadProc.GetDynamicLoadLibrary(oLoadInfo);

	EXPECT_EQ(oDynamicLoadedLibrary.m_srLoadResult, S_OK);
	EXPECT_NE(oDynamicLoadedLibrary.m_hLibrary, HMODULE{});
}

TEST_F(TestDynamicLoadProc, LoadLibrary_PreLoaded_Failure)
{
	// Note: Need to pick something which should exist in the load path, but isn't already loaded, to
	// verify that we're not actually loading with the flag set.

	CDynamicLoadInfo_Library oLoadInfo;
	oLoadInfo.withLibraryName(_T("activeds.dll"));
	oLoadInfo.withExpectLibraryAlreadyLoaded(true);

	const CDynamicLoadedLibrary& oDynamicLoadedLibrary = m_oDynamicLoadProc.GetDynamicLoadLibrary(oLoadInfo);

	EXPECT_NE(oDynamicLoadedLibrary.m_srLoadResult, S_OK);
	EXPECT_EQ(oDynamicLoadedLibrary.m_hLibrary, HMODULE{});
}
