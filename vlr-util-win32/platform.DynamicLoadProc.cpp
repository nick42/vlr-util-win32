#include "pch.h"
#include "platform.DynamicLoadProc.h"

#include <vlr-util/util.convert.StringConversion.h>

namespace vlr {

namespace win32 {

namespace platform {

SResult CDynamicLoadProc::ResolveDynamicLoadForLibrary(
	CDynamicLoadedLibrary& oDynamicLoadLibrary)
{
	SResult sr;

	// This should not be the case on a call...
	if (oDynamicLoadLibrary.HasLoadBeenAttempted())
	{
		vlr::assert::HandleCheckFailure(VLR_LOG_CONTEXT_WARNING, _T("Unexpected call state for dynamic library resolution"));
		return SResult::Success_NoWorkDone;
	}

	// TODO? Add more options around loading
	if (m_fResultModuleLoad)
	{
		HMODULE hLibrary{};
		sr = m_fResultModuleLoad(oDynamicLoadLibrary.m_oLoadInfo, hLibrary);
		oDynamicLoadLibrary.m_srLoadResult = sr;
		oDynamicLoadLibrary.m_hLibrary = hLibrary;

		return SResult::Success;
	}

	sr = ResolveDynamicLoadForLibrary_PreLoaded(oDynamicLoadLibrary);
	if (!sr.isSuccess())
	{
		// Failure on load; propagate the failure code
		return sr;
	}
	if (sr == SResult::Success)
	{
		// Resolved from pre-loaded
		return sr;
	}

	sr = ResolveDynamicLoadForLibrary_Default(oDynamicLoadLibrary);
	if (!sr.isSuccess())
	{
		// Failure on load; propagate the failure code
		return sr;
	}

	return SResult::Success;
}

SResult CDynamicLoadProc::ResolveDynamicLoadForLibrary_PreLoaded(
	CDynamicLoadedLibrary& oDynamicLoadLibrary)
{
	if (!oDynamicLoadLibrary.m_oLoadInfo.GetExpectLibraryAlreadyLoaded())
	{
		return SResult::Success_NoWorkDone;
	}

	BOOL bResult{};

	auto sLibraryName = oDynamicLoadLibrary.m_oLoadInfo.GetLibraryName_Normalized();

	HMODULE hModule{};
	bResult = ::GetModuleHandleEx(
		0,
		sLibraryName.c_str(),
		&hModule);
	if (!bResult)
	{
		oDynamicLoadLibrary.m_srLoadResult = SResult::For_win32_LastError();
		return oDynamicLoadLibrary.m_srLoadResult;
	}

	oDynamicLoadLibrary.m_srLoadResult = S_OK;
	oDynamicLoadLibrary.m_hLibrary = hModule;

	return SResult::Success;
}

SResult CDynamicLoadProc::ResolveDynamicLoadForLibrary_Default(
	CDynamicLoadedLibrary& oDynamicLoadLibrary)
{
	auto sLibraryName = oDynamicLoadLibrary.m_oLoadInfo.GetLibraryName_Normalized();
	DWORD dwFlags = oDynamicLoadLibrary.m_oLoadInfo.GetEffectiveFlags_LoadLibraryEx();

	oDynamicLoadLibrary.m_hLibrary = ::LoadLibraryEx(
		sLibraryName.c_str(),
		NULL,
		dwFlags);
	if (oDynamicLoadLibrary.m_hLibrary)
	{
		oDynamicLoadLibrary.m_srLoadResult = S_OK;
	}
	else
	{
		oDynamicLoadLibrary.m_srLoadResult = SResult::For_win32_LastError();
	}

	return oDynamicLoadLibrary.m_srLoadResult;
}

SResult CDynamicLoadProc::PopulateFunctionIdentifier(
	const CDynamicLoadInfo_Function& oLoadInfo_Function,
	const CDynamicLoadInfo_Library& oLoadInfo_Library,
	vlr::tstring& sFunctionIdentifier)
{
	// Note: We use '/' as the delineator, because this won't be present in valid library names or function names.
	// We use the unqualified filename, because this is how Windows stores dynamic library references _normally_.
	// Note: It is possible to have multiple path-qualified DLL's with the same names loaded; TBD to support this.

	sFunctionIdentifier = fmt::format(_T("{}/{}"),
		oLoadInfo_Library.GetLibraryName_FilenameOnly(),
		oLoadInfo_Function.m_sFunctionName);

	return SResult::Success;
}

SResult CDynamicLoadProc::SaveLoadResultToMap(
	const CDynamicLoadInfo_Function& oLoadInfo_Function,
	const CDynamicLoadInfo_Library& oLoadInfo_Library,
	SResult srLoadResult,
	const SPCDynamicLoadedFunctionBase& spDynamicLoadedFunction)
{
	auto sFunctionIdentifier = GetFunctionIdentifier_Inline(oLoadInfo_Function, oLoadInfo_Library);

	return SaveLoadResultToMap(sFunctionIdentifier, srLoadResult, spDynamicLoadedFunction);
}

SResult CDynamicLoadProc::SaveLoadResultToMap(
	const vlr::tstring& sFunctionIdentifier,
	SResult srLoadResult,
	const SPCDynamicLoadedFunctionBase& spDynamicLoadedFunction)
{
	SPCDynamicLoadedFunctionBase spDynamicLoadedFunction_ToBeSaved = spDynamicLoadedFunction;
	if (!spDynamicLoadedFunction_ToBeSaved)
	{
		// We should not have a valid load without a valid function ptr
		VLR_ASSERT_NONZERO_OR_CONTINUE(!srLoadResult.isSuccess());

		spDynamicLoadedFunction_ToBeSaved = std::make_shared<CDynamicLoadedFunctionBase>();
		spDynamicLoadedFunction_ToBeSaved->m_srLoadResult = srLoadResult;
	}

	auto slDataAccess = std::scoped_lock{ m_mutexDataAccess };

	m_mapFunctionIdentifierToLoadedInstance[sFunctionIdentifier] = spDynamicLoadedFunction_ToBeSaved;

	return SResult::Success;
}

const CDynamicLoadedLibrary& CDynamicLoadProc::GetDynamicLoadLibrary(const vlr::tzstring_view svzLibraryName)
{
	return GetDynamicLoadLibrary(CDynamicLoadInfo_Library{ svzLibraryName });
}

const CDynamicLoadedLibrary& CDynamicLoadProc::GetDynamicLoadLibrary(const CDynamicLoadInfo_Library& oLoadInfo)
{
	SResult sr;

	auto slDataAccess = std::scoped_lock{ m_mutexDataAccess };

	// Note: We will always populate a map entry for the library, even if the load fails, so no need to do multiple lookups.

	auto& oDynamicLoadLibrary = m_mapLoadNameToLibrary[oLoadInfo.GetLibraryName_FilenameOnly()];
	if (oDynamicLoadLibrary.HasLoadBeenAttempted())
	{
		return oDynamicLoadLibrary;
	}

	// Attempt the load

	oDynamicLoadLibrary.m_oLoadInfo = oLoadInfo;

	sr = ResolveDynamicLoadForLibrary(oDynamicLoadLibrary);
	VLR_ASSERT_SR_SUCCEEDED_OR_CONTINUE(sr);

	return oDynamicLoadLibrary;
}

SResult CDynamicLoadProc::TryPopulateFunction(
	const CDynamicLoadInfo_Function& oLoadInfo,
	SPCDynamicLoadedFunctionBase& spDynamicLoadFunction_Result,
	const SPCDynamicLoadedFunctionBase& spDynamicLoadFunction_Typed /*= {}*/)
{
	SResult sr;

	auto slDataAccess = std::scoped_lock{ m_mutexDataAccess };

	auto fUpdateMapEntryForTypedFunctionAsApplicable = [&](SPCDynamicLoadedFunctionBase& spDynamicLoadFunction_Existing)
	{
		if (!spDynamicLoadFunction_Typed)
		{
			// Caller did not pass valid existing (typed, by implication) instance of function class; no need to update map
			return SResult::Success_NoWorkDone;
		}
		else if (spDynamicLoadFunction_Typed.get() == spDynamicLoadFunction_Existing.get())
		{
			// Nothing to do; function already in map
			// Note: This is an unexpected case, but should not cause errors...
			return SResult::Success_NoWorkDone;
		}
		// ... else, copy _data_ into the passed-in object, then copy passed-in object into our map entry

		spDynamicLoadFunction_Typed->m_srLoadResult = spDynamicLoadFunction_Existing->m_srLoadResult;
		spDynamicLoadFunction_Typed->SetFromRawPtr(spDynamicLoadFunction_Existing->m_pvRawFunctionPointer);

		spDynamicLoadFunction_Existing = spDynamicLoadFunction_Typed;

		return SResult::Success;
	};

	for (const auto& oLoadInfo_Library : oLoadInfo.m_vecLibraryLoadInfo)
	{
		auto sFunctionIdentifier = GetFunctionIdentifier_Inline(oLoadInfo, oLoadInfo_Library);

		auto iterIndex_Existing = m_mapFunctionIdentifierToLoadedInstance.find(sFunctionIdentifier);
		if (iterIndex_Existing != m_mapFunctionIdentifierToLoadedInstance.end())
		{
			SPCDynamicLoadedFunctionBase& spDynamicLoadFunction_Existing = iterIndex_Existing->second;
			if (!spDynamicLoadFunction_Existing)
			{
				vlr::assert::HandleCheckFailure(VLR_LOG_CONTEXT_WARNING, _T("Unexpected null in dynamic function load map"));
				continue;
			}
			if (!spDynamicLoadFunction_Existing->m_srLoadResult.isSuccess())
			{
				continue;
			}

			// We have a successful previous load of this function. All we need to do is potentially update our map entry 
			// (to typed pointer, if provided), and then return the entry.

			fUpdateMapEntryForTypedFunctionAsApplicable(spDynamicLoadFunction_Existing);

			spDynamicLoadFunction_Result = spDynamicLoadFunction_Existing;

			return SResult::Success;
		}

		const auto& oDynamicLoadLibrary = GetDynamicLoadLibrary(oLoadInfo_Library);
		if (!oDynamicLoadLibrary.LibraryLoadedSuccessfully())
		{
			sr = SaveLoadResultToMap(oLoadInfo, oLoadInfo_Library, ERROR_FILE_NOT_FOUND, nullptr);
			VLR_ASSERT_SR_SUCCEEDED_OR_CONTINUE(sr);
			continue;
		}

		auto fProc = ::GetProcAddress(oDynamicLoadLibrary.m_hLibrary, util::Convert::ToStdStringA(oLoadInfo.m_sFunctionName).c_str());
		if (!fProc)
		{
			// Not found in this library
			sr = SaveLoadResultToMap(oLoadInfo, oLoadInfo_Library, ERROR_FILE_NOT_FOUND, nullptr);
			VLR_ASSERT_SR_SUCCEEDED_OR_CONTINUE(sr);
			continue;
		}

		// We have loaded the function. Create an entry for it.

		auto spDynamicLoadFunction_Loaded = std::make_shared<CDynamicLoadedFunctionBase>();
		VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(spDynamicLoadFunction_Loaded);

		spDynamicLoadFunction_Loaded->m_srLoadResult = SResult::Success;
		spDynamicLoadFunction_Loaded->SetFromRawPtr(fProc);

		sr = SaveLoadResultToMap(oLoadInfo, oLoadInfo_Library, SResult::Success, spDynamicLoadFunction_Loaded);
		VLR_ASSERT_SR_SUCCEEDED_OR_CONTINUE(sr);

		fUpdateMapEntryForTypedFunctionAsApplicable(spDynamicLoadFunction_Loaded);

		spDynamicLoadFunction_Result = spDynamicLoadFunction_Loaded;

		return SResult::Success;
	}

	// Note: Should have updated all load info in map already (for all attempted load locations).

	return ERROR_FILE_NOT_FOUND;
}

} // namespace platform

} // namespace win32

} // namespace vlr
