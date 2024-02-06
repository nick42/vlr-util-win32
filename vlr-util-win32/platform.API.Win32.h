#pragma once

#include <map>
#include <wow64apiset.h>

#include <vlr-util/DynamicLoadedFunction.h>
#include <vlr-util/StringCompare.h>

#include "DynamicLoadedLibrary.h"

namespace vlr {

namespace win32 {

namespace platform {

namespace API {

namespace Win32 {

// https://learn.microsoft.com/en-us/windows/win32/api/wow64apiset/nf-wow64apiset-iswow64process2

typedef BOOL (WINAPI *API_IsWow64Process2)(
	/*[in]*/            HANDLE hProcess,
	/*[out]*/           USHORT* pProcessMachine,
	/*[out, optional]*/ USHORT* pNativeMachine
);

using F_IsWow64Process2 = vlr::CDynamicLoadedFunction<BOOL(
	/*[in]*/            HANDLE /*hProcess*/,
	/*[out]*/           USHORT* /*pProcessMachine*/,
	/*[out, optional]*/ USHORT* /*pNativeMachine*/
)>;

} // namespace Win32

class CWin32
{
public:
	static auto& GetSharedInstance()
	{
		static CWin32 theInstance;
		return theInstance;
	}

protected:
	std::map<vlr::tstring, CDynamicLoadedLibrary, vlr::StringCompare::asCaseInsensitive> m_mapLoadNameToLibrary;

	Win32::F_IsWow64Process2 m_fIsWow64Process2;

public:
	// Note: Use this member to specify an override for module resolution (eg: to add enhanced security).
	// The current default simply calls LoadLibary.
	using FResolveModuleLoad = std::function<SResult(const vlr::tzstring_view svzLibraryName, HMODULE hLibrary)>;
	FResolveModuleLoad m_fResultModuleLoad;

	const Win32::F_IsWow64Process2& GetFunction_IsWow64Process2();

protected:
	const CDynamicLoadedLibrary& GetDynamicLoadLibrary(const vlr::tzstring_view svzLibraryName);

};

} // namespace API

} // namespace platform

} // namespace win32

} // namespace vlr
