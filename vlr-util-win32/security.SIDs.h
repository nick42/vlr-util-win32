#pragma once

#include <memory>
#include <optional>
#include <map>
#include <shared_mutex>

#include <vlr-util/util.includes.h>
#include <vlr-util/zstring_view.h>

namespace vlr {

namespace win32 {

namespace security {

namespace SIDs {

class CSidNameLookupResult;
using SPCSidNameLookupResult = std::shared_ptr<CSidNameLookupResult>;

class CSidInfo
{
public:
	PSID m_pSid = nullptr;
	std::optional<vlr::tstring> m_osStringSid;
	SPCSidNameLookupResult m_spSidNameLookupResult;

public:
	inline decltype(auto) withPSID(PSID pSid)
	{
		m_pSid = pSid;
		m_osStringSid = {};
		return *this;
	}
	inline decltype(auto) withStringSid(const vlr::tstring& sStringSid)
	{
		m_osStringSid = sStringSid;
		return *this;
	}
	inline decltype(auto) withSidNameLookupResult(const SPCSidNameLookupResult& spSidNameLookupResult)
	{
		m_spSidNameLookupResult = spSidNameLookupResult;
		return *this;
	}

public:
	HRESULT PopulateData_StringSid();
	HRESULT PopulateData_SidNameLookupResult();

	HRESULT PopulateStringSid(std::optional<vlr::tstring>& osStringSid) const;
	vlr::tstring GetStringSid();
	vlr::tstring GetStringSid() const;

	vlr::tstring GetDisplay_Default() const;
	std::optional<vlr::tstring> GetDisplay_LogicalAccountName_Default() const;

public:
	CSidInfo() = default;
	CSidInfo(PSID pSid)
		: m_pSid{ pSid }
	{
	}
};

class CSidNameLookupResult
{
public:
	std::atomic<bool> m_bIsLookupResultComplete{ false };
	std::thread::id m_oOwningLookupThreadID{};
	// Note: If this is a failure code, then we failed the lookup processing unexpectedly
	SResult m_srGeneralProcessingResult;

	std::optional<vlr::tstring> m_osLookupSystemName;
	std::optional<DWORD> m_odwLookupError;

	vlr::tstring m_sStringSid;

	std::optional<WELL_KNOWN_SID_TYPE> m_oeWellKnownSid;

	vlr::tstring m_sAccountName;
	vlr::tstring m_sReferencedDomainName;
	SID_NAME_USE m_eUse = SidTypeUnknown;
};

class CSidNameLookupCache
{
protected:
	std::map<vlr::tstring, SPCSidNameLookupResult> m_oStringSidToLookupResultMap;
	mutable std::shared_mutex m_oAccessSync_StringSidToLookupResultMap;

	//mutable std::mutex m_oWaitEvent_LookuResultChanged;

public:
	HRESULT GetLookupResult(
		const vlr::tstring& sStringSid,
		SPCSidNameLookupResult& spSidNameLookupResult_Result);
	HRESULT SetLookupResult(
		const vlr::tstring& sStringSid,
		const SPCSidNameLookupResult& spSidNameLookupResult);
	// Note: This does two possible things:
	// - If the lookup is already created in the cache, returns the value
	// - If the lookup has not been done before, returns a new instance, with the "lookup complete" flag set to false
	HRESULT OnLookup_PopulateLookupResult(
		const vlr::tstring& sStringSid,
		SPCSidNameLookupResult& spSidNameLookupResult);

	HRESULT PopulateCache_WellKnownSids();

public:
	static CSidNameLookupCache& GetSharedInstance();

};

HRESULT DoConvertSidToStringSid(
	PSID pSid,
	vlr::tstring& sStringSid);

HRESULT DoLookupAccountSid(
	LPCTSTR pcszLookupSystemName,
	const CSidInfo& oSidInfo,
	SPCSidNameLookupResult& spSidNameLookupResult_Result);

} // namespace SIDs

} // namespace security

} // namespace win32

} // namespace vlr
