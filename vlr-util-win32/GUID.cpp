#include "pch.h"
#include "GUID.h"

#include <vlr-util/util.range_checked_cast.h>
#include <vlr-util/util.convert.StringConversion.h>

// Note: Most methods in this class are simple wrappers for functions in comapibase.h

namespace vlr {

namespace win32 {

SResult CGUID::CreateGUID()
{
	HRESULT hr;

	hr = CoCreateGuid(this);
	VLR_ASSERT_HR_SUCCEEDED_OR_RETURN_HRESULT(hr);

	return S_OK;
}

bool CGUID::operator==(const CGUID& oOther) const
{
	auto bResult = IsEqualGUID(*this, oOther);
	return (bResult == TRUE);
}

vlr::tstring CGUID::ToString() const
{
	static const auto _tFailureValue = vlr::tstring{};

	// Formatted GUIDs are 39 chars:
	// 32 characters from GUID
	// 4 hyphons
	// 2 braces
	// NULL terminator
	std::vector<wchar_t> vecFormatBuffer;
	vecFormatBuffer.resize(40);

	auto nFormatSize = StringFromGUID2(*this, vecFormatBuffer.data(), util::range_checked_cast<int>(vecFormatBuffer.size()));
	VLR_ASSERT_COMPARE_OR_RETURN_FAILURE_VALUE(nFormatSize, == , 39);

	auto svzFormatData = vlr::wzstring_view{ vecFormatBuffer.data(), util::range_checked_cast<size_t>(nFormatSize - 1), vlr::wzstring_view::StringIsNullTerminated{} };
	return util::Convert::ToStdString(svzFormatData);
}

SResult CGUID::ParseString(vlr::wzstring_view svzFormatValue)
{
	HRESULT hr;

	hr = CLSIDFromString(svzFormatValue, this);
	VLR_ON_HR_NON_S_OK__RETURN_HRESULT(hr);

	return S_OK;
}

SResult CGUID::ParseString(vlr::zstring_view svzFormatValue)
{
	auto swFormatValue = util::Convert::ToStdStringW(svzFormatValue);
	return ParseString(swFormatValue);
}

} // namespace win32

} // namespace vlr
