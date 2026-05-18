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
	DWORD dwDesiredAccess /*= SC_MANAGER_ALL_ACCESS*/)
{
	m_spAutoCleanupSCM = {};

	auto hSCM = ::OpenSCManager(
		oNetworkTargetInfo.GetNameForIntent_win32_OpenSCManager(),
		SERVICES_ACTIVE_DATABASE,
		dwDesiredAccess);
	if (hSCM == NULL)
	{
		return SResult::For_win32_LastError();
	}

	m_spAutoCleanupSCM = cpp::make_shared<util::win32::CAutoCleanup_SC_HANDLE>(hSCM);

	return S_OK;
}

SResult CServiceControl::SCM_CreateService(
	const CServiceConfig& oServiceConfig,
	SC_HANDLE& hService_Result)
{
	auto ohSCM = GetOpenHandle_SCM();
	if (!ohSCM.has_value())
	{
		return E_UNEXPECTED;
	}

	// Service name must be valid
	VLR_ASSERT_NOTBLANK_OR_RETURN_EUNEXPECTED(oServiceConfig.m_sServiceName);
	VLR_ASSERT_NOTBLANK_OR_RETURN_EUNEXPECTED(oServiceConfig.m_sServiceName_Display);

	// Some additional checks for validity, per docs
	VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(oServiceConfig.m_sServiceName.size(), <= , 256);
	VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(oServiceConfig.m_sServiceName.find(_T('/')), == , oServiceConfig.m_sServiceName.npos);
	VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(oServiceConfig.m_sServiceName.find(_T('\\')), == , oServiceConfig.m_sServiceName.npos);
	VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(oServiceConfig.m_sServiceName_Display.size(), <= , 256);

	// For security safety, the binary path should either not contain spaces, or be quoted if it does. This is 
	// because of the way the SCM parses the binary path; if there are spaces, it may be parsed in a way which 
	// allows for unexpected behavior. 
	// See: https://docs.microsoft.com/en-us/windows/win32/services/service-programs-and-their-executable-files#service-binary-pathname-rules
	if (oServiceConfig.m_sFilePath_ServiceBinary.find(_T(' ')) != oServiceConfig.m_sFilePath_ServiceBinary.npos)
	{
		VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(oServiceConfig.m_sFilePath_ServiceBinary.size(), >= , 3);
		VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(oServiceConfig.m_sFilePath_ServiceBinary.front(), == , _T('"'));
		VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(oServiceConfig.m_sFilePath_ServiceBinary.back(), == , _T('"'));
	}
	// Note: We could do some additional checks here, such as checking that the file path is absolute, or that 
	// it points to a valid file. However, these checks may be outside the scope of this function, and may not 
	// be necessary for all use cases.

	auto hService = ::CreateService(
		ohSCM.value(),
		oServiceConfig.m_sServiceName.c_str(),
		oServiceConfig.m_sServiceName_Display.c_str(),
		oServiceConfig.m_dwDesiredAccess,
		oServiceConfig.m_dwServiceType,
		oServiceConfig.m_dwStartType,
		oServiceConfig.m_dwErrorControl,
		oServiceConfig.m_sFilePath_ServiceBinary.c_str(),
		oServiceConfig.m_svzLoadOrderGroup,
		oServiceConfig.m_lpdwTagId,
		oServiceConfig.m_svzDependencies,
		oServiceConfig.m_svzRunAsAccount_Username,
		oServiceConfig.m_svzRunAsAccount_Password);
	if (hService == NULL)
	{
		return SResult::For_win32_LastError();
	}

	hService_Result = hService;

	return S_OK;
}

SResult CServiceControl::SCM_DeleteService(
	SC_HANDLE hService)
{
	auto bResult = ::DeleteService(
		hService);
	if (!bResult)
	{
		return SResult::For_win32_LastError();
	}

	return S_OK;
}

SResult CServiceControl::SCM_DeleteService(
	const CServiceConfig& oServiceConfig)
{
	HRESULT hr;

	// Service name must be valid
	VLR_ASSERT_NOTBLANK_OR_RETURN_EUNEXPECTED(oServiceConfig.m_sServiceName);

	SC_HANDLE hService{};
	hr = SCM_OpenService(
		oServiceConfig.m_sServiceName.c_str(),
		DELETE,
		hService);
	VLR_ON_HR_NON_S_OK__RETURN_HRESULT(hr);
	auto oOnDestroy_CloseService = util::win32::CAutoCleanup_SC_HANDLE{ hService };

	return SCM_DeleteService(hService);
}

SResult CServiceControl::SCM_OpenService(
	vlr::tzstring_view_param svzServiceName,
	DWORD dwDesiredAccess,
	SC_HANDLE& hService_Result)
{
	auto ohSCM = GetOpenHandle_SCM();
	if (!ohSCM.has_value())
	{
		return E_UNEXPECTED;
	}

	auto hService = ::OpenService(
		ohSCM.value(),
		svzServiceName,
		dwDesiredAccess);
	if (hService == NULL)
	{
		return SResult::For_win32_LastError();
	}

	hService_Result = hService;

	return S_OK;
}

SResult CServiceControl::SCM_QueryServiceConfig(
	SC_HANDLE hService,
	std::vector<BYTE>& vecServiceConfigData)
{
	VLR_ASSERT_COMPARE_OR_RETURN_EXPRESSION(hService, != , nullptr, HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));
	VLR_ASSERT_COMPARE_OR_RETURN_EXPRESSION(hService, != , INVALID_HANDLE_VALUE, HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));

	// Pick a semi-reasonable default size, if buffer not pre-allocated
	// Note: Docs say 8K max data size
	if (vecServiceConfigData.size() == 0)
	{
		vecServiceConfigData.resize(4096);
	}

	for (size_t nAttempt = 0; nAttempt < 2; ++nAttempt)
	{
		DWORD dwBufferSizeBytes = vlr::util::range_checked_cast<DWORD>(vecServiceConfigData.size());

		DWORD dwRequiredBufferSize{};
		BOOL bResult = QueryServiceConfig(
			hService,
			reinterpret_cast<LPQUERY_SERVICE_CONFIG>(vecServiceConfigData.data()),
			dwBufferSizeBytes,
			&dwRequiredBufferSize);
		if (bResult)
		{
			return S_OK;
		}
		DWORD dwLastError = ::GetLastError();
		if (dwLastError == ERROR_INSUFFICIENT_BUFFER)
		{
			VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(dwRequiredBufferSize, > , dwBufferSizeBytes);
			vecServiceConfigData.resize(dwRequiredBufferSize);
			continue;
		}
		return SResult::For_win32_ErrorCode(dwLastError);
	}

	// Ran out of retries

	return E_UNEXPECTED;
}

} // namespace win32

} // namespace vlr
