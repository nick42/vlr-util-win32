#pragma once

#include "vlr-util/config.h"
#include "vlr-util/util.Result.h"
#include "vlr-util/AppOptions.h"

#include "RegistryAccess.h"

namespace vlr {

namespace win32 {

// Note: By default, the class will interact with the shared instance of CAppOptions.

class CAppOptionSource_Registry
{
protected:
	CAppOptions* m_pAppOptions_Override = nullptr;

	inline auto& GetAppOptions()
	{
		if (m_pAppOptions_Override)
		{
			return *m_pAppOptions_Override;
		}
		else
		{
			return CAppOptions::GetSharedInstance();
		}
	}

public:
	decltype(auto) withAppOptionsOverride(CAppOptions* pAppOptions)
	{
		m_pAppOptions_Override = pAppOptions;
		return *this;
	}

public:
	SResult ReadAllValuesFromPathAsOptions(
		const CRegistryAccess& oReg,
		vlr::tzstring_view svzPath);

};

} // namespace win32

} // namespace vlr
