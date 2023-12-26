#include "pch.h"
#include "ServiceControl.h"

namespace vlr {

namespace win32 {

std::optional<SC_HANDLE> CServiceControl::GetOpenHandle_SCM() const
{
	static constexpr auto _tFailureValue = std::optional<SC_HANDLE>{};

	if (!m_spAutoCleanupSCM)
	{
		return {};
	}
	if (m_spAutoCleanupSCM->m_hSCM == nullptr)
	{
		VLR_HANDLE_ASSERTION_FAILURE__AND_RETURN_FAILURE_VALUE;
	}

	return m_spAutoCleanupSCM->m_hSCM;
}

SResult CServiceControl::Connect(
	const ops::CNetworkTargetInfo& oNetworkTargetInfo,
	DWORD dwDesiredAccess /*= SC_MANAGER_ALL_ACCESS*/ )
{
	m_spAutoCleanupSCM = {};

	auto hSCM = ::OpenSCManager(
		oNetworkTargetInfo.GetNameForIntent_win32_OpenSCManager(),
		SERVICES_ACTIVE_DATABASE,
		dwDesiredAccess );
	if (hSCM == NULL)
	{
		return SResult::For_win32_LastError();
	}

	m_spAutoCleanupSCM = cpp::make_shared<util::CAutoCleanup_SC_HANDLE>( hSCM );

	return S_OK;
}

SResult CServiceControl::SCM_CreateService(
	const CServiceConfig& oServiceConfig,
	SC_HANDLE& hService_Result )
{
	auto ohSCM = GetOpenHandle_SCM();
	if (!ohSCM.has_value())
	{
		return E_UNEXPECTED;
	}

	// Service name must be valid
	VLR_ASSERT_NOTBLANK_OR_RETURN_EUNEXPECTED( oServiceConfig.m_svzServiceName );
	VLR_ASSERT_NOTBLANK_OR_RETURN_EUNEXPECTED( oServiceConfig.m_svzServiceName_Display );

	// Some additional checks for validity, per docs
	VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED( oServiceConfig.m_svzServiceName.size(), <= , 256 );
	VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED( oServiceConfig.m_svzServiceName.find( _T( '/' ) ), == , oServiceConfig.m_svzServiceName.npos );
	VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED( oServiceConfig.m_svzServiceName.find( _T( '\\' ) ), == , oServiceConfig.m_svzServiceName.npos );
	VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED( oServiceConfig.m_svzServiceName_Display.size(), <= , 256 );

	auto hService = ::CreateService(
		ohSCM.value(),
		oServiceConfig.m_svzServiceName,
		oServiceConfig.m_svzServiceName_Display,
		oServiceConfig.m_dwDesiredAccess,
		oServiceConfig.m_dwServiceType,
		oServiceConfig.m_dwStartType,
		oServiceConfig.m_dwErrorControl,
		oServiceConfig.m_sFilePath_ServiceBinary.c_str(),
		oServiceConfig.m_svzLoadOrderGroup,
		oServiceConfig.m_lpdwTagId,
		oServiceConfig.m_svzDependencies,
		oServiceConfig.m_svzRunAsAccount_Username,
		oServiceConfig.m_svzRunAsAccount_Password );
	if (hService == NULL)
	{
		return SResult::For_win32_LastError();
	}

	hService_Result = hService;

	return S_OK;
}

SResult CServiceControl::SCM_DeleteService(
	SC_HANDLE hService )
{
	auto bResult = ::DeleteService(
		hService );
	if (!bResult)
	{
		return SResult::For_win32_LastError();
	}

	return S_OK;
}

SResult CServiceControl::SCM_DeleteService(
	const CServiceConfig& oServiceConfig )
{
	HRESULT hr;

	// Service name must be valid
	VLR_ASSERT_NOTBLANK_OR_RETURN_EUNEXPECTED( oServiceConfig.m_svzServiceName );

	SC_HANDLE hService{};
	hr = SCM_OpenService(
		oServiceConfig.m_svzServiceName,
		DELETE,
		hService );
	VLR_ON_HR_NON_S_OK__RETURN_HRESULT( hr );
	auto oOnDestroy_CloseService = util::CAutoCleanup_SC_HANDLE{ hService };

	return SCM_DeleteService( hService );
}

SResult CServiceControl::SCM_OpenService(
	vlr::tzstring_view svzServiceName,
	DWORD dwDesiredAccess,
	SC_HANDLE& hService_Result )
{
	auto ohSCM = GetOpenHandle_SCM();
	if (!ohSCM.has_value())
	{
		return E_UNEXPECTED;
	}

	auto hService = ::OpenService(
		ohSCM.value(),
		svzServiceName,
		dwDesiredAccess );
	if (hService == NULL)
	{
		return SResult::For_win32_LastError();
	}

	hService_Result = hService;

	return S_OK;
}

} // namespace win32

} // namespace vlr
