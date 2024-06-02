#pragma once

#include "vlr-util/config.h"
#include "vlr-util/util.Result.h"

namespace vlr {

namespace win32 {

class CGUID
	: public GUID
{
public:
	SResult CreateGUID();

	bool operator==(const CGUID& oOther) const;

	vlr::tstring ToString() const;

	SResult ParseString(vlr::wzstring_view svzFormatValue);
	// Note: Win32 method uses LPOLESTR, so char version involves conversion
	SResult ParseString(vlr::zstring_view svzFormatValue);

public:
	inline static auto NewGUID()
	{
		CGUID oGUID;
		oGUID.CreateGUID();
		return oGUID;
	}
	template <typename TString>
	inline static auto FromString(const TString& tString)
	{
		CGUID oGUID;
		oGUID.ParseString(tString);
		return oGUID;
	}

public:
	constexpr CGUID()
		: GUID{}
	{}
	constexpr CGUID(const GUID& oGUID)
		: GUID{ oGUID }
	{}
};

} // namespace win32

} // namespace vlr
