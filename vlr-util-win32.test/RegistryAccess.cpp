#include "pch.h"

#include <vector>
#include <fmt/format.h>

#include "vlr-util/StringCompare.h"
#include "vlr-util/util.data_adaptor.MultiSZ.h"

#include "vlr-util-win32/RegistryAccess.h"

using namespace vlr;
using namespace vlr::win32;

static constexpr auto svzBaseKey_Test = tzstring_view{ _T("SOFTWARE\\vlr-test") };
static constexpr auto svzBaseKey_Invalid = tzstring_view{ _T("SOFTWARE\\vlr-test-invalid") };

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

	// Very key exists using checking method
	sr = oReg.CheckKeyExists(sTestKey);
	EXPECT_EQ(sr, SResult::Success);

	const auto oDeleteKeyOptions = RegistryAccess::Options_DeleteKey{}
	.withSafeDeletePath(svzBaseKey_Test);
	sr = oReg.DeleteKey(sTestKey, oDeleteKeyOptions);
	EXPECT_EQ(sr, SResult::Success);

	// Should now be S_FALSE for check
	sr = oReg.CheckKeyExists(sTestKey);
	EXPECT_EQ(sr, SResult::Success_WithNuance);
}

TEST(RegistryAccess, ReadValueBase)
{
	SResult sr;

	using QWORD = unsigned __int64;

	static constexpr auto svzTestKey = svzBaseKey_Test;
	static constexpr auto sTestValueName_Invalid = vlr::tzstring_view{ _T("testInvalid") };
	static constexpr auto sTestValueName_SZ = vlr::tzstring_view{ _T("testString") };
	static constexpr auto svzTestValue_SZ = vlr::tzstring_view{ _T("value") };
	static constexpr auto sTestValueName_DWORD = vlr::tzstring_view{ _T("testDWORD") };
	static constexpr auto nTestValue_DWORD = DWORD{ 42 };
	static constexpr auto sTestValueName_QWORD = vlr::tzstring_view{ _T("testQWORD") };
	static constexpr auto nTestValue_QWORD = QWORD{ 42 };
	static constexpr auto sTestValueName_MultiSz = vlr::tzstring_view{ _T("testMultiSz") };
	static const auto arrTestValue_MultiSz = std::vector<vlr::tstring>{ _T("value1"), _T("value2") };
	static constexpr auto sTestValueName_BINARY = vlr::tzstring_view{ _T("testBinary") };
	static const auto arrTestValue_Binary = std::vector<BYTE>{ 0x12, 0x34, 0x56, 0x78 };

	auto oReg = RegistryAccess{ HKEY_CURRENT_USER };

	// Invalid value
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		sr = oReg.ReadValueBase(svzTestKey, sTestValueName_Invalid, dwType, arrData);
		EXPECT_EQ(sr.isSuccess(), false);
		EXPECT_EQ(sr.asHRESULT(), __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
	}

	// Normal read with empty buffer initially
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		sr = oReg.ReadValueBase(svzTestKey, sTestValueName_SZ, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(dwType, REG_SZ);
		// Note: This is GE because in some cases, the Reg[...] call may return the string without
		// the NULL-terminator, and we cannot assume this. Careful with Base functions.
		EXPECT_GE(arrData.size(), svzTestValue_SZ.size() * sizeof(TCHAR));
		EXPECT_LE(arrData.size(), (svzTestValue_SZ.size() + 1) * sizeof(TCHAR));

		auto svzValue = vlr::tstring_view{reinterpret_cast<const TCHAR*>(arrData.data()), 5};
		EXPECT_EQ(StringCompare::CS().AreEqual(svzValue, svzTestValue_SZ), true);
	}

	// Passing a pre-allocated buffer
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		arrData.resize(1024);
		sr = oReg.ReadValueBase(svzTestKey, sTestValueName_SZ, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(dwType, REG_SZ);
		EXPECT_GE(arrData.size(), svzTestValue_SZ.size() * sizeof(TCHAR));
		EXPECT_LE(arrData.size(), (svzTestValue_SZ.size() + 1) * sizeof(TCHAR));

		auto svzValue = vlr::tstring_view{ reinterpret_cast<const TCHAR*>(arrData.data()), 5 };
		EXPECT_EQ(StringCompare::CS().AreEqual(svzValue, svzTestValue_SZ), true);
	}

	// Read DWORD
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		sr = oReg.ReadValueBase(svzTestKey, sTestValueName_DWORD, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(dwType, REG_DWORD);
		EXPECT_EQ(arrData.size(), sizeof(DWORD));

		auto tValue = *reinterpret_cast<const DWORD*>(arrData.data());
		EXPECT_EQ(tValue, nTestValue_DWORD);
	}

	// Read QWORD
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		sr = oReg.ReadValueBase(svzTestKey, sTestValueName_QWORD, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(dwType, REG_QWORD);
		EXPECT_EQ(arrData.size(), sizeof(QWORD));

		auto tValue = *reinterpret_cast<const QWORD*>(arrData.data());
		EXPECT_EQ(tValue, nTestValue_QWORD);
	}

	// Read MULTI_SZ
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		sr = oReg.ReadValueBase(svzTestKey, sTestValueName_MultiSz, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(dwType, REG_MULTI_SZ);

		auto arrValues = util::data_adaptor::MultiSZToStructuredData(reinterpret_cast<const TCHAR*>(arrData.data()));
		ASSERT_EQ(arrValues.size(), arrTestValue_MultiSz.size());
		for (size_t i=0; i < arrValues.size(); ++i)
		{
			EXPECT_EQ(StringCompare::CS().AreEqual(arrValues[i], arrTestValue_MultiSz[i]), true);
		}
	}

	// Read Binary
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		sr = oReg.ReadValueBase(svzTestKey, sTestValueName_BINARY, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(dwType, REG_BINARY);

		ASSERT_EQ(arrData.size(), arrTestValue_Binary.size());
		for (size_t i = 0; i < arrData.size(); ++i)
		{
			EXPECT_EQ(arrData[i], arrTestValue_Binary[i]);
		}
	}
}

