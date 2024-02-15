#include "pch.h"
#include "com.CoInitState.h"

#include <combaseapi.h>

namespace vlr {

namespace util {

SResult CCoInitState::IfApplicable_Uninitialize()
{
	// We should release the COM runtime once per successful initialization call per thread (even for S_FALSE)

	if (!m_srCoInitialize.isSuccess())
	{
		return SResult::Success_NoWorkDone;
	}

	::CoUninitialize();

	return SResult::Success;
}

SResult CCoInitState::OnCoInitialize_ClearExistingData()
{
	if (!m_srCoInitialize.isSet())
	{
		return SResult::Success_NoWorkDone;
	}

	IfApplicable_Uninitialize();

	*this = {};

	return SResult::Success;
}

SResult CCoInitState::CallCoInitialize(const CoInitFlags& oFlags)
{
	OnCoInitialize_ClearExistingData();

	m_dwInitThreadID = ::GetCurrentThreadId();
	m_oFlags = oFlags;
	m_srCoInitialize = ::CoInitializeEx(NULL, m_oFlags.m_dwCoInit);

	return m_srCoInitialize;
}

SResult CCoInitState::CallCoInitializeSecurity(const CCoInitSecuritySettings& oInitializeSecuritySettings)
{
	if (m_srCoInitializeSecurity.isSet())
	{
		// Should be warning of some sort?
	}

	m_oInitializeSecuritySettings = oInitializeSecuritySettings;
	m_srCoInitializeSecurity = {};

	m_srCoInitializeSecurity = ::CoInitializeSecurity(
		m_oInitializeSecuritySettings.m_pSecDesc,
		m_oInitializeSecuritySettings.m_cAuthSvc,
		m_oInitializeSecuritySettings.m_pArrAuthServices,
		NULL,
		m_oInitializeSecuritySettings.m_dwAuthnLevel,
		m_oInitializeSecuritySettings.m_dwImpLevel,
		m_oInitializeSecuritySettings.m_pAuthList,
		m_oInitializeSecuritySettings.m_dwCapabilities,
		NULL);

	return m_srCoInitializeSecurity;
}

} // namespace util

} // namespace vlr
