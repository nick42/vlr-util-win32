#pragma once

#include <vlr-util/util.includes.h>
#include <vlr-util/util.std_aliases.h>
#include <vlr-util/zstring_view.h>
#include <vlr-util/util.data_adaptor.MultiSZ.h>
#include <vlr-util/util.Result.h>
#include <vlr-util/ModuleContext.Compilation.h>

VLR_NAMESPACE_BEGIN(vlr)

VLR_NAMESPACE_BEGIN(win32)

class RegistryAccess
{
public:
	using QWORD = unsigned __int64;

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

	struct Options_DeleteKeysOrValues
	{
		bool m_bEnsureSafeDelete = false;
		std::vector<tstring> m_arrSafeDeletePaths;

		decltype(auto) withSafeDeletePath(tstring_view svzPath)
		{
			m_arrSafeDeletePaths.emplace_back(svzPath);
			return *this;
		}
	};
	SResult DeleteKey(
		tzstring_view svzKeyName,
		const Options_DeleteKeysOrValues& options = {}) const;

	SResult ReadValueBase(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		DWORD& dwType_Result, 
		std::vector<BYTE>& arrData) const;
	SResult WriteValueBase(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const DWORD& dwType,
		const std::vector<BYTE>& arrData) const;

	SResult ReadValue_String(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::string& saValue) const;
	SResult ReadValue_String(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::string& saValue,
		const std::string& saDefaultResultOnNoValue) const;
	SResult WriteValue_String(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::string& saValue) const;
	SResult WriteValue_String(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::string_view& svValue) const;

	SResult ReadValue_String(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::wstring& swValue) const;
	SResult ReadValue_String(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::wstring& swValue,
		const std::wstring& swDefaultResultOnNoValue) const;
	SResult WriteValue_String(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::wstring& swValue) const;
	SResult WriteValue_String(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::wstring_view& svValue) const;

	SResult ReadValue_DWORD(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		DWORD& dwValue) const;
	SResult ReadValue_DWORD(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		DWORD& dwValue,
		const DWORD& dwDefaultResultOnNoValue) const;
	SResult WriteValue_DWORD(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const DWORD& dwValue) const;

	SResult ReadValue_QWORD(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		QWORD& qwValue) const;
	SResult ReadValue_QWORD(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		QWORD& dwValue,
		const QWORD& qwDefaultResultOnNoValue) const;
	SResult WriteValue_QWORD(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const QWORD& qwValue) const;

	SResult ReadValue_MultiSz(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::vector<vlr::tstring>& arrValueCollection) const;
	SResult ReadValue_MultiSz(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::vector<vlr::tstring>& arrValueCollection,
		const std::vector<vlr::tstring>& arrDefaultResultOnNoValue) const;
	SResult WriteValue_MultiSz(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::vector<vlr::tstring>& arrValueCollection) const;

	SResult ReadValue_Binary(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::vector<BYTE>& arrData) const;
	SResult ReadValue_Binary(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::vector<BYTE>& arrData,
		const std::vector<BYTE>& arrDefaultResultOnNoValue) const;
	SResult WriteValue_Binary(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::vector<BYTE>& arrData) const;

	SResult DeleteValue(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const Options_DeleteKeysOrValues& options = {});

	// Note: This is the "high-level" interface.
	// These methods have template specializations for default supported data types.

	template< typename TValue >
	SResult ReadValue(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		TValue& tValue) const
	{
		static_assert("Unhanded type");
	}
	template< typename TValue >
	SResult ReadValue(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		TValue& tValue,
		const TValue& tDefaultResultOnNoValue) const
	{
		static_assert("Unhanded type");
	}
	template< typename TValue >
	SResult WriteValue(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const TValue& tValue) const
	{
		static_assert("Unhanded type");
	}

