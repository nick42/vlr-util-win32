#include "pch.h"
#include "platform.API.Win32.h"

namespace vlr {

namespace win32 {

namespace platform {

namespace API {

const Win32::F_IsWow64Process2& CWin32::GetFunction_IsWow64Process2()
{
	if (m_fIsWow64Process2.m_srLoadResult.isSet())
	{
		return m_fIsWow64Process2;
	}

	const auto& oLibrary = GetDynamicLoadLibrary(_T("Kernel32.dll"));
	if (!oLibrary.m_hLibrary)
	{
		m_fIsWow64Process2.m_srLoadResult = E_FAIL;
		return m_fIsWow64Process2;
	}

	auto fProc = ::GetProcAddress(oLibrary.m_hLibrary, "IsWow64Process2");
	if (!fProc)
	{
		m_fIsWow64Process2.m_srLoadResult = HRESULT_FROM_WIN32(GetLastError());
		return m_fIsWow64Process2;
	}

	m_fIsWow64Process2.m_fFunction = (Win32::API_IsWow64Process2)fProc;
	m_fIsWow64Process2.m_srLoadResult = S_OK;

	return m_fIsWow64Process2;
}

const CDynamicLoadedLibrary& CWin32::GetDynamicLoadLibrary(const vlr::tzstring_view svzLibraryName)
{
	auto sLibraryName = svzLibraryName.toStdString();

	auto iterIndex = m_mapLoadNameToLibrary.find(sLibraryName);
	if (iterIndex != m_mapLoadNameToLibrary.end())
	{
		return iterIndex->second;
	}

	auto& oDynamicLoadLibrary = m_mapLoadNameToLibrary[sLibraryName];
	oDynamicLoadLibrary.m_sLoadName = sLibraryName;

	// TODO? Add more options around loading
	if (m_fResultModuleLoad)
	{
		HMODULE hLibrary{};
		auto sr = m_fResultModuleLoad(svzLibraryName, hLibrary);
		oDynamicLoadLibrary.m_srLoadResult = sr;
		oDynamicLoadLibrary.m_hLibrary = hLibrary;
	}
	else
	{
		oDynamicLoadLibrary.m_hLibrary = ::LoadLibrary(svzLibraryName);
		if (oDynamicLoadLibrary.m_hLibrary)
		{
			oDynamicLoadLibrary.m_srLoadResult = S_OK;
		}
		else
		{
			oDynamicLoadLibrary.m_srLoadResult = SResult::For_win32_LastError();
		}
	}

	return oDynamicLoadLibrary;
}

} // namespace API

} // namespace platform

} // namespace win32

} // namespace vlr
