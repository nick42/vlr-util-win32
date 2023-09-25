#include "pch.h"

#include <fmt/format.h>

#include "vlr-util-win32/RegistryAccess.h"

using namespace vlr;
using namespace vlr::win32;

static const auto svzBaseKey_Test = tzstring_view{ _T("SOFTWARE\\vlr-test") };
static const auto svzBaseKey_Invalid = tzstring_view{ _T("SOFTWARE\\vlr-test-invalid") };

// Note: Baseline uses HKEY_CURRENT_USER, because this works without elevated privileges

TEST(RegistryAccess, DoesKeyExist)
{
	SResult sr;

	auto oReg = RegistryAccess{ HKEY_CURRENT_USER };

	sr = oReg.CheckKeyExists(svzBaseKey_Test);
	EXPECT_EQ(sr, SResult::Success);

	{
		auto bKeyExists = oReg.DoesKeyExist(svzBaseKey_Test);
		EXPECT_EQ(bKeyExists, true);
	}

	sr = oReg.CheckKeyExists(svzBaseKey_Invalid);
	EXPECT_EQ(sr, SResult::Success_WithNuance);

	{
		auto bKeyExists = oReg.DoesKeyExist(svzBaseKey_Invalid);
		EXPECT_EQ(bKeyExists, false);
	}
}

TEST(RegistryAccess, CreateKeyAndDeleteKey)
{
	SResult sr;

	auto sTestKey = fmt::format(_T("{}\\{}"), svzBaseKey_Test, _T("testCreateDelete"));

	auto oReg = RegistryAccess{ HKEY_CURRENT_USER };

	sr = oReg.EnsureKeyExists(sTestKey);
	EXPECT_EQ(sr, SResult::Success);

	// Ensure we can call it multiple times
	sr = oReg.EnsureKeyExists(sTestKey);
	EXPECT_EQ(sr, SResult::Success);

	const auto oDeleteKeyOptions = RegistryAccess::Options_DeleteKey{}
	.withSafeDeletePath(svzBaseKey_Test);
	sr = oReg.DeleteKey(sTestKey, oDeleteKeyOptions);
	EXPECT_EQ(sr, SResult::Success);
}
