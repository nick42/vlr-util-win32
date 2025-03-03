#include "pch.h"
#include "security.SIDs.h"

#include <sddl.h>

#include <vlr-util/util.includes.h>
#include <vlr-util/AutoFreeResource.h>
#include <vlr-util/formatpf.h>

#include <vlr-util-win32/enum.WELL_KNOWN_SID_TYPE.h>
#include <vlr-util-win32/enum.SID_NAME_USE.h>

namespace vlr {

namespace win32 {

namespace security {

namespace SIDs {

HRESULT CSidInfo::PopulateData_StringSid()
{
	return PopulateStringSid(m_osStringSid);
}

HRESULT CSidInfo::PopulateData_SidNameLookupResult()
{
	return DoLookupAccountSid(nullptr, *this, m_spSidNameLookupResult);
}

HRESULT CSidInfo::PopulateStringSid(std::optional<vlr::tstring>& osStringSid) const
{
	HRESULT hr;

	vlr::tstring sStringSid;
	hr = DoConvertSidToStringSid(m_pSid, sStringSid);
	VLR_ON_HR_NON_S_OK__RETURN_HRESULT(hr);

	osStringSid = sStringSid;

	return S_OK;
}

vlr::tstring CSidInfo::GetStringSid()
{
	if (m_osStringSid.has_value())
	{
		return m_osStringSid.value();
	}
	if (m_pSid == nullptr)
	{
		return _T("[null]");
	}

	PopulateStringSid(m_osStringSid);

	return m_osStringSid.value_or(_T(""));
}

vlr::tstring CSidInfo::GetStringSid() const
{
	if (m_osStringSid.has_value())
	{
		return m_osStringSid.value();
	}
	if (m_pSid == nullptr)
	{
		return _T("[null]");
	}

	std::optional<vlr::tstring> osStringSid;
	PopulateStringSid(osStringSid);

	return osStringSid.value_or(_T(""));
}

vlr::tstring CSidInfo::GetDisplay_Default() const
{
	vlr::tstring sAccountNamePostfix;
	auto osLogicalAccountName = GetDisplay_LogicalAccountName_Default();
	if (osLogicalAccountName.has_value())
	{
		sAccountNamePostfix = vlr::formatpf(_T(" (%s)"), osLogicalAccountName.value());
	}

	return vlr::formatpf(_T("%s%s"), GetStringSid(), sAccountNamePostfix);
}

std::optional<vlr::tstring> CSidInfo::GetDisplay_LogicalAccountName_Default() const
{
	if (!m_spSidNameLookupResult)
	{
		return {};
	}

	if (m_spSidNameLookupResult->m_oeWellKnownSid.has_value())
	{
		return vlr::formatpf(_T("well-known: %s"),
			enums::CFormatEnum<WELL_KNOWN_SID_TYPE>::FormatValue(m_spSidNameLookupResult->m_oeWellKnownSid.value()));
	}

	// TODO: More here...

	return vlr::formatpf(_T("[%s] %s\\%s"),
		enums::CFormatEnum<SID_NAME_USE>::FormatValue(m_spSidNameLookupResult->m_eUse),
		m_spSidNameLookupResult->m_sReferencedDomainName,
		m_spSidNameLookupResult->m_sAccountName);
}

HRESULT CSidNameLookupCache::GetLookupResult(
	const vlr::tstring& sStringSid,
	SPCSidNameLookupResult& spSidNameLookupResult_Result)
{
	const auto oLockForRead = std::shared_lock{ m_oAccessSync_StringSidToLookupResultMap };

	auto iterMapIndex = m_oStringSidToLookupResultMap.find(sStringSid);
	if (iterMapIndex == m_oStringSidToLookupResultMap.end())
	{
		return S_FALSE;
	}
	const auto& spSidNameLookupResult = iterMapIndex->second;
	if (!spSidNameLookupResult->m_srGeneralProcessingResult.isSuccess())
	{
		return S_FALSE;
	}
	if (!spSidNameLookupResult->m_bIsLookupResultComplete)
	{
		// Lookup in progress for this SID; don't return it
		return S_FALSE;
	}

	spSidNameLookupResult_Result = spSidNameLookupResult;

	return S_OK;
}

HRESULT CSidNameLookupCache::SetLookupResult(
	const vlr::tstring& sStringSid,
	const SPCSidNameLookupResult& spSidNameLookupResult)
{
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(spSidNameLookupResult);

	const auto oLock = std::lock_guard{ m_oAccessSync_StringSidToLookupResultMap };

	m_oStringSidToLookupResultMap[sStringSid] = spSidNameLookupResult;

	return S_OK;
}

HRESULT CSidNameLookupCache::OnLookup_PopulateLookupResult(
	const vlr::tstring& sStringSid,
	SPCSidNameLookupResult& spSidNameLookupResult)
{
	const auto oLock = std::lock_guard{ m_oAccessSync_StringSidToLookupResultMap };

	auto iterMapIndex = m_oStringSidToLookupResultMap.find(sStringSid);
	if (iterMapIndex != m_oStringSidToLookupResultMap.end())
	{
		spSidNameLookupResult = iterMapIndex->second;
		return S_OK;
	}

	spSidNameLookupResult = std::make_shared<CSidNameLookupResult>();
	VLR_ASSERT_ALLOCATED_OR_RETURN_STANDARD_ERROR(spSidNameLookupResult);

	spSidNameLookupResult->m_sStringSid = sStringSid;
	spSidNameLookupResult->m_oOwningLookupThreadID = std::this_thread::get_id();
	// Note: Default struct init has "complete" flag set to false, so we're returning a lookup "in progress"

	m_oStringSidToLookupResultMap[sStringSid] = spSidNameLookupResult;

	return S_OK;
}

HRESULT CSidNameLookupCache::PopulateCache_WellKnownSids()
{
	const auto oLock = std::lock_guard{ m_oAccessSync_StringSidToLookupResultMap };

	HRESULT hr;
	BOOL bSuccess;

	for (UINT nEnumValue = WinNullSid; nEnumValue <= WinBuiltinDeviceOwnersSid; ++nEnumValue)
	{
		auto eWellKnownSid = static_cast<WELL_KNOWN_SID_TYPE>(nEnumValue);

		bool bSidCreateValid = true;
		std::vector<BYTE> oSidDataArray;
		DWORD dwBufferLength = 1024;
		oSidDataArray.resize(1024);
		do
		{
			bSuccess = ::CreateWellKnownSid(
				eWellKnownSid,
				NULL,
				oSidDataArray.data(),
				&dwBufferLength);
			if (bSuccess)
			{
				break;
			}
			auto dwLastError = ::GetLastError();
			if (dwLastError == ERROR_INVALID_PARAMETER)
			{
				bSidCreateValid = false;
				break;
			}
			if (dwLastError == ERROR_INSUFFICIENT_BUFFER)
			{
				VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(dwBufferLength, > , oSidDataArray.size());
				oSidDataArray.resize(dwBufferLength);
				continue;
			}

			VLR_HANDLE_ASSERTION_FAILURE__AND_RETURN_EXPRESSION(HRESULT_FROM_WIN32(dwLastError));
		} while (true);
		if (!bSidCreateValid)
		{
			// This well-known Sid is not valid in current context
			continue;
		}

		vlr::tstring sStringSid;
		hr = DoConvertSidToStringSid(oSidDataArray.data(), sStringSid);
		VLR_ASSERT_HR_SUCCEEDED_OR_RETURN_HRESULT(hr);

		auto spSidNameLookupResult = std::make_shared<CSidNameLookupResult>();
		VLR_ASSERT_ALLOCATED_OR_RETURN_STANDARD_ERROR(spSidNameLookupResult);
		spSidNameLookupResult->m_sStringSid = sStringSid;
		spSidNameLookupResult->m_oeWellKnownSid = eWellKnownSid;

		spSidNameLookupResult->m_bIsLookupResultComplete = true;

		m_oStringSidToLookupResultMap[sStringSid] = spSidNameLookupResult;
	}

	return S_OK;
}

CSidNameLookupCache& CSidNameLookupCache::GetSharedInstance()
{
	static auto oInstance = CSidNameLookupCache{};

	static const auto bHaveRun_PopulateCache_WellKnownSids = [&]
	{
		oInstance.PopulateCache_WellKnownSids();
		return true;
	}();

	return oInstance;
}

HRESULT DoConvertSidToStringSid(
	PSID pSid,
	vlr::tstring& sStringSid)
{
	static const auto _tFailureValue = E_FAIL;

	// Convert binary SID to string SID

	LPTSTR pszStringSid = nullptr;
	auto bSuccess = ::ConvertSidToStringSid(
		pSid,
		&pszStringSid);
	VLR_ASSERT_NONZERO_OR_RETURN_FAILURE_VALUE(bSuccess);
	VLR_ASSERT_NONZERO_OR_RETURN_FAILURE_VALUE(pszStringSid);
	auto oOnDestroy_FreeStringSid = MakeAutoCleanup_viaLocalFree(pszStringSid);

	sStringSid = vlr::tstring{ pszStringSid };

	return S_OK;
}

HRESULT DoLookupAccountSid(
	LPCTSTR pcszLookupSystemName,
	const CSidInfo& oSidInfo,
	SPCSidNameLookupResult& spSidNameLookupResult_Result)
{
	HRESULT hr;

	auto& oSidNameLookupCache = CSidNameLookupCache::GetSharedInstance();

	SPCSidNameLookupResult spSidNameLookupResult;
	hr = oSidNameLookupCache.OnLookup_PopulateLookupResult(
		oSidInfo.GetStringSid(),
		spSidNameLookupResult);
	VLR_ASSERT_SUCCEEDED_OR_RETURN_RESULT(hr);
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(spSidNameLookupResult);

	// We're going to always set the result value, even if we return an error
	//auto oOnDestroy_AssignResult = MakeActionOnDestruction([&] { spSidNameLookupResult_Result = spSidNameLookupResult; });
	spSidNameLookupResult_Result = spSidNameLookupResult;

	// Check to see if we unexpectedly failed internally, and if so, return that value immediately (no retry)
	if (!spSidNameLookupResult->m_srGeneralProcessingResult.isSuccess())
	{
		return spSidNameLookupResult->m_srGeneralProcessingResult;
	}

	// If spSidNameLookupResult->m_bIsLookupResultComplete is false, then another thread may be doing the lookup, 
	// and we should wait for that thread if necessary.
	if (spSidNameLookupResult->m_bIsLookupResultComplete)
	{
		return S_OK;
	}
	if (spSidNameLookupResult->m_oOwningLookupThreadID != std::this_thread::get_id())
	{
		// Lookups may take some time; eg: this may need to contact a DC to resolve a name.
		// 30 seconds current timeout
		static constexpr ULONGLONG nTickCountTimout = 1000 * 30;

		auto nTickCountStart = GetTickCount64();
		while (GetTickCount64() - nTickCountStart < nTickCountTimout)
		{
			Sleep(100);
			if (spSidNameLookupResult->m_bIsLookupResultComplete)
			{
				return S_OK;
			}
		}

		spSidNameLookupResult->m_srGeneralProcessingResult = __HRESULT_FROM_WIN32(ERROR_TIMEOUT);
		return spSidNameLookupResult->m_srGeneralProcessingResult;
	}
	// Note: If we get here, we own the thread to do the lookup

	// On function exit, set the lookup as complete. We need to ensure that we set a general result code in all success cases.
	// The default will exist here if we early-abort the lookup.
	spSidNameLookupResult->m_srGeneralProcessingResult = E_UNEXPECTED;
	auto oOnDestroy_SetLookupComplete = MakeActionOnDestruction([&]()
	{
		spSidNameLookupResult->m_bIsLookupResultComplete = true;
	});

	BOOL bSuccess;

	DWORD dwBufferLen_AccountName = 0;
	DWORD dwBufferLen_ReferencedDomainName = 0;

	bSuccess = ::LookupAccountSid(
		pcszLookupSystemName,
		oSidInfo.m_pSid,
		nullptr,
		&dwBufferLen_AccountName,
		nullptr,
		&dwBufferLen_ReferencedDomainName,
		&spSidNameLookupResult->m_eUse);
	VLR_ASSERT_COMPARE_OR_RETURN_EUNEXPECTED(bSuccess, == , FALSE);
	auto dwLastError = ::GetLastError();
	if (dwLastError != ERROR_INSUFFICIENT_BUFFER)
	{
		spSidNameLookupResult->m_srGeneralProcessingResult = S_OK;
		spSidNameLookupResult->m_odwLookupError = dwLastError;
		return S_FALSE;
	}

	spSidNameLookupResult->m_sAccountName.resize(dwBufferLen_AccountName);
	spSidNameLookupResult->m_sReferencedDomainName.resize(dwBufferLen_ReferencedDomainName);
	bSuccess = ::LookupAccountSid(
		pcszLookupSystemName,
		oSidInfo.m_pSid,
		spSidNameLookupResult->m_sAccountName.data(),
		&dwBufferLen_AccountName,
		spSidNameLookupResult->m_sReferencedDomainName.data(),
		&dwBufferLen_ReferencedDomainName,
		&spSidNameLookupResult->m_eUse);
	if (!bSuccess)
	{
		spSidNameLookupResult->m_srGeneralProcessingResult = S_OK;
		spSidNameLookupResult->m_odwLookupError = ::GetLastError();
		return HRESULT_FROM_NT(spSidNameLookupResult->m_odwLookupError.value());
	}

	spSidNameLookupResult->m_sAccountName.resize(spSidNameLookupResult->m_sAccountName.size() - 1);
	spSidNameLookupResult->m_sReferencedDomainName.resize(spSidNameLookupResult->m_sReferencedDomainName.size() - 1);

	spSidNameLookupResult->m_srGeneralProcessingResult = S_OK;

	return S_OK;
}

} // namespace SIDs

} // namespace security

} // namespace win32

} // namespace vlr
