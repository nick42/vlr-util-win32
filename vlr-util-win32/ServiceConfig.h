#pragma once

#include <vlr-util/util.includes.h>
#include <vlr-util/util.logical_zstring_view.h>

namespace vlr {

namespace win32 {

class CServiceConfig
{
public:
	util::logical_tzstring_view m_svzServiceName;
	util::logical_tzstring_view m_svzServiceName_Display;
	DWORD m_dwDesiredAccess = SERVICE_ALL_ACCESS;
	DWORD m_dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	DWORD m_dwStartType = SERVICE_DEMAND_START;
	DWORD m_dwErrorControl = SERVICE_ERROR_NORMAL;
	vlr::tstring m_sFilePath_ServiceBinary;
	util::logical_tzstring_view m_svzLoadOrderGroup;
	LPDWORD m_lpdwTagId = nullptr;
	util::logical_tzstring_view m_svzDependencies;
	util::logical_tzstring_view m_svzRunAsAccount_Username;
	util::logical_tzstring_view m_svzRunAsAccount_Password;

public:
	inline auto& withServiceName(vlr::tzstring_view_param svzServiceName) noexcept
	{
		m_svzServiceName = svzServiceName;
		return *this;
	}
	inline auto& withServiceName_Display(vlr::tzstring_view_param svzServiceName_Display) noexcept
	{
		m_svzServiceName_Display = svzServiceName_Display;
		return *this;
	}
	inline auto& withFilePath_ServiceBinary(vlr::tstring sFilePath_ServiceBinary)
	{
		m_sFilePath_ServiceBinary = sFilePath_ServiceBinary;
		return *this;
	}

};

} // namespace win32

} // namespace vlr