	template<>
	inline SResult ReadValue<std::string>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::string& tValue) const
	{
		return ReadValue_String(
			svzKeyName,
			svzValueName,
			tValue);
	}
	template<>
	inline SResult ReadValue<std::string>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::string& tValue,
		const std::string& tDefaultResultOnNoValue) const
	{
		return ReadValue_String(
			svzKeyName,
			svzValueName,
			tValue,
			tDefaultResultOnNoValue);
	}
	template<>
	inline SResult WriteValue<std::string>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::string& tValue) const
	{
		return WriteValue_String(
			svzKeyName,
			svzValueName,
			tValue);
	}
	template<>
	inline SResult WriteValue<vlr::zstring_view>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const vlr::zstring_view& tValue) const
	{
		return WriteValue_String(
			svzKeyName,
			svzValueName,
			tValue);
	}
	template<>
	inline SResult WriteValue<std::string_view>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::string_view& tValue) const
	{
		return WriteValue_String(
			svzKeyName,
			svzValueName,
			tValue);
	}

	template<>
	inline SResult ReadValue<std::wstring>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::wstring& tValue) const
	{
		return ReadValue_String(
			svzKeyName,
			svzValueName,
			tValue);
	}
	template<>
	inline SResult ReadValue<std::wstring>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::wstring& tValue,
		const std::wstring& tDefaultResultOnNoValue) const
	{
		return ReadValue_String(
			svzKeyName,
			svzValueName,
			tValue,
			tDefaultResultOnNoValue);
	}
	template<>
	inline SResult WriteValue<std::wstring>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::wstring& tValue) const
	{
		return WriteValue_String(
			svzKeyName,
			svzValueName,
			tValue);
	}
	template<>
	inline SResult WriteValue<vlr::wzstring_view>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const vlr::wzstring_view& tValue) const
	{
		return WriteValue_String(
			svzKeyName,
			svzValueName,
			tValue);
	}
	template<>
	inline SResult WriteValue<std::wstring_view>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::wstring_view& tValue) const
	{
		return WriteValue_String(
			svzKeyName,
			svzValueName,
			tValue);
	}

	template<>
	inline SResult ReadValue<DWORD>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		DWORD& tValue) const
	{
		return ReadValue_DWORD(
			svzKeyName,
			svzValueName,
			tValue);
	}
	template<>
	inline SResult ReadValue<DWORD>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		DWORD& tValue,
		const DWORD& tDefaultResultOnNoValue) const
	{
		return ReadValue_DWORD(
			svzKeyName,
			svzValueName,
			tValue,
			tDefaultResultOnNoValue);
	}
	template<>
	inline SResult WriteValue<DWORD>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const DWORD& tValue) const
	{
		return WriteValue_DWORD(
			svzKeyName,
			svzValueName,
			tValue);
	}

	template<>
	inline SResult ReadValue<QWORD>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		QWORD& tValue) const
	{
		return ReadValue_QWORD(
			svzKeyName,
			svzValueName,
			tValue);
	}
	template<>
	inline SResult ReadValue<QWORD>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		QWORD& tValue,
		const QWORD& tDefaultResultOnNoValue) const
	{
		return ReadValue_QWORD(
			svzKeyName,
			svzValueName,
			tValue,
			tDefaultResultOnNoValue);
	}
	template<>
	inline SResult WriteValue<QWORD>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const QWORD& tValue) const
	{
		return WriteValue_QWORD(
			svzKeyName,
			svzValueName,
			tValue);
	}

	template<>
	inline SResult ReadValue<std::vector<vlr::tstring>>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::vector<vlr::tstring>& tValue) const
	{
		return ReadValue_MultiSz(
			svzKeyName,
			svzValueName,
			tValue);
	}
	template<>
	inline SResult ReadValue<std::vector<vlr::tstring>>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::vector<vlr::tstring>& tValue,
		const std::vector<vlr::tstring>& tDefaultResultOnNoValue) const
	{
		return ReadValue_MultiSz(
			svzKeyName,
			svzValueName,
			tValue,
			tDefaultResultOnNoValue);
	}
	template<>
	inline SResult WriteValue<std::vector<vlr::tstring>>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::vector<vlr::tstring>& tValue) const
	{
		return WriteValue_MultiSz(
			svzKeyName,
			svzValueName,
			tValue);
	}

	template<>
	inline SResult ReadValue<std::vector<BYTE>>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::vector<BYTE>& tValue) const
	{
		return ReadValue_Binary(
			svzKeyName,
			svzValueName,
			tValue);
	}
	template<>
	inline SResult ReadValue<std::vector<BYTE>>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		std::vector<BYTE>& tValue,
		const std::vector<BYTE>& tDefaultResultOnNoValue) const
	{
		return ReadValue_Binary(
			svzKeyName,
			svzValueName,
			tValue,
			tDefaultResultOnNoValue);
	}
	template<>
	inline SResult WriteValue<std::vector<BYTE>>(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const std::vector<BYTE>& tValue) const
	{
		return WriteValue_Binary(
			svzKeyName,
			svzValueName,
			tValue);
	}

	// TODO? Support data coercion

	SResult convertRegDataToValue_String(
		const DWORD& dwType,
		const std::vector<BYTE>& arrData,
		std::string& saValue) const;
	SResult convertValueToRegData_String(
		const std::string& saValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertValueToRegData_String(
		const std::string_view& svValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertRegDataToValue_String(
		const DWORD& dwType,
		const std::vector<BYTE>& arrData,
		std::wstring& swValue) const;
	SResult convertValueToRegData_String(
		const std::wstring& swValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertValueToRegData_String(
		const std::wstring_view& svValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertRegDataToValueDirect_String_NativeType(
		const DWORD& dwType,
		const std::vector<BYTE>& arrData,
		std::string& saValue) const;
	SResult convertRegDataToValueDirect_String_NativeType(
		const DWORD& dwType,
		const std::vector<BYTE>& arrData,
		std::wstring& swValue) const;
	SResult convertValueToRegDataDirect_String_NativeType(
		const std::string& saValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertValueToRegDataDirect_String_NativeType(
		const std::wstring& swValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertValueToRegDataDirect_String_NativeType(
		const vlr::zstring_view& svzValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertValueToRegDataDirect_String_NativeType(
		const vlr::wzstring_view& svzValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertValueToRegDataDirect_String_NativeType(
		const std::string_view& svValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertValueToRegDataDirect_String_NativeType(
		const std::wstring_view& svValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertRegDataToValue_DWORD(
		const DWORD& dwType,
		const std::vector<BYTE>& arrData,
		DWORD& dwValue) const;
	SResult convertValueToRegData_DWORD(
		const DWORD& dwValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertRegDataToValue_QWORD(
		const DWORD& dwType,
		const std::vector<BYTE>& arrData,
		QWORD& qwValue) const;
	SResult convertValueToRegData_QWORD(
		const QWORD& qwValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertRegDataToValue_MultiSz(
		const DWORD& dwType,
		const std::vector<BYTE>& arrData,
		std::vector<vlr::tstring>& arrValueCollection) const;
	SResult convertValueToRegData_MultiSz(
		const std::vector<vlr::tstring>& arrValueCollection,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertRegDataToValue_Binary(
		const DWORD& dwType,
		const std::vector<BYTE>& arrData,
		std::vector<BYTE>& arrBinaryData) const;
	SResult convertValueToRegData_Binary(
		const std::vector<BYTE>& arrBinaryData,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;

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
