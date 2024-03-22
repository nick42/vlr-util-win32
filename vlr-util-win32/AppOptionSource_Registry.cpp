#include "pch.h"
#include "AppOptionSource_Registry.h"

namespace vlr {

namespace win32 {

SResult CAppOptionSource_Registry::ReadAllValuesFromPathAsOptions(
	const CRegistryAccess& oReg,
	vlr::tzstring_view svzPath)
{
	SResult sr;

	auto& oAppOptions = GetAppOptions();

	std::unordered_map<vlr::tstring, CRegistryAccess::ValueMapEntry> mapNameToValue;
	sr = oReg.RealAllValuesIntoMap(svzPath, mapNameToValue);
	if (sr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		// This means the key doesn't exist; treating this as a normal NOOP result
		return S_FALSE;
	}
	VLR_ASSERT_SR_SUCCEEDED_OR_RETURN_SRESULT(sr);

	for (const auto& oMapPair : mapNameToValue)
	{
		const auto& oValueMapEntry = oMapPair.second;
		const auto& sNativeOptionName = oValueMapEntry.m_sValueName;

		SPCAppOptionSpecifiedValue spSpecifiedValue;
		auto fMakeAppOptionSpecifiedValue = [&](const auto& tValue)
		{
			spSpecifiedValue = std::make_shared<CAppOptionSpecifiedValue>(
				AppOptionSource::SystemConfigRespository,
				sNativeOptionName,
				tValue);
		};

		if (oValueMapEntry.m_spValue_SZ)
		{
			fMakeAppOptionSpecifiedValue(*oValueMapEntry.m_spValue_SZ);
		}
		else if (oValueMapEntry.m_spValue_DWORD)
		{
			fMakeAppOptionSpecifiedValue(static_cast<uint32_t>(*oValueMapEntry.m_spValue_DWORD));
		}
		else if (oValueMapEntry.m_spValue_QWORD)
		{
			fMakeAppOptionSpecifiedValue(static_cast<uint64_t>(*oValueMapEntry.m_spValue_QWORD));
		}
		else
		{
			// Not supported atm; skip
		}

		if (!spSpecifiedValue)
		{
			continue;
		}

		sr = oAppOptions.AddSpecifiedValue(spSpecifiedValue);
		VLR_ASSERT_SR_SUCCEEDED_OR_CONTINUE(sr);
	}

	return S_OK;
}

} // namespace win32

} // namespace vlr
