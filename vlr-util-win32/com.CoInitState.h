#pragma once

#include <combaseapi.h>
#include <memory>

#include <vlr-util/util.Result.h>

#include "com.CoInitSecuritySettings.h"

namespace vlr {

namespace util {

struct CoInitFlags
{
public:
	DWORD m_dwCoInit = COINIT_MULTITHREADED;
};

class CCoInitState
{
public:
	CoInitFlags m_oFlags;
	DWORD m_dwInitThreadID = 0;
	SResult m_srCoInitialize;

	CCoInitSecuritySettings m_oInitializeSecuritySettings;
	SResult m_srCoInitializeSecurity;

protected:
	SResult IfApplicable_Uninitialize();
	SResult OnCoInitialize_ClearExistingData();

public:
	inline bool IsInitializedOnThread() const
	{
		return m_srCoInitialize.isSet();
	}
	inline bool IsInitializedToCompatibleApartmentModel() const
	{
		return m_srCoInitialize.isSuccess();
	}

	SResult CallCoInitialize(const CoInitFlags& oFlags);
	SResult CallCoInitializeSecurity(const CCoInitSecuritySettings& oInitializeSecuritySettings);

public:
	~CCoInitState()
	{
		IfApplicable_Uninitialize();
	}
};
using SPCCoInitState = std::shared_ptr<CCoInitState>;

} // namespace util

} // namespace vlr
