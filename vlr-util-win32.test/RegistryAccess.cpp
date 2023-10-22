#include "pch.h"

#include <vector>
#include <fmt/format.h>

#include "vlr-util/cpp_namespace.h"
#include "vlr-util/StringCompare.h"
#include "vlr-util/util.data_adaptor.MultiSZ.h"
#include "vlr-util/util.convert.StringConversion.h"

#include "vlr-util-win32/RegistryAccess.h"

using namespace vlr;
using namespace vlr::win32;

using QWORD = CRegistryAccess::QWORD;

static constexpr auto svzBaseKey_Test = tzstring_view{ _T("SOFTWARE\\vlr-test") };
static constexpr auto svzBaseKey_Invalid = tzstring_view{ _T("SOFTWARE\\vlr-test-invalid") };

// Note: Pre-defined values matching test data which is pre-loaded

static constexpr auto svzTestKey = svzBaseKey_Test;
static constexpr auto svzTestValueName_SZ = vlr::tzstring_view{ _T("testString") };
static constexpr auto svzTestValue_SZ = vlr::tzstring_view{ _T("value") };
static constexpr auto svzTestValueName_DWORD = vlr::tzstring_view{ _T("testDWORD") };
static constexpr auto nTestValue_DWORD = DWORD{ 42 };
static constexpr auto svzTestValueName_QWORD = vlr::tzstring_view{ _T("testQWORD") };
static constexpr auto nTestValue_QWORD = QWORD{ 42 };
static constexpr auto svzTestValueName_MultiSz = vlr::tzstring_view{ _T("testMultiSz") };
static const auto arrTestValue_MultiSz = std::vector<vlr::tstring>{ _T("value1"), _T("value2") };
static constexpr auto svzTestValueName_BINARY = vlr::tzstring_view{ _T("testBinary") };
static const auto arrTestValue_Binary = std::vector<BYTE>{ 0x12, 0x34, 0x56, 0x78 };

// Value which should not exist in test data
static constexpr auto svzTestValueName_Invalid = vlr::tzstring_view{ _T("testInvalid") };

void ValidateReadDataMatch_SZ(DWORD dwType, cpp::span<const BYTE> spanData)
{
	EXPECT_EQ(dwType, REG_SZ);
	// Note: This is GE because in some cases, the Reg[...] call may return the string without
	// the NULL-terminator, and we cannot assume this. Careful with Base functions.
	EXPECT_GE(spanData.size(), svzTestValue_SZ.size() * sizeof(TCHAR));
	EXPECT_LE(spanData.size(), (svzTestValue_SZ.size() + 1) * sizeof(TCHAR));

	auto svzValue = vlr::tstring_view{ reinterpret_cast<const TCHAR*>(spanData.data()), 5 };
	EXPECT_EQ(StringCompare::CS().AreEqual(svzValue, svzTestValue_SZ), true);
}

void ValidateReadDataMatch_DWORD(DWORD dwType, cpp::span<const BYTE> spanData)
{
	EXPECT_EQ(dwType, REG_DWORD);
	EXPECT_EQ(spanData.size(), sizeof(DWORD));

	auto tValue = *reinterpret_cast<const DWORD*>(spanData.data());
	EXPECT_EQ(tValue, nTestValue_DWORD);
}

void ValidateReadDataMatch_QWORD(DWORD dwType, cpp::span<const BYTE> spanData)
{
	EXPECT_EQ(dwType, REG_QWORD);
	EXPECT_EQ(spanData.size(), sizeof(QWORD));

	auto tValue = *reinterpret_cast<const QWORD*>(spanData.data());
	EXPECT_EQ(tValue, nTestValue_QWORD);
}

void ValidateReadDataMatch_MultiSZ(DWORD dwType, cpp::span<const BYTE> spanData)
{
	EXPECT_EQ(dwType, REG_MULTI_SZ);

	auto arrValues = util::data_adaptor::MultiSZToStructuredData(reinterpret_cast<const TCHAR*>(spanData.data()));
	ASSERT_EQ(arrValues.size(), arrTestValue_MultiSz.size());
	for (size_t i = 0; i < arrValues.size(); ++i)
	{
		EXPECT_EQ(StringCompare::CS().AreEqual(arrValues[i], arrTestValue_MultiSz[i]), true);
	}
}

