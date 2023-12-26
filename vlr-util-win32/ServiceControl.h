#pragma once

#include <vlr-util/util.includes.h>
#include <vlr-util/util.Result.h>
#include <vlr-util/ops.NetworkTargetInfo.h>
#include <vlr-util/util.AutoCleanup_SC_HANDLE.h>

#include <vlr-util-win32/ServiceConfig.h>

namespace vlr {

namespace win32 {

class CServiceControl
{
protected:
	cpp::shared_ptr<util::CAutoCleanup_SC_HANDLE> m_spAutoCleanupSCM;

	std::optional<SC_HANDLE> GetOpenHandle_SCM() const;

public:
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
		vlr::tzstring_view svzServiceName,
		DWORD dwDesiredAccess,
		SC_HANDLE& hService_Result );

};

} // namespace win32

} // namespace vlr
