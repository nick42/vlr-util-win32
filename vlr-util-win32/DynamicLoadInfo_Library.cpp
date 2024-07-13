#include "pch.h"
#include "DynamicLoadInfo_Library.h"

#include <filesystem>

#include <vlr-util/platform.FileSystemOps.h>

namespace vlr {

namespace win32 {

SResult CDynamicLoadInfo_Library::SetLibraryName(vlr::tzstring_view svzLibraryName)
{
	m_sLibraryName = svzLibraryName.toStdString();
	auto oLibraryPath = std::filesystem::path{ m_sLibraryName };

	m_sLibraryName_Normalized = oLibraryPath.native();
	m_sLibraryFilename = oLibraryPath.filename();
	m_bLibraryNameIsAbsolutePathQualified = oLibraryPath.is_absolute();
	if (m_bLibraryNameIsAbsolutePathQualified)
	{
		auto sr = vlr::platform::CFileSystmOps{}.CheckFileExists(m_sLibraryName_Normalized);
		m_bLibraryPathExists = (sr == SResult::Success);
	}
	else
	{
		m_bLibraryPathExists = {};
	}

	return SResult::Success;
}

DWORD CDynamicLoadInfo_Library::GetEffectiveFlags_LoadLibraryEx() const
{
	// Ref: https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order

	if (m_optExplicitFlags_LoadLibraryEx.has_value())
	{
		return m_optExplicitFlags_LoadLibraryEx.value();
	}

	DWORD dwFlags = 0;

	// Note: If you have an explicit path, you can specify that LoadLibraryEx substitute the module's path for the 
	// binary's path in the search list. We will do this by default.
	if (m_bLibraryNameIsAbsolutePathQualified && m_bOnAbsolutePathQualified_SetFlag_AlternativeSearchPath)
	{
		dwFlags |= LOAD_WITH_ALTERED_SEARCH_PATH;
	}

	return dwFlags;
}

} // namespace win32

} // namespace vlr
