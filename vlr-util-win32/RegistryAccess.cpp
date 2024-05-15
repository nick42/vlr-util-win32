#include "pch.h"
#include "RegistryAccess.h"

#include "vlr-util/StringCompare.h"
#include "vlr-util/util.range_checked_cast.h"
#include "vlr-util/util.convert.StringConversion.h"

#include "AutoCleanupTypedefs.h"
#include "ModuleContext.Runtime.h"

namespace vlr {

namespace win32 {

SResult CRegistryAccess::CheckKeyExists(tzstring_view svzKeyName) const
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

SResult CRegistryAccess::EnsureKeyExists(
	tzstring_view svzKeyName,
	const Options_EnsureKeyExists& /*options*/ /*= {}*/) const
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

SResult CRegistryAccess::DeleteKey(
	tzstring_view svzKeyName,
	const Options_DeleteKeysOrValues& options /*= {}*/) const
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

SResult CRegistryAccess::ReadValueInfo(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	DWORD& dwType_Result,
	DWORD& dwSize_Result) const
{
	SResult sr;
	LONG lResult{};

	HKEY hKey{};
	sr = openKey(svzKeyName, KEY_READ, hKey);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(hKey);
	auto onDestroy_CloseRegKey = AutoCloseRegKey{ hKey };

	lResult = RegQueryValueEx(
		hKey,
		svzValueName,
		NULL,
		&dwType_Result,
		NULL,
		&dwSize_Result);
	// Note: This appears to always return ERROR_SUCCESS
	if (lResult == ERROR_SUCCESS)
	{
		return SResult::Success;
	}
	// ... but this would also sorta be expected
	if (lResult == ERROR_MORE_DATA)
	{
		return SResult::Success;
	}

	return SResult::For_win32_ErrorCode(lResult);
}

SResult CRegistryAccess::ReadValueBase(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	DWORD& dwType_Result,
	std::vector<BYTE>& arrData) const
{
	SResult sr;
	LONG lResult{};

	HKEY hKey{};
	sr = openKey(svzKeyName, KEY_READ, hKey);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(hKey);
	auto onDestroy_CloseRegKey = AutoCloseRegKey{ hKey };

	// Note: For the query, a data size 0 indicates that the data is not required. This is not 
	// what we want here. So we need to set a default if not provided.
	if (arrData.size() == 0)
	{
		arrData.resize(m_OnReadValue_nDefaultBufferSize);
	}

	size_t nIterationCount = 0;
	while (true)
	{
		nIterationCount++;

		DWORD dwBufferSize = util::range_checked_cast<DWORD>(arrData.size());
		lResult = RegQueryValueEx(
			hKey,
			svzValueName,
			NULL,
			&dwType_Result,
			arrData.data(),
			&dwBufferSize);
		if (lResult == ERROR_SUCCESS)
		{
			// Truncate buffer to data size
			arrData.resize(dwBufferSize);
			break;
		}
		if (lResult == ERROR_MORE_DATA)
		{
			if (nIterationCount >= m_nMaxIterationCountForRead)
			{
				// TODO: Add context for error
				return E_FAIL;
			}

			arrData.resize(dwBufferSize);
			continue;
		}

		// We got an unhandled/unexpected error
		return __HRESULT_FROM_WIN32(lResult);
	}

	return S_OK;
}

SResult CRegistryAccess::WriteValueBase(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	const DWORD& dwType,
	cpp::span<const BYTE> spanData) const
{
	SResult sr;
	LONG lResult{};

	HKEY hKey{};
	sr = openKey(svzKeyName, KEY_WRITE, hKey);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(hKey);
	auto onDestroy_CloseRegKey = AutoCloseRegKey{ hKey };

	size_t nIterationCount = 0;
	while (true)
	{
		nIterationCount++;

		DWORD dwBufferSize = util::range_checked_cast<DWORD>(spanData.size());
		lResult = RegSetValueEx(
			hKey,
			svzValueName,
			NULL,
			dwType,
			spanData.data(),
			dwBufferSize);
		if (lResult == ERROR_SUCCESS)
		{
			break;
		}

		// We got an unhandled/unexpected error
		return __HRESULT_FROM_WIN32(lResult);
	}

	return S_OK;
}

SResult CRegistryAccess::ReadValue_String(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	std::string& saValue) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = ReadValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = convertRegDataToValue_String(
		dwType,
		arrData,
		saValue);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::ReadValue_String(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	std::string& saValue,
	const std::string& saDefaultResultOnNoValue) const
{
	SResult sr;

	sr = ReadValue_String(
		svzKeyName,
		svzValueName,
		saValue);
	if (sr.asHRESULT() == __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		saValue = saDefaultResultOnNoValue;
		return SResult::Success;
	}
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::WriteValue_String(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	const std::string& saValue) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = convertValueToRegData_String(
		saValue,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = WriteValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::WriteValue_String(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	const std::string_view& svValue) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = convertValueToRegData_String(
		svValue,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = WriteValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::ReadValue_String(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	std::wstring& swValue) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = ReadValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = convertRegDataToValue_String(
		dwType,
		arrData,
		swValue);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::ReadValue_String(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	std::wstring& swValue,
	const std::wstring& swDefaultResultOnNoValue) const
{
	SResult sr;

	sr = ReadValue_String(
		svzKeyName,
		svzValueName,
		swValue);
	if (sr.asHRESULT() == __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		swValue = swDefaultResultOnNoValue;
		return SResult::Success;
	}
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::WriteValue_String(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	const std::wstring& swValue) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = convertValueToRegData_String(
		swValue,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = WriteValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::WriteValue_String(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	const std::wstring_view& svValue) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = convertValueToRegData_String(
		svValue,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = WriteValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::ReadValue_DWORD(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	DWORD& dwValue) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = ReadValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = convertRegDataToValue_DWORD(
		dwType,
		arrData,
		dwValue);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::ReadValue_DWORD(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	DWORD& dwValue,
	const DWORD& dwDefaultResultOnNoValue) const
{
	SResult sr;

	sr = ReadValue_DWORD(
		svzKeyName,
		svzValueName,
		dwValue);
	if (sr.asHRESULT() == __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		dwValue = dwDefaultResultOnNoValue;
		return SResult::Success;
	}
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::WriteValue_DWORD(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	const DWORD& dwValue) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = convertValueToRegData_DWORD(
		dwValue,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = WriteValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::ReadValue_QWORD(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	QWORD& qwValue) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = ReadValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = convertRegDataToValue_QWORD(
		dwType,
		arrData,
		qwValue);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::ReadValue_QWORD(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	QWORD& qwValue,
	const QWORD& qwDefaultResultOnNoValue) const
{
	SResult sr;

	sr = ReadValue_QWORD(
		svzKeyName,
		svzValueName,
		qwValue);
	if (sr.asHRESULT() == __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		qwValue = qwDefaultResultOnNoValue;
		return SResult::Success;
	}
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::WriteValue_QWORD(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	const QWORD& qwValue) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = convertValueToRegData_QWORD(
		qwValue,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = WriteValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::ReadValue_MultiSz(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	std::vector<vlr::tstring>& arrValueCollection) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = ReadValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = convertRegDataToValue_MultiSz(
		dwType,
		arrData,
		arrValueCollection);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::ReadValue_MultiSz(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	std::vector<vlr::tstring>& arrValueCollection,
	const std::vector<vlr::tstring>& arrDefaultResultOnNoValue) const
{
	SResult sr;

	sr = ReadValue_MultiSz(
		svzKeyName,
		svzValueName,
		arrValueCollection);
	if (sr.asHRESULT() == __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		arrValueCollection = arrDefaultResultOnNoValue;
		return SResult::Success;
	}
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::WriteValue_MultiSz(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	const std::vector<vlr::tstring>& arrValueCollection) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = convertValueToRegData_MultiSz(
		arrValueCollection,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = WriteValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

// Note: Binary can be made faster by optimizing to not copy data.
// Not doing this initially for consistency; update if speed for this type becomes desired.

SResult CRegistryAccess::ReadValue_Binary(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	std::vector<BYTE>& arrBinaryData) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = ReadValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = convertRegDataToValue_Binary(
		dwType,
		arrData,
		arrBinaryData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::ReadValue_Binary(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	std::vector<BYTE>& arrBinaryData,
	const std::vector<BYTE>& arrDefaultResultOnNoValue) const
{
	SResult sr;

	sr = ReadValue_Binary(
		svzKeyName,
		svzValueName,
		arrBinaryData);
	if (sr.asHRESULT() == __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		arrBinaryData = arrDefaultResultOnNoValue;
		return SResult::Success;
	}
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::WriteValue_Binary(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	cpp::span<const BYTE> spanData) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = convertValueToRegData_Binary(
		spanData,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	sr = WriteValueBase(
		svzKeyName,
		svzValueName,
		dwType,
		arrData);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::DeleteValue(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	const Options_DeleteKeysOrValues& options /*= {}*/)
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

	HKEY hKey{};
	sr = openKey(svzKeyName, KEY_WRITE, hKey);
	if (!sr.isSuccess())
	{
		return SResult::Success_WithNuance;
	}
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(hKey);
	auto onDestroy_CloseRegKey = AutoCloseRegKey{ hKey };

	lResult = ::RegDeleteValue(
		hKey,
		svzValueName);
	if (lResult != ERROR_SUCCESS)
	{
		return __HRESULT_FROM_WIN32(lResult);
	}

	return SResult::Success;
}

SResult CRegistryAccess::convertRegDataToValue_String(
	const DWORD& dwType,
	cpp::span<const BYTE> spanData,
	std::string& saValue) const
{
	SResult sr;

	bool bDirectConversion = false;

	switch (dwType)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
		bDirectConversion = true;
		break;

	default:
		return E_UNEXPECTED;
	}

	if (bDirectConversion)
	{
		if constexpr (ModuleContext::Compilation::DefaultCharTypeIs_char())
		{
			sr = convertRegDataToValueDirect_String_NativeType(dwType, spanData, saValue);
			VLR_ON_SR_ERROR_RETURN_VALUE(sr);
		}
		else
		{
			std::wstring sValueNative;
			sr = convertRegDataToValueDirect_String_NativeType(dwType, spanData, sValueNative);
			VLR_ON_SR_ERROR_RETURN_VALUE(sr);
			saValue = util::Convert::ToStdStringA(sValueNative);
		}
	}

	return SResult::Success;
}

SResult CRegistryAccess::convertValueToRegData_String(
	const std::string& saValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	SResult sr;

	if constexpr (ModuleContext::Compilation::DefaultCharTypeIs_char())
	{
		sr = convertValueToRegDataDirect_String_NativeType(saValue, dwType, arrData);
		VLR_ON_SR_ERROR_RETURN_VALUE(sr);
	}
	else
	{
		std::wstring sValueNative = util::Convert::ToStdStringW(saValue);
		sr = convertValueToRegDataDirect_String_NativeType(sValueNative, dwType, arrData);
		VLR_ON_SR_ERROR_RETURN_VALUE(sr);
	}

	return SResult::Success;
}

SResult CRegistryAccess::convertValueToRegData_String(
	const std::string_view& svValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	SResult sr;

	if constexpr (ModuleContext::Compilation::DefaultCharTypeIs_char())
	{
		sr = convertValueToRegDataDirect_String_NativeType(svValue, dwType, arrData);
		VLR_ON_SR_ERROR_RETURN_VALUE(sr);
	}
	else
	{
		std::wstring sValueNative = util::Convert::ToStdStringW(svValue);
		sr = convertValueToRegDataDirect_String_NativeType(sValueNative, dwType, arrData);
		VLR_ON_SR_ERROR_RETURN_VALUE(sr);
	}

	return SResult::Success;
}

SResult CRegistryAccess::convertRegDataToValue_String(
	const DWORD& dwType,
	cpp::span<const BYTE> spanData,
	std::wstring& swValue) const
{
	SResult sr;

	bool bDirectConversion = false;

	switch (dwType)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
		bDirectConversion = true;
		break;

	default:
		return E_UNEXPECTED;
	}

	if (bDirectConversion)
	{
		if constexpr (ModuleContext::Compilation::DefaultCharTypeIs_char())
		{
			std::string sValueNative;
			sr = convertRegDataToValueDirect_String_NativeType(dwType, spanData, sValueNative);
			VLR_ON_SR_ERROR_RETURN_VALUE(sr);
			swValue = util::Convert::ToStdStringW(sValueNative);
		}
		else
		{
			sr = convertRegDataToValueDirect_String_NativeType(dwType, spanData, swValue);
			VLR_ON_SR_ERROR_RETURN_VALUE(sr);
		}
	}

	return SResult::Success;
}

SResult CRegistryAccess::convertValueToRegData_String(
	const std::wstring& swValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	SResult sr;

	if constexpr (ModuleContext::Compilation::DefaultCharTypeIs_char())
	{
		std::string sValueNative = util::Convert::ToStdStringA(swValue);
		sr = convertValueToRegDataDirect_String_NativeType(sValueNative, dwType, arrData);
		VLR_ON_SR_ERROR_RETURN_VALUE(sr);
	}
	else
	{
		sr = convertValueToRegDataDirect_String_NativeType(swValue, dwType, arrData);
		VLR_ON_SR_ERROR_RETURN_VALUE(sr);
	}

	return SResult::Success;
}

SResult CRegistryAccess::convertValueToRegData_String(
	const std::wstring_view& svValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	SResult sr;

	if constexpr (ModuleContext::Compilation::DefaultCharTypeIs_char())
	{
		std::string sValueNative = util::Convert::ToStdStringA(svValue);
		sr = convertValueToRegDataDirect_String_NativeType(sValueNative, dwType, arrData);
		VLR_ON_SR_ERROR_RETURN_VALUE(sr);
	}
	else
	{
		sr = convertValueToRegDataDirect_String_NativeType(svValue, dwType, arrData);
		VLR_ON_SR_ERROR_RETURN_VALUE(sr);
	}

	return SResult::Success;
}

SResult CRegistryAccess::convertRegDataToValueDirect_String_NativeType(
	const DWORD& /*dwType*/,
	cpp::span<const BYTE> spanData,
	std::string& saValue) const
{
	size_t nCountOfCharsInBuffer = spanData.size() / sizeof(char);
	const char* pStringData = reinterpret_cast<const char*>(spanData.data());
	bool bStringIsNullTerminated = (pStringData[nCountOfCharsInBuffer - 1] == _T('\0'));
	size_t nStringLength = bStringIsNullTerminated ? nCountOfCharsInBuffer - 1 : nCountOfCharsInBuffer;
	if (bStringIsNullTerminated)
	{
		// Read as NULL-terminated string
		auto svzValue = zstring_view{ pStringData, nStringLength, zstring_view::StringIsNullTerminated{} };
		saValue = svzValue.toStdString();
	}
	else
	{
		// Read as string_view (not NULL-terminated)
		auto svValue = std::string_view{ pStringData, nStringLength };
		saValue = std::string{ svValue };
	}

	return SResult::Success;
}

SResult CRegistryAccess::convertRegDataToValueDirect_String_NativeType(
	const DWORD& /*dwType*/,
	cpp::span<const BYTE> spanData,
	std::wstring& swValue) const
{
	size_t nCountOfCharsInBuffer = spanData.size() / sizeof(wchar_t);
	const wchar_t* pStringData = reinterpret_cast<const wchar_t*>(spanData.data());
	bool bStringIsNullTerminated = (pStringData[nCountOfCharsInBuffer - 1] == _T('\0'));
	size_t nStringLength = bStringIsNullTerminated ? nCountOfCharsInBuffer - 1 : nCountOfCharsInBuffer;
	if (bStringIsNullTerminated)
	{
		// Read as NULL-terminated string
		auto svzValue = wzstring_view{ pStringData, nStringLength, wzstring_view::StringIsNullTerminated{} };
		swValue = svzValue.toStdString();
	}
	else
	{
		// Read as string_view (not NULL-terminated)
		auto svValue = std::wstring_view{ pStringData, nStringLength };
		swValue = std::wstring{ svValue };
	}

	return SResult::Success;
}

SResult CRegistryAccess::convertValueToRegDataDirect_String_NativeType(
	const std::string& sValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	return convertValueToRegDataDirect_String_NativeType(
		static_cast<const std::string_view&>(sValue),
		dwType,
		arrData);
}

SResult CRegistryAccess::convertValueToRegDataDirect_String_NativeType(
	const std::wstring& sValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	return convertValueToRegDataDirect_String_NativeType(
		static_cast<const std::wstring_view&>(sValue),
		dwType,
		arrData);
}

SResult CRegistryAccess::convertValueToRegDataDirect_String_NativeType(
	const vlr::zstring_view& svzValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	return convertValueToRegDataDirect_String_NativeType(
		static_cast<const std::string_view&>(svzValue),
		dwType,
		arrData);
}

SResult CRegistryAccess::convertValueToRegDataDirect_String_NativeType(
	const vlr::wzstring_view& svzValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	return convertValueToRegDataDirect_String_NativeType(
		static_cast<const std::wstring_view&>(svzValue),
		dwType,
		arrData);
}

SResult CRegistryAccess::convertValueToRegDataDirect_String_NativeType(
	const std::string_view& svValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	dwType = REG_SZ;

	size_t nByteCountData = svValue.size() * sizeof(char);
	arrData.resize(nByteCountData + sizeof(char));
	memcpy_s(arrData.data(), arrData.size(), svValue.data(), nByteCountData);
	// Need to manually terminate string
	reinterpret_cast<char*>(arrData.data())[nByteCountData] = '\0';

	return SResult::Success;
}

SResult CRegistryAccess::convertValueToRegDataDirect_String_NativeType(
	const std::wstring_view& svValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	dwType = REG_SZ;

	size_t nByteCountData = svValue.size() * sizeof(wchar_t);
	arrData.resize(nByteCountData + sizeof(wchar_t));
	memcpy_s(arrData.data(), arrData.size(), svValue.data(), nByteCountData);
	// Need to manually terminate string
	reinterpret_cast<char*>(arrData.data())[nByteCountData] = L'\0';

	return SResult::Success;
}

SResult CRegistryAccess::convertRegDataToValue_DWORD(
	const DWORD& dwType,
	cpp::span<const BYTE> spanData,
	DWORD& dwValue) const
{
	SResult sr;

	bool bDirectConversion = false;

	switch (dwType)
	{
	case REG_DWORD:
		bDirectConversion = true;
		break;

	default:
		return E_UNEXPECTED;
	}

	if (bDirectConversion)
	{
		VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(spanData.size(), == , sizeof(DWORD));
		dwValue = *reinterpret_cast<const DWORD*>(spanData.data());
	}

	return SResult::Success;
}

SResult CRegistryAccess::convertValueToRegData_DWORD(
	const DWORD& dwValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	dwType = REG_DWORD;

	size_t nByteCountData = sizeof(DWORD);
	arrData.resize(nByteCountData);
	memcpy_s(arrData.data(), arrData.size(), &dwValue, nByteCountData);

	return SResult::Success;
}

SResult CRegistryAccess::convertRegDataToValue_QWORD(
	const DWORD& dwType,
	cpp::span<const BYTE> spanData,
	QWORD& qwValue) const
{
	SResult sr;

	bool bDirectConversion = false;

	switch (dwType)
	{
	case REG_QWORD:
		bDirectConversion = true;
		break;

	default:
		return E_UNEXPECTED;
	}

	if (bDirectConversion)
	{
		VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(spanData.size(), == , sizeof(QWORD));
		qwValue = *reinterpret_cast<const QWORD*>(spanData.data());
	}

	return SResult::Success;
}

SResult CRegistryAccess::convertValueToRegData_QWORD(
	const QWORD& qwValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	dwType = REG_QWORD;

	size_t nByteCountData = sizeof(QWORD);
	arrData.resize(nByteCountData);
	memcpy_s(arrData.data(), arrData.size(), &qwValue, nByteCountData);

	return SResult::Success;
}

SResult CRegistryAccess::convertRegDataToValue_MultiSz(
	const DWORD& dwType,
	cpp::span<const BYTE> spanData,
	std::vector<vlr::tstring>& arrValueCollection) const
{
	SResult sr;

	bool bDirectConversion = false;

	switch (dwType)
	{
	case REG_MULTI_SZ:
		bDirectConversion = true;
		break;

	default:
		return E_UNEXPECTED;
	}

	if (bDirectConversion)
	{
		sr = util::data_adaptor::HelperFor_MultiSZ<TCHAR>{}.ToStructuredData(reinterpret_cast<const TCHAR*>(spanData.data()), arrValueCollection);
		VLR_ASSERT_SR_SUCCEEDED_OR_RETURN_SRESULT(sr);
	}

	return SResult::Success;
}

SResult CRegistryAccess::convertValueToRegData_MultiSz(
	const std::vector<vlr::tstring>& arrValueCollection,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	SResult sr;

	dwType = REG_MULTI_SZ;

	sr = util::data_adaptor::HelperFor_MultiSZ<TCHAR>{}.ToMultiSz(arrValueCollection, arrData);
	VLR_ASSERT_SR_SUCCEEDED_OR_RETURN_SRESULT(sr);

	return SResult::Success;
}

SResult CRegistryAccess::convertRegDataToValue_Binary(
	const DWORD& dwType,
	cpp::span<const BYTE> spanData,
	std::vector<BYTE>& arrBinaryData) const
{
	SResult sr;

	bool bDirectConversion = false;

	switch (dwType)
	{
	case REG_BINARY:
		bDirectConversion = true;
		break;

	default:
		return E_UNEXPECTED;
	}

	if (bDirectConversion)
	{
		arrBinaryData = std::vector<BYTE>{ spanData.begin(), spanData.end() };
	}

	return SResult::Success;
}

SResult CRegistryAccess::convertRegDataToValue_Binary_AsFallback(
	const DWORD& /*dwType*/,
	cpp::span<const BYTE> spanData,
	std::vector<BYTE>& arrBinaryData) const
{
	arrBinaryData = std::vector<BYTE>{ spanData.begin(), spanData.end() };

	return SResult::Success;
}

SResult CRegistryAccess::convertValueToRegData_Binary(
	cpp::span<const BYTE> spanData,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	SResult sr;

	dwType = REG_BINARY;
	arrData = std::vector<BYTE>{ spanData.begin(), spanData.end() };

	return SResult::Success;
}

SResult CRegistryAccess::EnumAllValues(
	tzstring_view svzKeyName,
	const OnEnumValueData& fOnEnumValueData) const
{
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(fOnEnumValueData);

	SResult sr;
	LONG lResult{};

	HKEY hKey{};
	sr = openKey(svzKeyName, KEY_READ, hKey);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(hKey);
	auto onDestroy_CloseRegKey = AutoCloseRegKey{ hKey };

	DWORD dwNumValues{};
	DWORD dwMaxValueNameChars{};
	DWORD dwMaxValueDataBytes{};
	lResult = RegQueryInfoKey(
		hKey,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&dwNumValues,
		&dwMaxValueNameChars,
		&dwMaxValueDataBytes,
		NULL,
		NULL);
	VLR_ASSERT_COMPARE_OR_RETURN_HRESULT_LAST_ERROR(lResult, == , ERROR_SUCCESS);
	if (dwNumValues == 0)
	{
		return S_OK;
	}

	std::vector<TCHAR> arrNameData;
	// Note: We need to add one char to buffer size for NULL-terminator
	arrNameData.resize(dwMaxValueNameChars + 1);
	std::vector<BYTE> arrValueData;
	arrValueData.resize(dwMaxValueDataBytes);

	for (DWORD i = 0; i < dwNumValues; ++i)
	{
		DWORD dwValueNameSizeChars = util::range_checked_cast<DWORD>(arrNameData.size());
		DWORD dwValueType{};
		DWORD dwValueDataSizeBytes = util::range_checked_cast<DWORD>(arrValueData.size());

		lResult = RegEnumValue(
			hKey,
			i,
			arrNameData.data(),
			&dwValueNameSizeChars,
			NULL,
			&dwValueType,
			arrValueData.data(),
			&dwValueDataSizeBytes);
		if (lResult == ERROR_NO_MORE_ITEMS)
		{
			return S_OK;
		}
		// Note: There's a possible race condition here, where a value might be written while we're enumerating,
		// and it exceeds the max buffer size. Ignoring this case for now.
		if (lResult != ERROR_SUCCESS)
		{
			return SResult::For_win32_ErrorCode(lResult);
		}

		const auto& oEnumValueData = EnumValueData{}
			.withIndex(i)
			// Note: Cannot ensure this is NULL-terminated
			.withName(vlr::tstring_view{arrNameData.data(), dwValueNameSizeChars})
			.withType(dwValueType)
			.withData(cpp::span<BYTE>{arrValueData.data(), dwValueDataSizeBytes})
			;
		sr = fOnEnumValueData(oEnumValueData);
		// Note: If we fail the callback, we early-abort and return the error code
		if (!sr.isSuccess())
		{
			return sr;
		}
	}

	return SResult::Success;
}

SResult CRegistryAccess::populateValueMapEntryFromEnumValueData(
	const EnumValueData& oEnumValueData,
	ValueMapEntry& oValueMapEntry) const
{
	SResult sr;

	oValueMapEntry.m_dwType = oEnumValueData.m_dwType;
	oValueMapEntry.m_sValueName = cpp::tstring{ oEnumValueData.m_svName };

	switch (oEnumValueData.m_dwType)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
		oValueMapEntry.m_spValue_SZ = cpp::make_shared<cpp::tstring>();
		sr = convertRegDataToValue_String(oEnumValueData.m_dwType, oEnumValueData.m_spanData, *oValueMapEntry.m_spValue_SZ);
		VLR_ASSERT_SR_SUCCEEDED_OR_RETURN_SRESULT(sr);
		break;
	case REG_DWORD:
		oValueMapEntry.m_spValue_DWORD = cpp::make_shared<DWORD>();
		sr = convertRegDataToValue_DWORD(oEnumValueData.m_dwType, oEnumValueData.m_spanData, *oValueMapEntry.m_spValue_DWORD);
		VLR_ASSERT_SR_SUCCEEDED_OR_RETURN_SRESULT(sr);
		break;
	case REG_QWORD:
		oValueMapEntry.m_spValue_QWORD = cpp::make_shared<QWORD>();
		sr = convertRegDataToValue_QWORD(oEnumValueData.m_dwType, oEnumValueData.m_spanData, *oValueMapEntry.m_spValue_QWORD);
		VLR_ASSERT_SR_SUCCEEDED_OR_RETURN_SRESULT(sr);
		break;
	case REG_MULTI_SZ:
		oValueMapEntry.m_spValue_MultiSZ = cpp::make_shared<std::vector<cpp::tstring>>();
		sr = convertRegDataToValue_MultiSz(oEnumValueData.m_dwType, oEnumValueData.m_spanData, *oValueMapEntry.m_spValue_MultiSZ);
		VLR_ASSERT_SR_SUCCEEDED_OR_RETURN_SRESULT(sr);
		break;
	case REG_BINARY:
		oValueMapEntry.m_spValue_Binary = cpp::make_shared<std::vector<BYTE>>();
		sr = convertRegDataToValue_Binary(oEnumValueData.m_dwType, oEnumValueData.m_spanData, *oValueMapEntry.m_spValue_Binary);
		VLR_ASSERT_SR_SUCCEEDED_OR_RETURN_SRESULT(sr);
		break;

	default:
		oValueMapEntry.m_spValue_TypeUnhandled = cpp::make_shared<std::vector<BYTE>>();
		sr = convertRegDataToValue_Binary_AsFallback(oEnumValueData.m_dwType, oEnumValueData.m_spanData, *oValueMapEntry.m_spValue_TypeUnhandled);
		VLR_ASSERT_SR_SUCCEEDED_OR_RETURN_SRESULT(sr);
		break;
	}

	return SResult::Success;
}

SResult CRegistryAccess::RealAllValuesIntoMap(
	tzstring_view svzKeyName,
	std::unordered_map<vlr::tstring, ValueMapEntry>& mapNameToValue) const
{
	SResult sr;

	auto fOnEnumValueData_AddToMap = [&](const CRegistryAccess::EnumValueData& oEnumValueData) -> SResult
	{
		auto& oValueMapEntry = mapNameToValue[vlr::tstring{oEnumValueData.m_svName}];
		sr = populateValueMapEntryFromEnumValueData(oEnumValueData, oValueMapEntry);
		VLR_ASSERT_SR_SUCCEEDED_OR_RETURN_SRESULT(sr);

		return SResult::Success;
	};

	sr = EnumAllValues(svzKeyName, fOnEnumValueData_AddToMap);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::ReadValueObfuscated(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	ValueMapEntry& oValueMapEntry)
{
	SResult sr;

	auto fValueMatchesForRead = [&](const CRegistryAccess::EnumValueData& oEnumValueData) -> SResult
	{
		if (StringCompare::CS().AreEqual(oEnumValueData.m_svName, svzValueName))
		{
			return SResult::Success;
		}

		return SResult::Success_WithNuance;
	};

	bool bFoundValue = false;
	auto fOnEnumValueData_PopulateOnMatch = [&](const CRegistryAccess::EnumValueData& oEnumValueData) -> SResult
	{
		sr = fValueMatchesForRead(oEnumValueData);
		if (sr != SResult::Success)
		{
			return SResult::Success_NoWorkDone;
		}

		bFoundValue = true;

		sr = populateValueMapEntryFromEnumValueData(oEnumValueData, oValueMapEntry);
		VLR_ASSERT_SR_SUCCEEDED_OR_RETURN_SRESULT(sr);

		return SResult::Success;
	};

	sr = EnumAllValues(svzKeyName, fOnEnumValueData_PopulateOnMatch);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	if (!bFoundValue)
	{
		return SResult::Success_WithNuance;
	}

	return SResult::Success;
}

SResult CRegistryAccess::ReadValuesObfuscated(
	tzstring_view svzKeyName,
	const std::vector<cpp::tstring>& arrValueNames,
	std::vector<ValueMapEntry>& arrValueMapEntryCollection)
{
	SResult sr;

	auto fValueMatchesForRead = [&](const CRegistryAccess::EnumValueData& oEnumValueData) -> SResult
	{
		for (const auto& sValueName : arrValueNames)
		{
			if (StringCompare::CS().AreEqual(oEnumValueData.m_svName, sValueName))
			{
				return SResult::Success;
			}
		}

		return SResult::Success_WithNuance;
	};

	auto fOnEnumValueData_PopulateOnMatch = [&](const CRegistryAccess::EnumValueData& oEnumValueData) -> SResult
	{
		sr = fValueMatchesForRead(oEnumValueData);
		if (sr != SResult::Success)
		{
			return SResult::Success_NoWorkDone;
		}

		auto& oValueMapEntry = arrValueMapEntryCollection.emplace_back();

		sr = populateValueMapEntryFromEnumValueData(oEnumValueData, oValueMapEntry);
		VLR_ASSERT_SR_SUCCEEDED_OR_RETURN_SRESULT(sr);

		return SResult::Success;
	};

	sr = EnumAllValues(svzKeyName, fOnEnumValueData_PopulateOnMatch);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::EnumAllSubkeys(
	tzstring_view svzKeyName,
	const OnEnumSubkeyData& fOnEnumSubkeyData) const
{
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(fOnEnumSubkeyData);

	SResult sr;
	LONG lResult{};

	HKEY hKey{};
	sr = openKey(svzKeyName, KEY_READ, hKey);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(hKey);
	auto onDestroy_CloseRegKey = AutoCloseRegKey{ hKey };

	DWORD dwSubkeyCount{};
	DWORD dwMaxSubkeyNameChars{};
	DWORD dwMaxSubkeyClassChars{};
	lResult = RegQueryInfoKey(
		hKey,
		NULL,
		NULL,
		NULL,
		&dwSubkeyCount,
		&dwMaxSubkeyNameChars,
		&dwMaxSubkeyClassChars,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);
	VLR_ASSERT_COMPARE_OR_RETURN_HRESULT_LAST_ERROR(lResult, == , ERROR_SUCCESS);
	if (dwSubkeyCount == 0)
	{
		return S_OK;
	}

	std::vector<TCHAR> arrSubkeyNameData;
	// Note: We need to add one char to buffer size for NULL-terminator
	arrSubkeyNameData.resize(dwMaxSubkeyNameChars + 1);
	std::vector<TCHAR> arrSubkeyClassData;
	arrSubkeyClassData.resize(dwMaxSubkeyClassChars + 1);

	for (DWORD i = 0; i < dwSubkeyCount; ++i)
	{
		EnumSubkeyData oEnumSubkeyData{};
		oEnumSubkeyData.withIndex(i);

		DWORD dwSubkeyNameSizeChars = util::range_checked_cast<DWORD>(arrSubkeyNameData.size());
		DWORD dwSubkeyClassSizeChars = util::range_checked_cast<DWORD>(arrSubkeyClassData.size());

		lResult = RegEnumKeyEx(
			hKey,
			i,
			arrSubkeyNameData.data(),
			&dwSubkeyNameSizeChars,
			NULL,
			arrSubkeyClassData.data(),
			&dwSubkeyClassSizeChars,
			&oEnumSubkeyData.m_ftLastWriteTime);
		if (lResult == ERROR_NO_MORE_ITEMS)
		{
			return S_OK;
		}
		// Note: There's a possible race condition here, where a value might be written while we're enumerating,
		// and it exceeds the max buffer size. Ignoring this case for now.
		if (lResult != ERROR_SUCCESS)
		{
			return SResult::For_win32_ErrorCode(lResult);
		}

		oEnumSubkeyData.withName(vlr::tstring_view{arrSubkeyNameData.data(), dwSubkeyNameSizeChars});
		oEnumSubkeyData.withClass(vlr::tstring_view{arrSubkeyClassData.data(), dwSubkeyClassSizeChars});

		sr = fOnEnumSubkeyData(oEnumSubkeyData);
		// Note: If we fail the callback, we early-abort and return the error code
		if (!sr.isSuccess())
		{
			return sr;
		}
	}

	return SResult::Success;
}

SResult CRegistryAccess::ReadAllSubkeysIntoVector(
	tzstring_view svzKeyName,
	std::vector<cpp::tstring>& arrSubkeyNames)
{
	SResult sr;

	auto fOnEnumSubkeyData_AddToResult = [&](const EnumSubkeyData& oEnumSubkeyData) -> SResult
	{
		arrSubkeyNames.emplace_back(oEnumSubkeyData.m_svName);

		return SResult::Success;
	};

	sr = EnumAllSubkeys(svzKeyName, fOnEnumSubkeyData_AddToResult);
	VLR_ON_SR_ERROR_RETURN_VALUE(sr);

	return SResult::Success;
}

SResult CRegistryAccess::openKey(
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

DWORD CRegistryAccess::getWow64RedirectionKeyAccessMask() const
{
	switch (m_eWow64KeyAccessOption)
	{
	default:
		VLR_ASSERT_ON_UNHANDLED_SWITCH_CASE;
	case RegistryAccess::Wow64KeyAccessOption::Unknown:
	case RegistryAccess::Wow64KeyAccessOption::UseDefault:
		return 0;
	case RegistryAccess::Wow64KeyAccessOption::UseExplicit32bit:
		return KEY_WOW64_32KEY;
	case RegistryAccess::Wow64KeyAccessOption::UseExplicit64bit:
		return KEY_WOW64_64KEY;
	case RegistryAccess::Wow64KeyAccessOption::UsePlatformNative:
		if (vlr::ModuleContext::Runtime::NativePlatformIs_64bit())
		{
			return KEY_WOW64_64KEY;
		}
		else
		{
			return 0;
		}
	}
}

} // namespace win32

} // namespace vlr
