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

	inline decltype(auto) withAuthenticationLevel(DWORD dwAuthnLevel)
	{
		m_dwAuthnLevel = dwAuthnLevel;
		return *this;
	}
	inline decltype(auto) withAuthenticationLevel_None()
	{
		m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;
		return *this;
	}
	inline decltype(auto) withAuthenticationLevel_Connect()
	{
		m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_CONNECT;
		return *this;
	}
	inline decltype(auto) withAuthenticationLevel_Call()
	{
		m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_CALL;
		return *this;
	}
	inline decltype(auto) withAuthenticationLevel_Packet()
	{
		m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT;
		return *this;
	}
	inline decltype(auto) withAuthenticationLevel_Packet_WithIntegrity()
	{
		m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;
		return *this;
	}
	inline decltype(auto) withAuthenticationLevel_Packet_WithPrivacy()
	{
		m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
		return *this;
	}

	inline decltype(auto) withImpersonationLevel(DWORD dwImpLevel)
	{
		m_dwImpLevel = dwImpLevel;
		return *this;
	}
	inline decltype(auto) withImpersonationLevel_Anonymous()
	{
		m_dwImpLevel = RPC_C_IMP_LEVEL_ANONYMOUS;
		return *this;
	}
	inline decltype(auto) withImpersonationLevel_Identify()
	{
		m_dwImpLevel = RPC_C_IMP_LEVEL_IDENTIFY;
		return *this;
	}
	inline decltype(auto) withImpersonationLevel_Impersonate()
	{
		m_dwImpLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
		return *this;
	}
	inline decltype(auto) withImpersonationLevel_Delegate()
	{
		m_dwImpLevel = RPC_C_IMP_LEVEL_DELEGATE;
		return *this;
	}
};

} // namespace util

} // namespace vlr
