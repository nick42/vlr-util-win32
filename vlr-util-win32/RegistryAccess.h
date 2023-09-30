#pragma once

#include <vlr-util/util.includes.h>
#include <vlr-util/zstring_view.h>
#include <vlr-util/util.data_adaptor.MultiSZ.h>
#include <vlr-util/util.Result.h>
#include <vlr-util/ModuleContext.Compilation.h>

VLR_NAMESPACE_BEGIN(vlr)

VLR_NAMESPACE_BEGIN(win32)

class RegistryAccess
{
protected:
	HKEY m_hBaseKey = {};

	static constexpr size_t m_nMaxIterationCountForRead_Default = 2;
	size_t m_nMaxIterationCountForRead = m_nMaxIterationCountForRead_Default;
	static constexpr size_t m_OnReadValue_nDefaultBufferSize = 1024;

	auto getBaseKey() const
	{
		return m_hBaseKey;
	}

public:
	SResult CheckKeyExists(tzstring_view svzKeyName) const;
	inline bool DoesKeyExist(tzstring_view svzKeyName) const
	{
		auto sr = CheckKeyExists(svzKeyName);
		return (sr == SResult::Success);
	}

	struct Options_EnsureKeyExists
	{
		// TODO: Add desired permissions, etc.
	};
	SResult EnsureKeyExists(
		tzstring_view svzKeyName,
		const Options_EnsureKeyExists& options = {}) const;

	struct Options_DeleteKey
	{
		bool m_bEnsureSafeDelete = true;
		std::vector<tstring> m_arrSafeDeletePaths;

		decltype(auto) withSafeDeletePath(tstring_view svzPath)
		{
			m_arrSafeDeletePaths.emplace_back(svzPath);
			return *this;
		}
	};
	SResult DeleteKey(
		tzstring_view svzKeyName,
		const Options_DeleteKey& options = {}) const;

	SResult ReadValueBase(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		DWORD& dwType_Result, 
		std::vector<BYTE>& arrData);
	SResult WriteValueBase(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const DWORD& dwType,
		const std::vector<BYTE>& arrData);

	SResult ReadValue_String(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::string& saValue);
	SResult WriteValue_String(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::string& saValue);
	SResult ReadValue_String(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::wstring& swValue);
	SResult WriteValue_String(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::wstring& swValue);

	// TODO? Support data coercion

	SResult convertRegDataToValue_String(
		const DWORD& dwType,
		const std::vector<BYTE>& arrData,
		std::string& saValue);
	SResult convertValueToRegData_String(
		const std::string& saValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData);
	SResult convertRegDataToValue_String(
		const DWORD& dwType,
		const std::vector<BYTE>& arrData,
		std::wstring& swValue);
	SResult convertValueToRegData_String(
		const std::wstring& swValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData);
	SResult convertRegDataToValueDirect_String_NativeType(
		const DWORD& dwType,
		const std::vector<BYTE>& arrData,
		std::string& saValue);
	SResult convertRegDataToValueDirect_String_NativeType(
		const DWORD& dwType,
		const std::vector<BYTE>& arrData,
		std::wstring& swValue);
	SResult convertValueToRegDataDirect_String_NativeType(
		const std::string& saValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData);
	SResult convertValueToRegDataDirect_String_NativeType(
		const std::wstring& swValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData);

protected:
	SResult openKey(
		tzstring_view svzKeyName,
		DWORD dwAccessMask,
		HKEY& hKey_Result) const;
	DWORD getWow64RedirectionKeyAccessMask() const;

public:
	RegistryAccess() = default;
	RegistryAccess(HKEY hBaseKey)
		: m_hBaseKey{ hBaseKey }
	{}
};

VLR_NAMESPACE_END //( win32 )

VLR_NAMESPACE_END //( vlr )
