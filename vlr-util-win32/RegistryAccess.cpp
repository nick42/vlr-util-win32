#include "pch.h"
#include "RegistryAccess.h"

#include "vlr-util/StringCompare.h"
#include "vlr-util/util.range_checked_cast.h"
#include "vlr-util/util.convert.StringConversion.h"

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

SResult RegistryAccess::ReadValueBase(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	DWORD& dwType_Result,
	std::vector<BYTE>& arrData) const
{
	SResult sr;
	LONG lResult{};

	HKEY hKey{};
	sr = openKey(svzKeyName, KEY_READ, hKey);
	if (!sr.isSuccess())
	{
		return SResult::Success_WithNuance;
	}
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

SResult RegistryAccess::WriteValueBase(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	const DWORD& dwType,
	const std::vector<BYTE>& arrData) const
{
	SResult sr;
	LONG lResult{};

	HKEY hKey{};
	sr = openKey(svzKeyName, KEY_WRITE, hKey);
	if (!sr.isSuccess())
	{
		return SResult::Success_WithNuance;
	}
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(hKey);
	auto onDestroy_CloseRegKey = AutoCloseRegKey{ hKey };

	size_t nIterationCount = 0;
	while (true)
	{
		nIterationCount++;

		DWORD dwBufferSize = util::range_checked_cast<DWORD>(arrData.size());
		lResult = RegSetValueEx(
			hKey,
			svzValueName,
			NULL,
			dwType,
			arrData.data(),
			util::range_checked_cast<DWORD>(arrData.size()));
		if (lResult == ERROR_SUCCESS)
		{
			break;
		}

		// We got an unhandled/unexpected error
		return __HRESULT_FROM_WIN32(lResult);
	}

	return S_OK;
}

SResult RegistryAccess::ReadValue_String(
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

SResult RegistryAccess::ReadValue_String(
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

SResult RegistryAccess::WriteValue_String(
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

SResult RegistryAccess::ReadValue_String(
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

SResult RegistryAccess::ReadValue_String(
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

SResult RegistryAccess::WriteValue_String(
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

SResult RegistryAccess::ReadValue_DWORD(
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

SResult RegistryAccess::ReadValue_DWORD(
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

SResult RegistryAccess::WriteValue_DWORD(
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

SResult RegistryAccess::ReadValue_QWORD(
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

SResult RegistryAccess::ReadValue_QWORD(
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

SResult RegistryAccess::WriteValue_QWORD(
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

SResult RegistryAccess::ReadValue_MultiSz(
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

SResult RegistryAccess::ReadValue_MultiSz(
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

SResult RegistryAccess::WriteValue_MultiSz(
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

SResult RegistryAccess::ReadValue_Binary(
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

SResult RegistryAccess::ReadValue_Binary(
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

SResult RegistryAccess::WriteValue_Binary(
	tzstring_view svzKeyName,
	tzstring_view svzValueName,
	const std::vector<BYTE>& arrBinaryData) const
{
	SResult sr;

	DWORD dwType{};
	std::vector<BYTE> arrData{};
	sr = convertValueToRegData_Binary(
		arrBinaryData,
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

SResult RegistryAccess::convertRegDataToValue_String(
	const DWORD& dwType,
	const std::vector<BYTE>& arrData,
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
			sr = convertRegDataToValueDirect_String_NativeType(dwType, arrData, saValue);
			VLR_ON_SR_ERROR_RETURN_VALUE(sr);
		}
		else
		{
			std::wstring sValueNative;
			sr = convertRegDataToValueDirect_String_NativeType(dwType, arrData, sValueNative);
			VLR_ON_SR_ERROR_RETURN_VALUE(sr);
			saValue = util::Convert::ToStdStringA(sValueNative);
		}
	}

	return SResult::Success;
}

SResult RegistryAccess::convertValueToRegData_String(
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

SResult RegistryAccess::convertRegDataToValue_String(
	const DWORD& dwType,
	const std::vector<BYTE>& arrData,
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
			sr = convertRegDataToValueDirect_String_NativeType(dwType, arrData, sValueNative);
			VLR_ON_SR_ERROR_RETURN_VALUE(sr);
			swValue = util::Convert::ToStdStringW(sValueNative);
		}
		else
		{
			sr = convertRegDataToValueDirect_String_NativeType(dwType, arrData, swValue);
			VLR_ON_SR_ERROR_RETURN_VALUE(sr);
		}
	}

	return SResult::Success;
}

SResult RegistryAccess::convertValueToRegData_String(
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

SResult RegistryAccess::convertRegDataToValueDirect_String_NativeType(
	const DWORD& dwType,
	const std::vector<BYTE>& arrData,
	std::string& saValue) const
{
	size_t nCountOfCharsInBuffer = arrData.size() / sizeof(char);
	const char* pStringData = reinterpret_cast<const char*>(arrData.data());
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

SResult RegistryAccess::convertRegDataToValueDirect_String_NativeType(
	const DWORD& dwType,
	const std::vector<BYTE>& arrData,
	std::wstring& swValue) const
{
	size_t nCountOfCharsInBuffer = arrData.size() / sizeof(wchar_t);
	const wchar_t* pStringData = reinterpret_cast<const wchar_t*>(arrData.data());
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

SResult RegistryAccess::convertValueToRegDataDirect_String_NativeType(
	const std::string& sValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	dwType = REG_SZ;

	size_t nByteCountData = (sValue.length() + 1) * sizeof(char);
	arrData.resize(nByteCountData);
	// Note: This is safe, because the NULL-terminator is in the memory block
	memcpy_s(arrData.data(), arrData.size(), sValue.c_str(), nByteCountData);

	return SResult::Success;
}

SResult RegistryAccess::convertValueToRegDataDirect_String_NativeType(
	const std::wstring& sValue,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	dwType = REG_SZ;

	size_t nByteCountData = (sValue.length() + 1) * sizeof(wchar_t);
	arrData.resize(nByteCountData);
	// Note: This is safe, because the NULL-terminator is in the memory block
	memcpy_s(arrData.data(), arrData.size(), sValue.c_str(), nByteCountData);

	return SResult::Success;
}

SResult RegistryAccess::convertRegDataToValue_DWORD(
	const DWORD& dwType,
	const std::vector<BYTE>& arrData,
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
		VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(arrData.size(), == , sizeof(DWORD));
		dwValue = *reinterpret_cast<const DWORD*>(arrData.data());
	}

	return SResult::Success;
}

SResult RegistryAccess::convertValueToRegData_DWORD(
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

SResult RegistryAccess::convertRegDataToValue_QWORD(
	const DWORD& dwType,
	const std::vector<BYTE>& arrData,
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
		VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(arrData.size(), == , sizeof(QWORD));
		qwValue = *reinterpret_cast<const QWORD*>(arrData.data());
	}

	return SResult::Success;
}

SResult RegistryAccess::convertValueToRegData_QWORD(
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

SResult RegistryAccess::convertRegDataToValue_MultiSz(
	const DWORD& dwType,
	const std::vector<BYTE>& arrData,
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
		sr = util::data_adaptor::HelperFor_MultiSZ<TCHAR>{}.ToStructuredData(reinterpret_cast<const TCHAR*>(arrData.data()), arrValueCollection);
		VLR_ASSERT_SR_SUCCEEDED_OR_RETURN_SRESULT(sr);
	}

	return SResult::Success;
}

SResult RegistryAccess::convertValueToRegData_MultiSz(
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

SResult RegistryAccess::convertRegDataToValue_Binary(
	const DWORD& dwType,
	const std::vector<BYTE>& arrData,
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
		arrBinaryData = arrData;
	}

	return SResult::Success;
}

SResult RegistryAccess::convertValueToRegData_Binary(
	const std::vector<BYTE>& arrBinaryData,
	DWORD& dwType,
	std::vector<BYTE>& arrData) const
{
	SResult sr;

	dwType = REG_MULTI_SZ;

	arrData = arrBinaryData;

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
