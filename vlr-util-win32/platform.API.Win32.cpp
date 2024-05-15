#include "pch.h"
#include "platform.API.Win32.h"

namespace vlr {

namespace win32 {

namespace platform {

namespace API {

const CDynamicLoadedLibrary& CWin32::GetDynamicLoadLibrary(const vlr::tzstring_view svzLibraryName)
{
	return GetDynamicLoadLibrary(CDynamicLoadInfo_Library{ svzLibraryName });
}

const CDynamicLoadedLibrary& CWin32::GetDynamicLoadLibrary(const CDynamicLoadInfo_Library& oLoadInfo)
{
	auto slDataAccess = std::scoped_lock{ m_mutexDataAccess };

	auto iterIndex = m_mapLoadNameToLibrary.find(oLoadInfo.getLibraryName());
	if (iterIndex != m_mapLoadNameToLibrary.end())
	{
		return iterIndex->second;
	}

	auto& oDynamicLoadLibrary = m_mapLoadNameToLibrary[oLoadInfo.getLibraryName()];
	oDynamicLoadLibrary.m_oLoadInfo = oLoadInfo;

	// TODO? Add more options around loading
	if (m_fResultModuleLoad)
	{
		HMODULE hLibrary{};
		auto sr = m_fResultModuleLoad(oLoadInfo.getLibraryName(), hLibrary);
		oDynamicLoadLibrary.m_srLoadResult = sr;
		oDynamicLoadLibrary.m_hLibrary = hLibrary;

		return oDynamicLoadLibrary;
	}

	oDynamicLoadLibrary.m_hLibrary = ::LoadLibraryEx(
		oDynamicLoadLibrary.m_oLoadInfo.getLibraryName().c_str(),
		NULL,
		oDynamicLoadLibrary.m_oLoadInfo.getFLags_LoadLibraryEx());
	if (oDynamicLoadLibrary.m_hLibrary)
	{
		oDynamicLoadLibrary.m_srLoadResult = S_OK;
	}
	else
	{
		oDynamicLoadLibrary.m_srLoadResult = SResult::For_win32_LastError();
	}

	return oDynamicLoadLibrary;
}

SResult CWin32::TryLoadFunction(
	const CDynamicLoadInfo_Function& oLoadInfo,
	SPCDynamicLoadedFunctionBase& spDynamicLoadFunction)
{
	auto slDataAccess = std::scoped_lock{ m_mutexDataAccess };

	// Check if already loaded
	auto iterIndex = m_mapFunctionNameToLoadedInstance.find(oLoadInfo.m_saFunctionName);
	if (iterIndex != m_mapFunctionNameToLoadedInstance.end())
	{
		spDynamicLoadFunction = iterIndex->second;
		return SResult::Success;
	}

	spDynamicLoadFunction = std::make_shared<CDynamicLoadedFunctionBase>();
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(spDynamicLoadFunction);

	for (const auto& oLoadInfo_Library : oLoadInfo.m_vecLibraryLoadInfo)
	{
		const auto& oDynamicLoadLibrary = GetDynamicLoadLibrary(oLoadInfo_Library);
		if (!oDynamicLoadLibrary.LibraryLoadedSuccessfully())
		{
			continue;
		}

		auto fProc = ::GetProcAddress(oDynamicLoadLibrary.m_hLibrary, oLoadInfo.m_saFunctionName.c_str());
		if (!fProc)
		{
			// Not found in this library
			continue;
		}

		// We have loaded the function
		spDynamicLoadFunction->m_srLoadResult = SResult::Success;
		spDynamicLoadFunction->m_pvRawFunctionPointer = fProc;

		m_mapFunctionNameToLoadedInstance[oLoadInfo.m_saFunctionName] = spDynamicLoadFunction;

		return SResult::Success;
	}

	return ERROR_FILE_NOT_FOUND;
}

const Win32::F_IsWow64Process2& CWin32::GetFunction_IsWow64Process2()
{
	auto slDataAccess = std::scoped_lock{ m_mutexDataAccess };

	static constexpr vlr::zstring_view svzFunctionName = "IsWow64Process2";

	if (m_spIsWow64Process2)
	{
		return *m_spIsWow64Process2;
	}

	m_spIsWow64Process2 = std::make_shared<Win32::F_IsWow64Process2>();
	auto& oDynamicFunction = *m_spIsWow64Process2;

	// Check if the function may have already been cached by direct/named load
	auto iterIndex = m_mapFunctionNameToLoadedInstance.find(svzFunctionName.toStdString());
	if (iterIndex != m_mapFunctionNameToLoadedInstance.end())
	{
		const auto& spDynamicFunctionRaw = iterIndex->second;
		oDynamicFunction.m_srLoadResult = spDynamicFunctionRaw->m_srLoadResult;
		oDynamicFunction.SetFromRawPtr(spDynamicFunctionRaw->m_pvRawFunctionPointer);

		// Replace typeless function with typed function in map...
		m_mapFunctionNameToLoadedInstance[svzFunctionName.toStdString()] = m_spIsWow64Process2;

		return *m_spIsWow64Process2;
	}

	const auto& oLibrary = GetDynamicLoadLibrary(_T("Kernel32.dll"));
	if (!oLibrary.m_hLibrary)
	{
		oDynamicFunction.m_srLoadResult = E_FAIL;
		return oDynamicFunction;
	}

	auto fProc = ::GetProcAddress(oLibrary.m_hLibrary, svzFunctionName);
	if (!fProc)
	{
		oDynamicFunction.m_srLoadResult = HRESULT_FROM_WIN32(GetLastError());
		return oDynamicFunction;
	}

	oDynamicFunction.SetFromRawPtr(fProc);
	oDynamicFunction.m_srLoadResult = S_OK;

	return oDynamicFunction;
}

} // namespace API

} // namespace platform

} // namespace win32

} // namespace vlr
