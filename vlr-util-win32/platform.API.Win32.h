#pragma once

#include <map>
#include <mutex>
#include <wow64apiset.h>

#include <vlr-util/DynamicLoadedFunction.h>
#include <vlr-util/StringCompare.h>

#include "DynamicLoadedLibrary.h"
#include "DynamicLoadInfo_Function.h"
#include "platform.DynamicLoadProc.h"

namespace vlr {

namespace win32 {

namespace platform {

namespace API {

namespace Win32 {

// https://learn.microsoft.com/en-us/windows/win32/api/wow64apiset/nf-wow64apiset-iswow64process2

//typedef BOOL (WINAPI *API_IsWow64Process2)(
//	/*[in]*/            HANDLE hProcess,
//	/*[out]*/           USHORT* pProcessMachine,
//	/*[out, optional]*/ USHORT* pNativeMachine
//);

using F_IsWow64Process2 = vlr::CDynamicLoadedFunction<BOOL WINAPI(
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
	CDynamicLoadProc* m_pDynamicLoadProc_Override = nullptr;
	CDynamicLoadProc& GetDynamicLoadProc() const
	{
		return m_pDynamicLoadProc_Override ? *m_pDynamicLoadProc_Override : CDynamicLoadProc::GetSharedInstance();
	}

	std::recursive_mutex m_mutexDataAccess;

	std::shared_ptr<Win32::F_IsWow64Process2> m_spIsWow64Process2;

public:
	const Win32::F_IsWow64Process2& GetFunction_IsWow64Process2();

public:
	CWin32() = default;
	CWin32(CDynamicLoadProc* pDynamicLoadProc_Override)
		: m_pDynamicLoadProc_Override{ pDynamicLoadProc_Override }
	{}
};

} // namespace API

} // namespace platform

} // namespace win32

} // namespace vlr
