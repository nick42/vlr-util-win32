#include "pch.h"
#include "filesystem.Functions.h"

#include <vlr-util/util.data_adaptor.MultiSZ.h>

namespace vlr {

namespace win32 {

namespace filesystem {

HRESULT GetVolumePathNamesForVolumeName(const vlr::tstring& sVolumeName, std::vector<vlr::tstring>& vecPathNames_Result)
{
	TCHAR pszBuffer[2048];
	DWORD dwReturnLength{};
	BOOL bResult = ::GetVolumePathNamesForVolumeName(sVolumeName.c_str(), pszBuffer, 2048, &dwReturnLength);
	if (!bResult)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	auto spanVolumePathNames = cpp::span<const TCHAR>(pszBuffer, dwReturnLength);

	vlr::util::data_adaptor::HelperFor_MultiSZ{}.ToStructuredData(spanVolumePathNames, vecPathNames_Result);

	return S_OK;
}

} // namespace filesystem

} // namespace win32

} // namespace vlr
