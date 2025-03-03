#include "pch.h"
#include "security.SECURITY_DESCRIPTOR.h"

namespace vlr {

namespace util {

namespace win32 {

namespace security {

SResult CSecurityDescriptor::InitSecurityDescriptor()
{
	BOOL bResult = InitializeSecurityDescriptor(&m_stSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
	VLR_ASSERT_NONZERO_OR_RETURN_HRESULT_LAST_ERROR(bResult);

	return SResult::Success;
}

SResult CSecurityDescriptor::SetDacl(PACL pAcl, const CSetAclOptions& oSetAclOptions /*= {}*/)
{
	if ((!pAcl) && (oSetAclOptions.m_bAllowNull))
	{
		return SResult::Failure;
	}

	BOOL bResult = SetSecurityDescriptorDacl(&m_stSecurityDescriptor, TRUE, pAcl, oSetAclOptions.m_bShowAsDefaulted ? TRUE : FALSE);
	VLR_ASSERT_NONZERO_OR_RETURN_HRESULT_LAST_ERROR(bResult);

	return SResult::Success;
}

SResult CSecurityDescriptor::SetDacl_Cleared()
{
	BOOL bResult = SetSecurityDescriptorDacl(&m_stSecurityDescriptor, FALSE, nullptr, FALSE);
	VLR_ASSERT_NONZERO_OR_RETURN_HRESULT_LAST_ERROR(bResult);

	return SResult::Success;
}

SResult CSecurityDescriptor::SetSacl(PACL pAcl, const CSetAclOptions& oSetAclOptions /*= {}*/)
{
	if ((!pAcl) && (oSetAclOptions.m_bAllowNull))
	{
		return SResult::Failure;
	}

	BOOL bResult = SetSecurityDescriptorSacl(&m_stSecurityDescriptor, TRUE, pAcl, oSetAclOptions.m_bShowAsDefaulted ? TRUE : FALSE);
	VLR_ASSERT_NONZERO_OR_RETURN_HRESULT_LAST_ERROR(bResult);

	return SResult::Success;
}

SResult CSecurityDescriptor::SetSacl_Cleared()
{
	BOOL bResult = SetSecurityDescriptorSacl(&m_stSecurityDescriptor, FALSE, nullptr, FALSE);
	VLR_ASSERT_NONZERO_OR_RETURN_HRESULT_LAST_ERROR(bResult);

	return SResult::Success;
}

} // namespace security

} // namespace win32

} // namespace util

} // namespace vlr
