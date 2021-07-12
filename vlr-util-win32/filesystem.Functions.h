#pragma once

#include <list>

#include <vlr-util/util.includes.h>

VLR_NAMESPACE_BEGIN( vlr )

VLR_NAMESPACE_BEGIN( win32 )

VLR_NAMESPACE_BEGIN( filesystem )

HRESULT GetVolumePathNamesForVolumeName( const vlr::tstring& sVolumeName, std::list<vlr::tstring>& oPathNameList );

VLR_NAMESPACE_END //( filesystem )

VLR_NAMESPACE_END //( win32 )

VLR_NAMESPACE_END //( vlr )
