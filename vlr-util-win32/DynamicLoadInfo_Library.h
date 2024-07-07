#pragma once

#include <vlr-util/util.Result.h>
#include <vlr-util/util.std_aliases.h>

namespace vlr {

namespace win32 {

class CDynamicLoadInfo_Library
{
protected:
	vlr::tstring m_sLibraryName;
	DWORD m_dwFlags_LoadLibraryEx = 0;

public:
	decltype(auto) withLibraryName(vlr::tzstring_view svzLibraryName)
	{
		m_sLibraryName = svzLibraryName.toStdString();
		return *this;
	}
	decltype(auto) withFlags_LoadLibraryEx(DWORD dwFlags)
	{
		m_dwFlags_LoadLibraryEx = dwFlags;
		return *this;
	}
	decltype(auto) getLibraryName() const
	{
		return m_sLibraryName;
	}
	decltype(auto) getFLags_LoadLibraryEx() const
	{
		return m_dwFlags_LoadLibraryEx;
	}

	decltype(auto) GetLibraryName_FilenameOnly() const
	{
		// TODO: Implement this...
		return m_sLibraryName;
	}

public:
	CDynamicLoadInfo_Library() = default;
	CDynamicLoadInfo_Library(vlr::tzstring_view svzLibraryName)
		: m_sLibraryName{ svzLibraryName.toStdString() }
	{}
};

} // namespace win32

} // namespace vlr
