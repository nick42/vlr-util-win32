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
	EXPECT_GE(oAppOptions.GetCount_SpecifiedValues(), 5);

	// Look for something which should be there...
	{
		SPCAppOptionSpecifiedValue spSpecifiedValue;
		sr = oAppOptions.FindSpecifiedValueByName(_T("ProgramFilesDir"), spSpecifiedValue);
		EXPECT_EQ(sr, S_OK);
		EXPECT_NE(spSpecifiedValue, nullptr);
	}
}
