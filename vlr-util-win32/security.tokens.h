#pragma once

#include <vlr-util/util.includes.h>
#include <vlr-util/zstring_view.h>

VLR_NAMESPACE_BEGIN( vlr )

VLR_NAMESPACE_BEGIN( win32 )

VLR_NAMESPACE_BEGIN( security )

VLR_NAMESPACE_BEGIN( tokens )

HRESULT SetPrivilegeOnToken(
	HANDLE hToken,
	vlr::tzstring_view svzPrivilegeName,
	bool bEnable );

HRESULT SetPrivilegeOnProcess(
	HANDLE hProcess,
	vlr::tzstring_view svzPrivilegeName,
	bool bEnable );

VLR_NAMESPACE_END //( tokens )

VLR_NAMESPACE_END //( security )

VLR_NAMESPACE_END //( win32 )

VLR_NAMESPACE_END //( vlr )
