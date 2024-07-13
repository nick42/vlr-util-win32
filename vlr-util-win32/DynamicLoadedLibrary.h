#pragma once

#include <vlr-util/util.Result.h>
#include <vlr-util/util.std_aliases.h>

#include "DynamicLoadInfo_Library.h"

namespace vlr {

namespace win32 {

class CDynamicLoadedLibrary
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

protected:
	inline SResult OnDestructor_FreeLibrary()
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
		OnDestructor_FreeLibrary();
	}
};

} // namespace win32

} // namespace vlr
