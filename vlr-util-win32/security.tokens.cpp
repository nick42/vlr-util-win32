#include "pch.h"
#include "security.tokens.h"

#include <vlr-util/util.includes.h>
#include <vlr-util/AutoFreeResource.h>

namespace vlr {

namespace win32 {

namespace security {

namespace tokens {

HRESULT SetPrivilegeOnToken(
	HANDLE hToken,
	vlr::tzstring_view_param svzPrivilegeName,
	bool bEnable)
{
	BOOL bResult{};

	LUID nPrivilegeID{};
	bResult = ::LookupPrivilegeValue(
		NULL,
		svzPrivilegeName,
		&nPrivilegeID);
	if (!bResult)
	{
		auto dwLastError = ::GetLastError();
		return HRESULT_FROM_WIN32(dwLastError);
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
		sizeof(TOKEN_PRIVILEGES),
		nullptr,
		nullptr);
	if (!bResult)
	{
		auto dwLastError = ::GetLastError();
		// Note: ERROR_NOT_ALL_ASSIGNED if we could not set privilege on token, but this returns TRUE...
		return HRESULT_FROM_WIN32(dwLastError);
	}
	// Judgement call here: AdjustTokenPrivileges returns TRUE and ERROR_NOT_ALL_ASSIGNED, for partial assignment. 
	// We should return S_FALSE for this case, as some privileges may have been set, but not all, and the caller 
	// may want to know this. However, in practice, it seems that the most likely case is that the privilege was 
	// not set at all, and the caller likely wanted it to be set, so we will return a failure code for this case instead.
	auto dwLastError = ::GetLastError();
	if (dwLastError == ERROR_NOT_ALL_ASSIGNED)
	{
		return HRESULT_FROM_WIN32(dwLastError);
	}

	return S_OK;
}

HRESULT SetPrivilegeOnProcess(
	HANDLE hProcess,
	vlr::tzstring_view_param svzPrivilegeName,
	bool bEnable)
{
	BOOL bResult{};

	HANDLE hToken{};
	bResult = ::OpenProcessToken(
		hProcess,
		TOKEN_ADJUST_PRIVILEGES,
		&hToken);
	VLR_ASSERT_NONZERO_OR_RETURN_HRESULT_LAST_ERROR(bResult);
	auto oOnDestroy_FreeToken = vlr::MakeAutoCleanup_viaCloseHandle(hToken);

	return SetPrivilegeOnToken(hToken, svzPrivilegeName, bEnable);
}

} // namespace tokens

} // namespace security

} // namespace win32

} // namespace vlr
