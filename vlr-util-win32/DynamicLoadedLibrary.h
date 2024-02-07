#pragma once

#include <vlr-util/util.Result.h>
#include <vlr-util/util.std_aliases.h>

namespace vlr {

namespace win32 {

class CDynamicLoadedLibrary
{
public:
	vlr::tstring m_sLoadName;
	SResult m_srLoadResult;
	HMODULE m_hLibrary{};

	// Note: Normally, this should be true; library references are reference counted.
	bool m_bFreeLibraryOnDestroy = true;

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
