#pragma once

#include <combaseapi.h>

namespace vlr {

namespace util {

struct CCoInitSecuritySettings
{
public:
	PSECURITY_DESCRIPTOR m_pSecDesc = nullptr;
	LONG m_cAuthSvc = -1;
	SOLE_AUTHENTICATION_SERVICE* m_pArrAuthServices = nullptr;
	DWORD m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_DEFAULT;
	DWORD m_dwImpLevel = RPC_C_IMP_LEVEL_DEFAULT;
	void* m_pAuthList = nullptr;
	DWORD m_dwCapabilities = EOAC_DEFAULT;
};

} // namespace util

} // namespace vlr