void ValidateReadDataMatch_Binary(DWORD dwType, cpp::span<const BYTE> spanData)
{
	EXPECT_EQ(dwType, REG_BINARY);

	ASSERT_EQ(spanData.size(), arrTestValue_Binary.size());
	for (size_t i = 0; i < spanData.size(); ++i)
	{
		EXPECT_EQ(spanData[i], arrTestValue_Binary[i]);
	}
}

// Note: Baseline uses HKEY_CURRENT_USER, because this works without elevated privileges

TEST(RegistryAccess, DoesKeyExist)
{
	SResult sr;

	auto oReg = CRegistryAccess{ HKEY_CURRENT_USER };

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

	auto oReg = CRegistryAccess{ HKEY_CURRENT_USER };

	sr = oReg.EnsureKeyExists(sTestKey);
	EXPECT_EQ(sr, SResult::Success);

	// Ensure we can call it multiple times
	sr = oReg.EnsureKeyExists(sTestKey);
	EXPECT_EQ(sr, SResult::Success);

	// Very key exists using checking method
	sr = oReg.CheckKeyExists(sTestKey);
	EXPECT_EQ(sr, SResult::Success);

	const auto oDeleteKeyOptions = CRegistryAccess::Options_DeleteKeysOrValues{}
	.withSafeDeletePath(svzBaseKey_Test);
	sr = oReg.DeleteKey(sTestKey, oDeleteKeyOptions);
	EXPECT_EQ(sr, SResult::Success);

	// Should now be S_FALSE for check
	sr = oReg.CheckKeyExists(sTestKey);
	EXPECT_EQ(sr, SResult::Success_WithNuance);
}

