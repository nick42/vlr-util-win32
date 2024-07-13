#pragma once

#include <map>
#include <mutex>
#include <wow64apiset.h>

#include <vlr-util/DynamicLoadedFunction.h>
#include <vlr-util/StringCompare.h>
#include <vlr-util/zstring_view.h>

#include "DynamicLoadedLibrary.h"
#include "DynamicLoadInfo_Function.h"

namespace vlr {

namespace win32 {

namespace platform {

class CDynamicLoadProc
{
public:
	static auto& GetSharedInstance()
	{
		static CDynamicLoadProc theInstance;
		return theInstance;
	}

protected:
	std::recursive_mutex m_mutexDataAccess;
	// Note: In Windows, libraries are looked up by the case-insensitive name to check if they are loaded, and if found,
	// an existing library is always returned. So we emulate this here.
	std::map<vlr::tstring, CDynamicLoadedLibrary, vlr::StringCompare::asCaseInsensitive> m_mapLoadNameToLibrary;

	std::map<vlr::tstring, SPCDynamicLoadedFunctionBase, vlr::StringCompare::asCaseInsensitive> m_mapFunctionIdentifierToLoadedInstance;

	SResult ResolveDynamicLoadForLibrary(
		CDynamicLoadedLibrary& oDynamicLoadLibrary);
	SResult ResolveDynamicLoadForLibrary_PreLoaded(
		CDynamicLoadedLibrary& oDynamicLoadLibrary);
	SResult ResolveDynamicLoadForLibrary_Default(
		CDynamicLoadedLibrary& oDynamicLoadLibrary);

	SResult PopulateFunctionIdentifier(
		const CDynamicLoadInfo_Function& oLoadInfo_Function,
		const CDynamicLoadInfo_Library& oLoadInfo_Library,
		vlr::tstring& sFunctionIdentifier);
	inline vlr::tstring GetFunctionIdentifier_Inline(
		const CDynamicLoadInfo_Function& oLoadInfo_Function,
		const CDynamicLoadInfo_Library& oLoadInfo_Library)
	{
		vlr::tstring sFunctionIdentifier;
		PopulateFunctionIdentifier(oLoadInfo_Function, oLoadInfo_Library, sFunctionIdentifier);
		return sFunctionIdentifier;
	}

	SResult SaveLoadResultToMap(
		const CDynamicLoadInfo_Function& oLoadInfo_Function,
		const CDynamicLoadInfo_Library& oLoadInfo_Library,
		SResult srLoadResult,
		const SPCDynamicLoadedFunctionBase& spDynamicLoadedFunction);
	SResult SaveLoadResultToMap(
		const vlr::tstring& sFunctionIdentifier,
		SResult srLoadResult,
		const SPCDynamicLoadedFunctionBase& spDynamicLoadedFunction);

public:
	const CDynamicLoadedLibrary& GetDynamicLoadLibrary(const vlr::tzstring_view svzLibraryName);
	const CDynamicLoadedLibrary& GetDynamicLoadLibrary(const CDynamicLoadInfo_Library& oLoadInfo);

	// Note: If spDynamicLoadFunction_Typed is not null, this method will populate this instance on success, and store in the map.
	// This allows the class to potentially cache the typed version of a function class, rather than just the untyped base.
	SResult TryPopulateFunction(
		const CDynamicLoadInfo_Function& oLoadInfo,
		SPCDynamicLoadedFunctionBase& spDynamicLoadFunction_Result,
		const SPCDynamicLoadedFunctionBase& spDynamicLoadFunction_Typed = {});

	//SResult CheckIfFunctionLoaded(
	//	const CDynamicLoadInfo_Function& oLoadInfo,
	//	SPCDynamicLoadedFunctionBase& spDynamicLoadFunction);
	//SResult UpdateLoadedFunction(
	//	const CDynamicLoadInfo_Function& oLoadInfo,
	//	const SPCDynamicLoadedFunctionBase& spDynamicLoadFunction);

	// Note: Use this member to specify an override for module resolution (eg: to add enhanced security).
	// The current default simply calls LoadLibary.
	using FResolveModuleLoad = std::function<SResult(const CDynamicLoadInfo_Library& oLoadInfo, HMODULE hLibrary)>;
	FResolveModuleLoad m_fResultModuleLoad;

};

} // namespace platform

} // namespace win32

} // namespace vlr