TEST(RegistryAccess, WriteValueBase)
{
	SResult sr;

	using QWORD = unsigned __int64;

	static constexpr auto svzTestKey = svzBaseKey_Test;
	static constexpr auto sTestValueName_SZ = vlr::tzstring_view{ _T("testString_Copy") };
	static constexpr auto svzTestValue_SZ = vlr::tzstring_view{ _T("value") };
	static constexpr auto sTestValueName_DWORD = vlr::tzstring_view{ _T("testDWORD_Copy") };
	static constexpr auto nTestValue_DWORD = DWORD{ 42 };
	static constexpr auto sTestValueName_QWORD = vlr::tzstring_view{ _T("testQWORD_Copy") };
	static constexpr auto nTestValue_QWORD = QWORD{ 42 };
	static constexpr auto sTestValueName_MultiSz = vlr::tzstring_view{ _T("testMultiSz_Copy") };
	static const auto arrTestValue_MultiSz = std::vector<vlr::tstring>{ _T("value1"), _T("value2") };
	static constexpr auto sTestValueName_BINARY = vlr::tzstring_view{ _T("testBinary_Copy") };
	static const auto arrTestValue_Binary = std::vector<BYTE>{ 0x12, 0x34, 0x56, 0x78 };

	auto oReg = RegistryAccess{ HKEY_CURRENT_USER };

	// Write string
	{
		DWORD dwType = REG_SZ;
		std::vector<BYTE> arrData;
		size_t nByteCountData = (svzTestValue_SZ.size() + 1) * sizeof(TCHAR);
		arrData.resize(nByteCountData);
		// Note: This is safe, because the NULL-terminator is in the memory block
		memcpy_s(arrData.data(), arrData.size(), svzTestValue_SZ.data(), nByteCountData);

		sr = oReg.WriteValueBase(svzTestKey, sTestValueName_SZ, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
	}

	// Write DWORD
	{
		DWORD dwType = REG_DWORD;
		std::vector<BYTE> arrData;
		size_t nByteCountData = sizeof(DWORD);
		arrData.resize(nByteCountData);
		memcpy_s(arrData.data(), arrData.size(), &nTestValue_DWORD, nByteCountData);

		sr = oReg.WriteValueBase(svzTestKey, sTestValueName_DWORD, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
	}

	// Write QWORD
	{
		DWORD dwType = REG_QWORD;
		std::vector<BYTE> arrData;
		size_t nByteCountData = sizeof(QWORD);
		arrData.resize(nByteCountData);
		memcpy_s(arrData.data(), arrData.size(), &nTestValue_QWORD, nByteCountData);

		sr = oReg.WriteValueBase(svzTestKey, sTestValueName_QWORD, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
	}

	// Write MULTI_SZ
	{
		DWORD dwType = REG_MULTI_SZ;
		std::vector<BYTE> arrData;
		util::data_adaptor::HelperFor_MultiSZ<TCHAR>{}.ToMultiSz(arrTestValue_MultiSz, arrData);

		sr = oReg.WriteValueBase(svzTestKey, sTestValueName_MultiSz, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
	}

	// Write Binary
	{
		DWORD dwType = REG_BINARY;
		std::vector<BYTE> arrData = arrTestValue_Binary;

		sr = oReg.WriteValueBase(svzTestKey, sTestValueName_BINARY, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
	}
}
