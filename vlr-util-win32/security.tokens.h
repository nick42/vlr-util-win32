#pragma once

#include <vlr-util/util.includes.h>
#include <vlr-util/zstring_view.h>

namespace vlr {

namespace win32 {

namespace security {

namespace tokens {

HRESULT SetPrivilegeOnToken(
	HANDLE hToken,
	vlr::tzstring_view svzPrivilegeName,
	bool bEnable );

HRESULT SetPrivilegeOnProcess(
	HANDLE hProcess,
	vlr::tzstring_view svzPrivilegeName,
	bool bEnable );

} // namespace tokens

} // namespace security

} // namespace win32

} // namespace vlr
