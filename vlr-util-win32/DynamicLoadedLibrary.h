#pragma once

#include <vlr-util/util.Result.h>
#include <vlr-util/util.std_aliases.h>

#include "DynamicLoadInfo_Library.h"

namespace vlr {

namespace win32 {

namespace detail {

class CDynamicLoadedLibraryData
{
public:
	CDynamicLoadInfo_Library m_oLoadInfo;
	SResult m_srLoadResult;
	HMODULE m_hLibrary{};

	// Note: Normally, this should be true; library references are reference counted.
	bool m_bFreeLibraryOnDestroy = true;

public:
	inline bool HasLoadBeenAttempted() const
	{
		return (m_srLoadResult.isSet());
	}
	inline bool LibraryLoadedSuccessfully() const
	{
		return true
			&& m_srLoadResult.isSuccess()
			&& (m_hLibrary != NULL)
			;
	}

};

} // namespace detail

class CDynamicLoadedLibrary
	: public detail::CDynamicLoadedLibraryData
{
protected:
	inline SResult OnInvalidate_FreeLibrary()
	{
		if (m_hLibrary == nullptr)
		{
			return SResult::Success_NoWorkDone;
		}
		if (!m_bFreeLibraryOnDestroy)
		{
			return SResult::Success_NoWorkDone;
		}

		FreeLibrary(m_hLibrary);

		return SResult::Success;
	}

public:
	~CDynamicLoadedLibrary()
	{
		OnInvalidate_FreeLibrary();
	}

	// Note: Need "rule of 5" here, to ensure we free any loaded library before copying in new data

	CDynamicLoadedLibrary() = default;
	CDynamicLoadedLibrary(const CDynamicLoadedLibrary& oOther)
	{
		OnInvalidate_FreeLibrary();
		static_cast<CDynamicLoadedLibraryData&>(*this) = oOther;
	}
	CDynamicLoadedLibrary(CDynamicLoadedLibrary&& oOther)
	{
		OnInvalidate_FreeLibrary();
		static_cast<CDynamicLoadedLibraryData&>(*this) = std::move(oOther);
	}
	decltype(auto) operator=(const CDynamicLoadedLibrary& oOther)
	{
		OnInvalidate_FreeLibrary();
		static_cast<CDynamicLoadedLibraryData&>(*this) = oOther;
	}
	decltype(auto) operator=(CDynamicLoadedLibrary&& oOther)
	{
		OnInvalidate_FreeLibrary();
		static_cast<CDynamicLoadedLibraryData&>(*this) = std::move(oOther);
	}
};

} // namespace win32

} // namespace vlr
