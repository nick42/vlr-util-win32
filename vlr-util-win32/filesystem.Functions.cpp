#include "pch.h"
#include "filesystem.Functions.h"

#include <vlr-util/util.data_adaptor.MultiSZ.h>

VLR_NAMESPACE_BEGIN( vlr )

VLR_NAMESPACE_BEGIN( win32 )

VLR_NAMESPACE_BEGIN( filesystem )

HRESULT GetVolumePathNamesForVolumeName( const vlr::tstring& sVolumeName, std::vector<vlr::tstring>& oPathNameList )
{
	TCHAR pszBuffer[2048];
	DWORD dwReturnLength{};
	BOOL bResult = ::GetVolumePathNamesForVolumeName( sVolumeName.c_str(), pszBuffer, 2048, &dwReturnLength );
	if (!bResult)
	{
		return HRESULT_FROM_WIN32( GetLastError() );
	}

	vlr::util::data_adaptor::HelperFor_MultiSZ{}.ToStructuredData( pszBuffer, oPathNameList );

	return S_OK;
}

VLR_NAMESPACE_END //( filesystem )

VLR_NAMESPACE_END //( win32 )

VLR_NAMESPACE_END //( vlr )
