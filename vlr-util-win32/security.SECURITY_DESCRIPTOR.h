#pragma once

#include "vlr-util/util.Result.h"

namespace vlr {

namespace util {

namespace win32 {

namespace security {

struct CSetAclOptions
{
	// This is a semantic flag, intended to show if the value is derived from a defaulted mechanism.
	bool m_bShowAsDefaulted = false;

	// Note: Setting an explicit NULL ACL means that any/all access is allowed, including a user taking 
	// ownership of an object. This is usually bad/unintended. So, we'll have a flag to indicate if 
	// this is actually intended, and guard against accidental mistakes.
	bool m_bAllowNull = false;

	constexpr auto& withShowAsDefaulted(bool bValue) noexcept
	{
		m_bShowAsDefaulted = bValue;
		return *this;
	}
	constexpr auto& withAllowNull(bool bValue) noexcept
	{
		m_bAllowNull = bValue;
		return *this;
	}
};

class CSecurityDescriptor
{
protected:
	SECURITY_DESCRIPTOR m_stSecurityDescriptor{};
	bool m_bRawValueSet_Dacl = false;
	bool m_bRawValueSet_Sacl = false;

protected:
	SResult InitSecurityDescriptor();

public:
	inline auto& withDacl(PACL pAcl, const CSetAclOptions& oSetAclOptions = {})
	{
		SetDacl(pAcl, oSetAclOptions);
		return *this;
	}
	inline auto& withDacl_Cleared()
	{
		SetDacl_Cleared();
		return *this;
	}
	inline auto& withSacl(PACL pAcl, const CSetAclOptions& oSetAclOptions = {})
	{
		SetSacl(pAcl, oSetAclOptions);
		return *this;
	}
	inline auto& withSacl_Cleared()
	{
		SetSacl_Cleared();
		return *this;
	}

public:
	SResult SetDacl(PACL pAcl, const CSetAclOptions& oSetAclOptions = {});
	SResult SetDacl_Cleared();
	SResult SetSacl(PACL pAcl, const CSetAclOptions& oSetAclOptions = {});
	SResult SetSacl_Cleared();

public:
	constexpr SECURITY_DESCRIPTOR* GetWin32Ptr()
	{
		return &m_stSecurityDescriptor;
	}
};

} // namespace security

} // namespace win32

} // namespace util

} // namespace vlr
