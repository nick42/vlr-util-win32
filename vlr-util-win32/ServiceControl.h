#pragma once

#include <vlr-util/util.includes.h>
#include <vlr-util/util.Result.h>
#include <vlr-util/ops.NetworkTargetInfo.h>
#include <vlr-util/util.AutoCleanup_SC_HANDLE.h>

#include <vlr-util-win32/ServiceConfig.h>

VLR_NAMESPACE_BEGIN( vlr )

VLR_NAMESPACE_BEGIN( win32 )

class CServiceControl
{
protected:
	cpp::shared_ptr<util::CAutoCleanup_SC_HANDLE> m_spAutoCleanupSCM;

	std::optional<SC_HANDLE> GetOpenHandle_SCM() const;

public:
	util::CResult Connect(
		const ops::CNetworkTargetInfo& oNetworkTargetInfo,
		DWORD dwDesiredAccess = SC_MANAGER_ALL_ACCESS );

	util::CResult SCM_CreateService(
		const CServiceConfig& oServiceConfig,
		SC_HANDLE& hService_Result );

	util::CResult SCM_DeleteService(
		SC_HANDLE hService );
	util::CResult SCM_DeleteService(
		const CServiceConfig& oServiceConfig );

	util::CResult SCM_OpenService(
		vlr::tzstring_view svzServiceName,
		DWORD dwDesiredAccess,
		SC_HANDLE& hService_Result );

};

VLR_NAMESPACE_END //( win32 )

VLR_NAMESPACE_END //( vlr )
