#pragma once

#include <vlr-util/util.includes.h>
#include <vlr-util/util.Result.h>
#include <vlr-util/ops.NetworkTargetInfo.h>

#include <vlr-util-win32/ServiceConfig.h>
#include <vlr-util-win32/util.AutoCleanup_SC_HANDLE.h>

namespace vlr {

namespace win32 {

class CServiceControl
{
protected:
	cpp::shared_ptr<util::win32::CAutoCleanup_SC_HANDLE> m_spAutoCleanupSCM;

	std::optional<SC_HANDLE> GetOpenHandle_SCM() const;

public:
	inline SC_HANDLE GetOpenHandle_SCM_OrNull() const
	{
		auto optHandle = GetOpenHandle_SCM();
		return optHandle.value_or(nullptr);
	}

	SResult Connect(
		const ops::CNetworkTargetInfo& oNetworkTargetInfo,
		DWORD dwDesiredAccess = SC_MANAGER_ALL_ACCESS );

	SResult SCM_CreateService(
		const CServiceConfig& oServiceConfig,
		SC_HANDLE& hService_Result );

	SResult SCM_DeleteService(
		SC_HANDLE hService );
	SResult SCM_DeleteService(
		const CServiceConfig& oServiceConfig );

	SResult SCM_OpenService(
		vlr::tzstring_view_param svzServiceName,
		DWORD dwDesiredAccess,
		SC_HANDLE& hService_Result );

	SResult SCM_QueryServiceConfig(
		SC_HANDLE hService,
		std::vector<BYTE>& vecServiceConfigData);

};

} // namespace win32

} // namespace vlr
