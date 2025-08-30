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

	constexpr auto& withAuthenticationLevel(DWORD dwAuthnLevel) noexcept
	{
		m_dwAuthnLevel = dwAuthnLevel;
		return *this;
	}
	constexpr auto& withAuthenticationLevel_None() noexcept
	{
		m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;
		return *this;
	}
	constexpr auto& withAuthenticationLevel_Connect() noexcept
	{
		m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_CONNECT;
		return *this;
	}
	constexpr auto& withAuthenticationLevel_Call() noexcept
	{
		m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_CALL;
		return *this;
	}
	constexpr auto& withAuthenticationLevel_Packet() noexcept
	{
		m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT;
		return *this;
	}
	constexpr auto& withAuthenticationLevel_Packet_WithIntegrity() noexcept
	{
		m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;
		return *this;
	}
	constexpr auto& withAuthenticationLevel_Packet_WithPrivacy() noexcept
	{
		m_dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
		return *this;
	}

	constexpr auto& withImpersonationLevel(DWORD dwImpLevel) noexcept
	{
		m_dwImpLevel = dwImpLevel;
		return *this;
	}
	constexpr auto& withImpersonationLevel_Anonymous() noexcept
	{
		m_dwImpLevel = RPC_C_IMP_LEVEL_ANONYMOUS;
		return *this;
	}
	constexpr auto& withImpersonationLevel_Identify() noexcept
	{
		m_dwImpLevel = RPC_C_IMP_LEVEL_IDENTIFY;
		return *this;
	}
	constexpr auto& withImpersonationLevel_Impersonate() noexcept
	{
		m_dwImpLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
		return *this;
	}
	constexpr auto& withImpersonationLevel_Delegate() noexcept
	{
		m_dwImpLevel = RPC_C_IMP_LEVEL_DELEGATE;
		return *this;
	}
};

} // namespace util

} // namespace vlr
