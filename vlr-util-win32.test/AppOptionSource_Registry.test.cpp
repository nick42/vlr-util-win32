#include "pch.h"
#include "vlr-util-win32/AppOptionSource_Registry.h"

#include <gtest/gtest.h>

using namespace vlr;
using namespace vlr::win32;

TEST(AppOptionSource_Registry, ReadAllValuesFromPathAsOptions)
{
	SResult sr;

	CAppOptions oAppOptions;

	auto oReg = CRegistryAccess{ HKEY_LOCAL_MACHINE };

	CAppOptionSource_Registry oAppOptionSource_Registry;
	oAppOptionSource_Registry.withAppOptionsOverride(&oAppOptions);

	sr = oAppOptionSource_Registry.ReadAllValuesFromPathAsOptions(oReg, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion"));
	EXPECT_EQ(sr, S_OK);
	// This should have some values
	EXPECT_GE(oAppOptions.GetCount_SpecifiedValues(), 5U);

	// Look for something which should be there...
	{
		SPCAppOptionSpecifiedValue spSpecifiedValue;
		sr = oAppOptions.FindSpecifiedValueByName(_T("ProgramFilesDir"), spSpecifiedValue);
		EXPECT_EQ(sr, S_OK);
		EXPECT_NE(spSpecifiedValue, nullptr);
		// Note: Value type access tested elsewhere, not testing here
	}

	// Paths which do not exist should not fail, but return S_FALSE (or equivalent)
	{
		auto nCurrentCountSpecifiedValues = oAppOptions.GetCount_SpecifiedValues();

		sr = oAppOptionSource_Registry.ReadAllValuesFromPathAsOptions(oReg, _T("SOFTWARE\\BlahBlah\\NotThere"));
		EXPECT_TRUE(sr.isSuccess());
		EXPECT_NE(sr, S_OK);
		// Should not have modified the specified value count
		EXPECT_EQ(oAppOptions.GetCount_SpecifiedValues(), nCurrentCountSpecifiedValues);
	}
}
