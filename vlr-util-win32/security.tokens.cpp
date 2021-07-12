#include "pch.h"
#include "security.tokens.h"

#include <vlr-util/util.includes.h>
#include <vlr-util/AutoFreeResource.h>

VLR_NAMESPACE_BEGIN( vlr )

VLR_NAMESPACE_BEGIN( win32 )

VLR_NAMESPACE_BEGIN( security )

VLR_NAMESPACE_BEGIN( tokens )

HRESULT SetPrivilegeOnToken(
	HANDLE hToken,
    vlr::tzstring_view svzPrivilegeName,
    bool bEnable )
{
    BOOL bResult{};

    LUID nPrivilegeID{};
    bResult = ::LookupPrivilegeValue(
        NULL,
        svzPrivilegeName,
        &nPrivilegeID );
    if (!bResult)
    {
        auto dwLastError = ::GetLastError();
        return HRESULT_FROM_WIN32( dwLastError );
    }

    TOKEN_PRIVILEGES oTokenPrivileges{};
    oTokenPrivileges.PrivilegeCount = 1;
    auto& oPrivilege0 = oTokenPrivileges.Privileges[0];
    oPrivilege0.Luid = nPrivilegeID;
    oPrivilege0.Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

    bResult = ::AdjustTokenPrivileges(
        hToken,
        FALSE,
        &oTokenPrivileges,
        sizeof( TOKEN_PRIVILEGES ),
        nullptr,
        nullptr );
    if (!bResult)
    {
        auto dwLastError = ::GetLastError();
        // Note: ERROR_NOT_ALL_ASSIGNED if we could not set privilege on token...
        return HRESULT_FROM_WIN32( dwLastError );
    }

    return S_OK;
}

HRESULT SetPrivilegeOnProcess(
    HANDLE hProcess,
    vlr::tzstring_view svzPrivilegeName,
    bool bEnable )
{
    BOOL bResult{};

    HANDLE hToken{};
    bResult = ::OpenProcessToken(
        hProcess,
        TOKEN_ADJUST_PRIVILEGES,
        &hToken );
    VLR_ASSERT_NONZERO__OR_RETURN_HRESULT_LAST_ERROR( bResult );
    auto oOnDestroy_FreeToken = vlr::MakeAutoCleanup_viaCloseHandle( hToken );

    return SetPrivilegeOnToken( hToken, svzPrivilegeName, bEnable );
}

VLR_NAMESPACE_END //( tokens )

VLR_NAMESPACE_END //( security )

VLR_NAMESPACE_END //( win32 )

VLR_NAMESPACE_END //( vlr )