TEST(RegistryAccess, ReadValueInfo)
{
	SResult sr;

	auto oReg = CRegistryAccess{ HKEY_CURRENT_USER };

	// Invalid value
	{
		DWORD dwType{};
		DWORD dwSize{};
		sr = oReg.ReadValueInfo(svzTestKey, svzTestValueName_Invalid, dwType, dwSize);
		EXPECT_EQ(sr.isSuccess(), false);
		EXPECT_EQ(sr.asHRESULT(), __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
	}

	// Read REG_SZ
	{
		DWORD dwType{};
		DWORD dwSize{};
		sr = oReg.ReadValueInfo(svzTestKey, svzTestValueName_SZ, dwType, dwSize);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(dwType, REG_SZ);
		EXPECT_GE(dwSize, svzTestValue_SZ.size() * sizeof(TCHAR));
		EXPECT_LE(dwSize, (svzTestValue_SZ.size() + 1) * sizeof(TCHAR));
	}

	// Read DWORD
	{
		DWORD dwType{};
		DWORD dwSize{};
		sr = oReg.ReadValueInfo(svzTestKey, svzTestValueName_DWORD, dwType, dwSize);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(dwType, REG_DWORD);
		EXPECT_EQ(dwSize, sizeof(DWORD));
	}

	// Read QWORD
	{
		DWORD dwType{};
		DWORD dwSize{};
		sr = oReg.ReadValueInfo(svzTestKey, svzTestValueName_QWORD, dwType, dwSize);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(dwType, REG_QWORD);
		EXPECT_EQ(dwSize, sizeof(QWORD));
	}

	// Read MULTI_SZ
	{
		DWORD dwType{};
		DWORD dwSize{};
		sr = oReg.ReadValueInfo(svzTestKey, svzTestValueName_MultiSz, dwType, dwSize);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(dwType, REG_MULTI_SZ);

		size_t nExpectedSize = 0;
		for (size_t i = 0; i < arrTestValue_MultiSz.size(); ++i)
		{
			nExpectedSize += arrTestValue_MultiSz[i].size() * sizeof(TCHAR);
			nExpectedSize += sizeof(TCHAR);
		}
		// Ending double NULL:
		nExpectedSize += sizeof(TCHAR);

		EXPECT_EQ(dwSize, nExpectedSize);
	}

	// Read Binary
	{
		DWORD dwType{};
		DWORD dwSize{};
		sr = oReg.ReadValueInfo(svzTestKey, svzTestValueName_BINARY, dwType, dwSize);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(dwType, REG_BINARY);
		ASSERT_EQ(dwSize, arrTestValue_Binary.size());
	}
}

TEST(RegistryAccess, ReadValueBase)
{
	SResult sr;

	auto oReg = CRegistryAccess{ HKEY_CURRENT_USER };

	// Invalid value
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		sr = oReg.ReadValueBase(svzTestKey, svzTestValueName_Invalid, dwType, arrData);
		EXPECT_EQ(sr.isSuccess(), false);
		EXPECT_EQ(sr.asHRESULT(), __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
	}

	// Normal read with empty buffer initially
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		sr = oReg.ReadValueBase(svzTestKey, svzTestValueName_SZ, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
		ValidateReadDataMatch_SZ(dwType, arrData);
	}

	// Passing a pre-allocated buffer
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		arrData.resize(1024);
		sr = oReg.ReadValueBase(svzTestKey, svzTestValueName_SZ, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
		ValidateReadDataMatch_SZ(dwType, arrData);
	}

	// Read DWORD
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		sr = oReg.ReadValueBase(svzTestKey, svzTestValueName_DWORD, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
		ValidateReadDataMatch_DWORD(dwType, arrData);
	}

	// Read QWORD
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		sr = oReg.ReadValueBase(svzTestKey, svzTestValueName_QWORD, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
		ValidateReadDataMatch_QWORD(dwType, arrData);
	}

	// Read MULTI_SZ
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		sr = oReg.ReadValueBase(svzTestKey, svzTestValueName_MultiSz, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
		ValidateReadDataMatch_MultiSZ(dwType, arrData);
	}

	// Read Binary
	{
		DWORD dwType{};
		std::vector<BYTE> arrData;
		sr = oReg.ReadValueBase(svzTestKey, svzTestValueName_BINARY, dwType, arrData);
		EXPECT_EQ(sr, SResult::Success);
		ValidateReadDataMatch_Binary(dwType, arrData);
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

	auto oReg = CRegistryAccess{ HKEY_CURRENT_USER };

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

TEST(RegistryAccess, ReadValue_String)
{
	SResult sr;

	auto oReg = CRegistryAccess{ HKEY_CURRENT_USER };

	{
		std::string sValue;
		sr = oReg.ReadValue_String(svzTestKey, svzTestValueName_SZ, sValue);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(StringCompare::CS().AreEqual(sValue, svzTestValue_SZ), true);
	}
	{
		std::wstring sValue;
		sr = oReg.ReadValue_String(svzTestKey, svzTestValueName_SZ, sValue);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(StringCompare::CS().AreEqual(sValue, svzTestValue_SZ), true);
	}
}

TEST(RegistryAccess, WriteValue_String)
{
	SResult sr;

	static constexpr auto sTestValueName_SZ = vlr::tzstring_view{ _T("testString_Copy") };
	static constexpr auto svzTestValue_SZ = vlr::tzstring_view{ _T("value") };

	auto oReg = CRegistryAccess{ HKEY_CURRENT_USER };

	{
		std::string sValue = util::Convert::ToStdStringA(svzTestValue_SZ);
		sr = oReg.WriteValue_String(svzTestKey, sTestValueName_SZ, sValue);
		EXPECT_EQ(sr, SResult::Success);
	}
	{
		std::wstring sValue = util::Convert::ToStdStringW(svzTestValue_SZ);
		sr = oReg.WriteValue_String(svzTestKey, sTestValueName_SZ, sValue);
		EXPECT_EQ(sr, SResult::Success);
	}
}

TEST(RegistryAccess, ReadValue_Template)
{
	SResult sr;

	auto oReg = CRegistryAccess{ HKEY_CURRENT_USER };

	{
		std::string sValue;
		sr = oReg.ReadValue(svzTestKey, svzTestValueName_SZ, sValue);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(StringCompare::CS().AreEqual(sValue, svzTestValue_SZ), true);
	}
	{
		std::wstring sValue;
		sr = oReg.ReadValue(svzTestKey, svzTestValueName_SZ, sValue);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(StringCompare::CS().AreEqual(sValue, svzTestValue_SZ), true);
	}
	{
		DWORD dwValue;
		sr = oReg.ReadValue(svzTestKey, svzTestValueName_DWORD, dwValue);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(dwValue, nTestValue_DWORD);
	}
	{
		QWORD qwValue;
		sr = oReg.ReadValue(svzTestKey, svzTestValueName_QWORD, qwValue);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(qwValue, nTestValue_QWORD);
	}
	{
		std::vector<vlr::tstring> arrValueCollection;
		sr = oReg.ReadValue(svzTestKey, svzTestValueName_MultiSz, arrValueCollection);
		EXPECT_EQ(sr, SResult::Success);
		ASSERT_EQ(arrValueCollection.size(), arrTestValue_MultiSz.size());
		for (size_t i = 0; i < arrValueCollection.size(); ++i)
		{
			EXPECT_EQ(StringCompare::CS().AreEqual(arrValueCollection[i], arrTestValue_MultiSz[i]), true);
		}
	}
	{
		std::vector<BYTE> arrData;
		sr = oReg.ReadValue(svzTestKey, svzTestValueName_BINARY, arrData);
		EXPECT_EQ(sr, SResult::Success);
		ASSERT_EQ(arrData.size(), arrTestValue_Binary.size());
		for (size_t i = 0; i < arrData.size(); ++i)
		{
			EXPECT_EQ(arrData[i], arrTestValue_Binary[i]);
		}
	}
}

TEST(RegistryAccess, WriteValue_Template)
{
	SResult sr;

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

	auto oReg = CRegistryAccess{ HKEY_CURRENT_USER };

	{
		std::string sValue = util::Convert::ToStdStringA(svzTestValue_SZ);
		sr = oReg.WriteValue(svzTestKey, sTestValueName_SZ, sValue);
		EXPECT_EQ(sr, SResult::Success);
	}
	{
		std::wstring sValue = util::Convert::ToStdStringW(svzTestValue_SZ);
		sr = oReg.WriteValue(svzTestKey, sTestValueName_SZ, sValue);
		EXPECT_EQ(sr, SResult::Success);
	}
	{
		sr = oReg.WriteValue(svzTestKey, sTestValueName_DWORD, nTestValue_DWORD);
		EXPECT_EQ(sr, SResult::Success);
	}
	{
		sr = oReg.WriteValue(svzTestKey, sTestValueName_QWORD, nTestValue_QWORD);
		EXPECT_EQ(sr, SResult::Success);
	}
	{
		sr = oReg.WriteValue(svzTestKey, sTestValueName_MultiSz, arrTestValue_MultiSz);
		EXPECT_EQ(sr, SResult::Success);
	}
	{
		sr = oReg.WriteValue(svzTestKey, sTestValueName_BINARY, arrTestValue_Binary);
		EXPECT_EQ(sr, SResult::Success);
	}
}

TEST(RegistryAccess, ReadValue_WithDefault)
{
	SResult sr;

	static constexpr auto svzTestKey = svzBaseKey_Test;
	static constexpr auto sTestValueName_Invalid = vlr::tzstring_view{ _T("testInvalid") };
	static constexpr auto svzTestValue_SZ = vlr::tzstring_view{ _T("value") };
	static constexpr auto nTestValue_DWORD = DWORD{ 42 };
	static constexpr auto nTestValue_QWORD = QWORD{ 42 };
	static const auto arrTestValue_MultiSz = std::vector<vlr::tstring>{ _T("value1"), _T("value2") };
	static const auto arrTestValue_Binary = std::vector<BYTE>{ 0x12, 0x34, 0x56, 0x78 };

	auto oReg = CRegistryAccess{ HKEY_CURRENT_USER };

	{
		std::string sValue;
		sr = oReg.ReadValue(svzTestKey, sTestValueName_Invalid, sValue, util::Convert::ToStdStringA(svzTestValue_SZ));
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(StringCompare::CS().AreEqual(sValue, svzTestValue_SZ), true);
	}
	{
		std::wstring sValue;
		sr = oReg.ReadValue(svzTestKey, sTestValueName_Invalid, sValue, util::Convert::ToStdStringW(svzTestValue_SZ));
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(StringCompare::CS().AreEqual(sValue, svzTestValue_SZ), true);
	}
	{
		DWORD dwValue;
		sr = oReg.ReadValue(svzTestKey, sTestValueName_Invalid, dwValue, nTestValue_DWORD);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(dwValue, nTestValue_DWORD);
	}
	{
		QWORD qwValue;
		sr = oReg.ReadValue(svzTestKey, sTestValueName_Invalid, qwValue, nTestValue_QWORD);
		EXPECT_EQ(sr, SResult::Success);
		EXPECT_EQ(qwValue, nTestValue_QWORD);
	}
	{
		std::vector<vlr::tstring> arrValueCollection;
		sr = oReg.ReadValue(svzTestKey, sTestValueName_Invalid, arrValueCollection, arrTestValue_MultiSz);
		EXPECT_EQ(sr, SResult::Success);
		ASSERT_EQ(arrValueCollection.size(), arrTestValue_MultiSz.size());
		for (size_t i = 0; i < arrValueCollection.size(); ++i)
		{
			EXPECT_EQ(StringCompare::CS().AreEqual(arrValueCollection[i], arrTestValue_MultiSz[i]), true);
		}
	}
	{
		std::vector<BYTE> arrData;
		sr = oReg.ReadValue(svzTestKey, sTestValueName_Invalid, arrData, arrTestValue_Binary);
		EXPECT_EQ(sr, SResult::Success);
		ASSERT_EQ(arrData.size(), arrTestValue_Binary.size());
		for (size_t i = 0; i < arrData.size(); ++i)
		{
			EXPECT_EQ(arrData[i], arrTestValue_Binary[i]);
		}
	}
}

TEST(RegistryAccess, DeleteValue)
{
	SResult sr;

	static constexpr auto svzTestKey = svzBaseKey_Test;
	static constexpr auto sTestValueName_NewValue = vlr::tzstring_view{ _T("testNewValue") };
	static constexpr auto svzTestValue_SZ = vlr::tzstring_view{ _T("value") };

	auto oReg = CRegistryAccess{ HKEY_CURRENT_USER };

	vlr::tstring sValue;
	sr = oReg.ReadValue(svzTestKey, sTestValueName_NewValue, sValue);
	EXPECT_EQ(sr, __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

	sr = oReg.WriteValue(svzTestKey, sTestValueName_NewValue, svzTestValue_SZ);
	EXPECT_EQ(sr, S_OK);

	sr = oReg.ReadValue(svzTestKey, sTestValueName_NewValue, sValue);
	EXPECT_EQ(sr, S_OK);
	EXPECT_EQ(StringCompare::CS().AreEqual(sValue, svzTestValue_SZ), true);

	sr = oReg.DeleteValue(svzTestKey, sTestValueName_NewValue);
	EXPECT_EQ(sr, S_OK);

	sr = oReg.ReadValue(svzTestKey, sTestValueName_NewValue, sValue);
	EXPECT_EQ(sr, __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
}

TEST(RegistryAccess, EnumAllValues)
{
	SResult sr;

	// Note: We should read all the test values
	bool m_bReadTestValue_SZ = false;
	bool m_bReadTestValue_DWORD = false;
	bool m_bReadTestValue_QWORD = false;
	bool m_bReadTestValue_MultiSZ = false;
	bool m_bReadTestValue_Binary = false;

	auto oReg = CRegistryAccess{ HKEY_CURRENT_USER };

	auto fOnEnumValueData = [&](const CRegistryAccess::EnumValueData& oEnumValueData)
	{
		if (StringCompare::CS().AreEqual(oEnumValueData.m_svName, svzTestValueName_SZ))
		{
			ValidateReadDataMatch_SZ(oEnumValueData.m_dwType, oEnumValueData.m_spanData);
			m_bReadTestValue_SZ = true;
			return SResult::Success;
		}
		if (StringCompare::CS().AreEqual(oEnumValueData.m_svName, svzTestValueName_DWORD))
		{
			ValidateReadDataMatch_DWORD(oEnumValueData.m_dwType, oEnumValueData.m_spanData);
			m_bReadTestValue_DWORD = true;
			return SResult::Success;
		}
		if (StringCompare::CS().AreEqual(oEnumValueData.m_svName, svzTestValueName_QWORD))
		{
			ValidateReadDataMatch_QWORD(oEnumValueData.m_dwType, oEnumValueData.m_spanData);
			m_bReadTestValue_QWORD = true;
			return SResult::Success;
		}
		if (StringCompare::CS().AreEqual(oEnumValueData.m_svName, svzTestValueName_MultiSz))
		{
			ValidateReadDataMatch_MultiSZ(oEnumValueData.m_dwType, oEnumValueData.m_spanData);
			m_bReadTestValue_MultiSZ = true;
			return SResult::Success;
		}
		if (StringCompare::CS().AreEqual(oEnumValueData.m_svName, svzTestValueName_BINARY))
		{
			ValidateReadDataMatch_Binary(oEnumValueData.m_dwType, oEnumValueData.m_spanData);
			m_bReadTestValue_Binary = true;
			return SResult::Success;
		}

		return SResult::Success_NoWorkDone;
	};

	sr = oReg.EnumAllValues(svzTestKey, fOnEnumValueData);
	EXPECT_EQ(sr, S_OK);

	EXPECT_EQ(m_bReadTestValue_SZ, true);
	EXPECT_EQ(m_bReadTestValue_DWORD, true);
	EXPECT_EQ(m_bReadTestValue_QWORD, true);
	EXPECT_EQ(m_bReadTestValue_MultiSZ, true);
	EXPECT_EQ(m_bReadTestValue_Binary, true);
}

TEST(RegistryAccess, RealAllValuesIntoMap)
{
	SResult sr;

	// Note: We should read all the test values
	bool m_bReadTestValue_SZ = false;
	bool m_bReadTestValue_DWORD = false;
	bool m_bReadTestValue_QWORD = false;
	bool m_bReadTestValue_MultiSZ = false;
	bool m_bReadTestValue_Binary = false;

	auto oReg = CRegistryAccess{ HKEY_CURRENT_USER };

	std::unordered_map<vlr::tstring, CRegistryAccess::ValueMapEntry> mapNameToValue;
	sr = oReg.RealAllValuesIntoMap(svzTestKey, mapNameToValue);
	EXPECT_EQ(sr, S_OK);

	for (const auto& mapEntry : mapNameToValue)
	{
		const auto& sValueName = mapEntry.first;
		const auto& oValueMapEntry = mapEntry.second;

		if (StringCompare::CS().AreEqual(sValueName, svzTestValueName_SZ))
		{
			ASSERT_NE(oValueMapEntry.m_spValue_SZ, nullptr);
			EXPECT_EQ(StringCompare::CS().AreEqual(*oValueMapEntry.m_spValue_SZ, svzTestValue_SZ), true);
			m_bReadTestValue_SZ = true;
			continue;
		}
		if (StringCompare::CS().AreEqual(sValueName, svzTestValueName_DWORD))
		{
			ASSERT_NE(oValueMapEntry.m_spValue_DWORD, nullptr);
			EXPECT_EQ(*oValueMapEntry.m_spValue_DWORD, nTestValue_DWORD);
			m_bReadTestValue_DWORD = true;
			continue;
		}
		if (StringCompare::CS().AreEqual(sValueName, svzTestValueName_QWORD))
		{
			ASSERT_NE(oValueMapEntry.m_spValue_QWORD, nullptr);
			EXPECT_EQ(*oValueMapEntry.m_spValue_QWORD, nTestValue_QWORD);
			m_bReadTestValue_QWORD = true;
			continue;
		}
		if (StringCompare::CS().AreEqual(sValueName, svzTestValueName_MultiSz))
		{
			ASSERT_NE(oValueMapEntry.m_spValue_MultiSZ, nullptr);
			//ValidateReadDataMatch_MultiSZ(oEnumValueData.m_dwType, oEnumValueData.m_spanData);
			m_bReadTestValue_MultiSZ = true;
			continue;
		}
		if (StringCompare::CS().AreEqual(sValueName, svzTestValueName_BINARY))
		{
			ASSERT_NE(oValueMapEntry.m_spValue_Binary, nullptr);
			//ValidateReadDataMatch_Binary(oEnumValueData.m_dwType, oEnumValueData.m_spanData);
			m_bReadTestValue_Binary = true;
			continue;
		}
	};

	EXPECT_EQ(m_bReadTestValue_SZ, true);
	EXPECT_EQ(m_bReadTestValue_DWORD, true);
	EXPECT_EQ(m_bReadTestValue_QWORD, true);
	EXPECT_EQ(m_bReadTestValue_MultiSZ, true);
	EXPECT_EQ(m_bReadTestValue_Binary, true);
}
