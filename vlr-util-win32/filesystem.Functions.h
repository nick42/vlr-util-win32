#pragma once

#include <list>

#include <vlr-util/util.includes.h>

namespace vlr {

namespace win32 {

namespace filesystem {

HRESULT GetVolumePathNamesForVolumeName( const vlr::tstring& sVolumeName, std::list<vlr::tstring>& oPathNameList );

} // namespace filesystem

} // namespace win32

} // namespace vlr
