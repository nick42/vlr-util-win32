#include "pch.h"

#include <vlr-util-win32/GUID.h>

#include <vlr-util/util.convert.StringConversion.h>

using namespace vlr;
using namespace vlr::win32;

struct TestGUID
	: public testing::Test
{
	CGUID m_oGUID;
	const CGUID m_oGUID_Default;
};

TEST_F(TestGUID, Constructors)
{
	EXPECT_EQ(m_oGUID.Data1, 0);
	EXPECT_EQ(m_oGUID.Data2, 0);
	EXPECT_EQ(m_oGUID.Data3, 0);
	EXPECT_EQ(m_oGUID.Data4[0], 0);
	EXPECT_EQ(m_oGUID.Data4[1], 0);
	EXPECT_EQ(m_oGUID.Data4[2], 0);
	EXPECT_EQ(m_oGUID.Data4[3], 0);
	EXPECT_EQ(m_oGUID.Data4[4], 0);
	EXPECT_EQ(m_oGUID.Data4[5], 0);
	EXPECT_EQ(m_oGUID.Data4[6], 0);
	EXPECT_EQ(m_oGUID.Data4[7], 0);
}

TEST_F(TestGUID, FromExistingGUID_NULL)
{
	m_oGUID = CGUID{ GUID_NULL };

	auto nCompare = memcmp(&m_oGUID, &m_oGUID_Default, sizeof(m_oGUID));
	EXPECT_EQ(nCompare, 0);
}

TEST_F(TestGUID, NewGUID)
{
	m_oGUID.CreateGUID();

	auto nCompare = memcmp(&m_oGUID, &m_oGUID_Default, sizeof(m_oGUID));
	EXPECT_NE(nCompare, 0);
}

TEST_F(TestGUID, OperatorEqual)
{
	m_oGUID.NewGUID();
	CGUID oGUID_Other = CGUID::NewGUID();

	auto nCompare = memcmp(&m_oGUID, &oGUID_Other, sizeof(m_oGUID));
	EXPECT_NE(nCompare, 0);

	auto bEqual = (m_oGUID == oGUID_Other);
	EXPECT_NE(bEqual, true);
}

TEST_F(TestGUID, ToString_DefaultValue)
{
	auto sFormatValue = m_oGUID_Default.ToString();

	static constexpr vlr::tzstring_view svzExpectedValue = _T("{00000000-0000-0000-0000-000000000000}");

	EXPECT_STREQ(sFormatValue.c_str(), svzExpectedValue.asConstPtr());
}

TEST_F(TestGUID, ToString)
{
	m_oGUID.NewGUID();
	auto sFormatValue = m_oGUID.ToString();

	EXPECT_EQ(sFormatValue.length(), 38);
	EXPECT_EQ(sFormatValue[0], _T('{'));
	EXPECT_EQ(sFormatValue[37], _T('}'));
}

TEST_F(TestGUID, ParseString)
{
	m_oGUID.NewGUID();
	auto sFormatValue = m_oGUID.ToString();

	CGUID oGUID_Copy;
	auto hr = oGUID_Copy.ParseString(sFormatValue);
	EXPECT_EQ(hr, S_OK);

	EXPECT_TRUE(m_oGUID == oGUID_Copy);
}

TEST_F(TestGUID, ParseString_char)
{
	m_oGUID.NewGUID();
	auto sFormatValue = m_oGUID.ToString();
	auto saFormatValue = util::Convert::ToStdStringA(sFormatValue);

	CGUID oGUID_Copy;
	auto hr = oGUID_Copy.ParseString(saFormatValue);
	EXPECT_EQ(hr, S_OK);

	EXPECT_TRUE(m_oGUID == oGUID_Copy);
}

TEST_F(TestGUID, ParseString_Invalid)
{
	CGUID oGUID;
	auto hr = oGUID.ParseString(_T("blah"));
	EXPECT_NE(hr, S_OK);
}

TEST_F(TestGUID, FromString)
{
	m_oGUID.NewGUID();
	auto sFormatValue = m_oGUID.ToString();

	auto oGUID_Copy = CGUID::FromString(sFormatValue);

	EXPECT_TRUE(m_oGUID == oGUID_Copy);
}
