#pragma once

#include <vlr-util/util.Result.h>
#include <vlr-util/util.std_aliases.h>

namespace vlr {

namespace win32 {

class CDynamicLoadInfo_Library
{
protected:
	vlr::tstring m_sLibraryName;
	std::optional<DWORD> m_optExplicitFlags_LoadLibraryEx{};

	vlr::tstring m_sLibraryName_Normalized;
	vlr::tstring m_sLibraryFilename;
	bool m_bLibraryNameIsAbsolutePathQualified{};
	bool m_bLibraryPathExists{};

	// This flag indicates that the caller expects the library to be loaded, and it should be an error
	// if the library is not preloaded.
	bool m_bExpectLibraryAlreadyLoaded = false;

	// This flag tells the leader to use the module path instead of the calling binary path in the search list.
	// We do this by default, because it makes more sense in most cases.
	// Note: This flag will be ignored if explicit flags are specified.
	bool m_bOnAbsolutePathQualified_SetFlag_AlternativeSearchPath = true;

public:
	decltype(auto) withLibraryName(vlr::tzstring_view svzLibraryName)
	{
		SetLibraryName(svzLibraryName);
		return *this;
	}
	decltype(auto) withExplicitFlags_LoadLibraryEx(DWORD dwFlags)
	{
		m_optExplicitFlags_LoadLibraryEx = dwFlags;
		return *this;
	}
	decltype(auto) withExplicitFlags_LoadLibraryEx_Clear()
	{
		m_optExplicitFlags_LoadLibraryEx = {};
		return *this;
	}
	decltype(auto) withExpectLibraryAlreadyLoaded(bool bValue = true)
	{
		m_bExpectLibraryAlreadyLoaded = bValue;
		return true;
	}
	decltype(auto) withOnAbsolutePathQualified_SetFlag_AlternativeSearchPath(bool bValue)
	{
		m_bOnAbsolutePathQualified_SetFlag_AlternativeSearchPath = bValue;
		return *this;
	}

	inline const auto& GetLibraryName() const
	{
		return m_sLibraryName;
	}
	inline const auto& GetExplicitFlags_LoadLibraryEx() const
	{
		return m_optExplicitFlags_LoadLibraryEx;
	}
	inline const auto& GetLibraryName_Normalized() const
	{
		return m_sLibraryName_Normalized;
	}
	inline const auto& GetLibraryName_FilenameOnly() const
	{
		return m_sLibraryFilename;
	}
	inline const auto& GetExpectLibraryAlreadyLoaded() const
	{
		return m_bExpectLibraryAlreadyLoaded;
	}

	SResult SetLibraryName(vlr::tzstring_view svzLibraryName);

	// If explicit flags are set, this is returned.
	// Else, the function will return the most applicable default flags, based on settings.
	DWORD GetEffectiveFlags_LoadLibraryEx() const;

public:
	CDynamicLoadInfo_Library() = default;
	CDynamicLoadInfo_Library(vlr::tzstring_view svzLibraryName)
	{
		SetLibraryName(svzLibraryName);
	}
};

} // namespace win32

} // namespace vlr
