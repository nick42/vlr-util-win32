#include "pch.h"
#include "RegistryAccess.h"

#include "vlr-util/StringCompare.h"

#include "AutoCleanupTypedefs.h"

VLR_NAMESPACE_BEGIN(vlr)

VLR_NAMESPACE_BEGIN(win32)

SResult RegistryAccess::CheckKeyExists(tzstring_view svzKeyName) const
{
	SResult sr;

	HKEY hKey{};
	sr = openKey(svzKeyName, KEY_READ, hKey);
	if (!sr.isSuccess())
	{
		return SResult::Success_WithNuance;
	}
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(hKey);
	auto onDestroy_CloseRegKey = AutoCloseRegKey{ hKey };

	return SResult::Success;
}

SResult RegistryAccess::EnsureKeyExists(
	tzstring_view svzKeyName,
	const Options_EnsureKeyExists& options /*= {}*/) const
{
	SResult sr;
	LONG lResult{};

	// Note: The create (below) checks for existing already, so this is unnecessary
	//sr = CheckKeyExists(svzKeyName);
	//VLR_ON_SR_SUCCESS_RETURN_VALUE(sr);

	// TODO? Add security attributes handling

	HKEY hKey{};
	DWORD dwDisposition{};
	lResult = RegCreateKeyEx(
		getBaseKey(),
		svzKeyName,
		0,
		NULL,
		0,
		KEY_ALL_ACCESS | getWow64RedirectionKeyAccessMask(),
		NULL,
		&hKey,
		&dwDisposition);
	if (lResult != ERROR_SUCCESS)
	{
		return __HRESULT_FROM_WIN32(lResult);
	}
	auto onDestroy_CloseRegKey = AutoCloseRegKey{ hKey };

	switch (dwDisposition)
	{
	case REG_CREATED_NEW_KEY:
	case REG_OPENED_EXISTING_KEY:
		return SResult::Success;

	default:
		VLR_HANDLE_ASSERTION_FAILURE__AND_RETURN_EXPRESSION(SResult::Failure);
	}
}

SResult RegistryAccess::DeleteKey(
	tzstring_view svzKeyName,
	const Options_DeleteKey& options /*= {}*/) const
{
	SResult sr;
	LONG lResult{};

	if (options.m_bEnsureSafeDelete)
	{
		bool bKeyUnderSafeDeletePath = false;
		for (const auto& sPath : options.m_arrSafeDeletePaths)
		{
			if (StringCompare::CI().StringHasPrefix(svzKeyName, sPath))
			{
				bKeyUnderSafeDeletePath = true;
				break;
			}
		}
		if (!bKeyUnderSafeDeletePath)
		{
			return SResult::Failure;
		}
	}

	lResult = ::RegDeleteKey(
		getBaseKey(),
		svzKeyName);
	if (lResult != ERROR_SUCCESS)
	{
		return __HRESULT_FROM_WIN32(lResult);
	}

	return SResult::Success;
}

SResult RegistryAccess::openKey(
	tzstring_view svzKeyName,
	DWORD dwAccessMask,
	HKEY& hKey_Result) const
{
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(m_hBaseKey);

	LONG lResult = ::RegOpenKeyEx(
		getBaseKey(),
		svzKeyName,
		0,
		dwAccessMask | getWow64RedirectionKeyAccessMask(),
		&hKey_Result);
	if (lResult == ERROR_SUCCESS)
	{
		return SResult::Success;
	}

	return __HRESULT_FROM_WIN32(lResult);
}

DWORD RegistryAccess::getWow64RedirectionKeyAccessMask() const
{
	return 0;
}

VLR_NAMESPACE_END //(win32)

VLR_NAMESPACE_END //(vlr)
