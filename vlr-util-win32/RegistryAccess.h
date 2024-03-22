#pragma once

#include <vlr-util/cpp_namespace.h>
#include <vlr-util/strings.split.h>
#include <vlr-util/util.includes.h>
#include <vlr-util/util.std_aliases.h>
#include <vlr-util/zstring_view.h>
#include <vlr-util/util.data_adaptor.MultiSZ.h>
#include <vlr-util/util.Result.h>
#include <vlr-util/ModuleContext.Compilation.h>

#include "RegistryAccess_Wow64KeyAccessOption.h"

namespace vlr {

namespace win32 {

template <typename TChar, typename... Args>
decltype(auto) MakeRegistryPath(std::basic_string_view<TChar> svPathPrefix, std::basic_string_view<TChar> svPathComponent, Args&&... args)
{
	// Note, possible rainy-day optimization:
	// Convert to use fmt::join (https://fmt.dev/latest/api.html), by constexpr conversion of args to std::array,
	// and transformation to range by trimming string_view elements.

	constexpr auto fGetFormatString = [] {
		if constexpr (std::is_same_v<TChar, wchar_t>)
		{
			return L"{}\\{}";
		}
		else
		{
			return "{}\\{}";
		}
	};
	constexpr auto svzChars_PathSeparators = strings::DelimitersSpec<TChar>::GetChars_PathSeparators();

	auto sPath = fmt::format(fGetFormatString(),
		strings::GetTrimmedStringView(svPathPrefix, svzChars_PathSeparators),
		strings::GetTrimmedStringView(svPathComponent, svzChars_PathSeparators));
	if constexpr (sizeof...(args) == 0)
	{
		return sPath;
	}
	else
	{
		return MakeRegistryPath(sPath, args);
	}
}

class CRegistryAccess
{
public:
	using QWORD = unsigned __int64;

protected:
	HKEY m_hBaseKey = {};

	static constexpr size_t m_nMaxIterationCountForRead_Default = 2;
	size_t m_nMaxIterationCountForRead = m_nMaxIterationCountForRead_Default;
	static constexpr size_t m_OnReadValue_nDefaultBufferSize = 1024;

	RegistryAccess::SEWow64KeyAccessOption m_eWow64KeyAccessOption;

	virtual HKEY getBaseKey() const
	{
		return m_hBaseKey;
	}

public:
	inline SResult SetWow64KeyAccessOption(RegistryAccess::SEWow64KeyAccessOption eWow64KeyAccessOption)
	{
		m_eWow64KeyAccessOption = eWow64KeyAccessOption;
		return SResult::Success;
	}

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

	SResult ReadValueInfo(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		DWORD& dwType_Result,
		DWORD& dwSize_Result) const;

	SResult ReadValueBase(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		DWORD& dwType_Result, 
		std::vector<BYTE>& arrData) const;
	SResult WriteValueBase(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		const DWORD& dwType,
		cpp::span<const BYTE> spanData) const;

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
		cpp::span<const BYTE> spanData) const;

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
		cpp::span<const BYTE> spanData,
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
		cpp::span<const BYTE> spanData,
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
		cpp::span<const BYTE> spanData,
		std::string& saValue) const;
	SResult convertRegDataToValueDirect_String_NativeType(
		const DWORD& dwType,
		cpp::span<const BYTE> spanData,
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
		cpp::span<const BYTE> spanData,
		DWORD& dwValue) const;
	SResult convertValueToRegData_DWORD(
		const DWORD& dwValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertRegDataToValue_QWORD(
		const DWORD& dwType,
		cpp::span<const BYTE> spanData,
		QWORD& qwValue) const;
	SResult convertValueToRegData_QWORD(
		const QWORD& qwValue,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertRegDataToValue_MultiSz(
		const DWORD& dwType,
		cpp::span<const BYTE> spanData,
		std::vector<vlr::tstring>& arrValueCollection) const;
	SResult convertValueToRegData_MultiSz(
		const std::vector<vlr::tstring>& arrValueCollection,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;
	SResult convertRegDataToValue_Binary(
		const DWORD& dwType,
		cpp::span<const BYTE> spanData,
		std::vector<BYTE>& arrBinaryData) const;
	SResult convertRegDataToValue_Binary_AsFallback(
		const DWORD& dwType,
		cpp::span<const BYTE> spanData,
		std::vector<BYTE>& arrBinaryData) const;
	SResult convertValueToRegData_Binary(
		cpp::span<const BYTE> spanData,
		DWORD& dwType,
		std::vector<BYTE>& arrData) const;

	struct EnumValueData
	{
		DWORD m_dwIndex{};
		vlr::tstring_view m_svName;
		DWORD m_dwType{};
		cpp::span<const BYTE> m_spanData;

		decltype(auto) withIndex(DWORD dwIndex)
		{
			m_dwIndex = dwIndex;
			return *this;
		}
		decltype(auto) withName(vlr::tstring_view svName)
		{
			m_svName = svName;
			return *this;
		}
		decltype(auto) withType(DWORD dwType)
		{
			m_dwType = dwType;
			return *this;
		}
		decltype(auto) withData(cpp::span<const BYTE> spanData)
		{
			m_spanData = spanData;
			return *this;
		}
	};
	using OnEnumValueData = std::function<SResult(const EnumValueData& oEnumValueData)>;

	SResult EnumAllValues(
		tzstring_view svzKeyName,
		const OnEnumValueData& fOnEnumValueData) const;

	// Note: This does data copies and allocations, so prefer enum for search/speed
	struct ValueMapEntry
	{
		cpp::tstring m_sValueName;
		DWORD m_dwType{};
		cpp::shared_ptr<cpp::tstring> m_spValue_SZ;
		cpp::shared_ptr<DWORD> m_spValue_DWORD;
		cpp::shared_ptr<QWORD> m_spValue_QWORD;
		cpp::shared_ptr<std::vector<cpp::tstring>> m_spValue_MultiSZ;
		cpp::shared_ptr<std::vector<BYTE>> m_spValue_Binary;
		cpp::shared_ptr<std::vector<BYTE>> m_spValue_TypeUnhandled;
	};

	SResult populateValueMapEntryFromEnumValueData(
		const EnumValueData& oEnumValueData,
		ValueMapEntry& oValueMapEntry) const;

	SResult RealAllValuesIntoMap(
		tzstring_view svzKeyName,
		std::unordered_map<vlr::tstring, ValueMapEntry>& mapNameToValue) const;

	// This is a method which can be used to read a value without exposing the name of the value which is being read.
	// Since registry access calls can be audited, reading a value by name exposes the name. Instead of this, we can 
	// read all values, and filter for the value(s) we are interested in.

	SResult ReadValueObfuscated(
		tzstring_view svzKeyName,
		tzstring_view svzValueName,
		ValueMapEntry& oValueMapEntry);
	SResult ReadValuesObfuscated(
		tzstring_view svzKeyName,
		const std::vector<cpp::tstring>& arrValueNames,
		std::vector<ValueMapEntry>& arrValueMapEntryCollection);

	struct EnumSubkeyData
	{
		DWORD m_dwIndex{};
		vlr::tstring_view m_svName;
		vlr::tstring_view m_svClass;
		FILETIME m_ftLastWriteTime{};

		decltype(auto) withIndex(DWORD dwIndex)
		{
			m_dwIndex = dwIndex;
			return *this;
		}
		decltype(auto) withName(vlr::tstring_view svName)
		{
			m_svName = svName;
			return *this;
		}
		decltype(auto) withClass(vlr::tstring_view svClass)
		{
			m_svClass = svClass;
			return *this;
		}
		decltype(auto) withLastWriteTime(const FILETIME& ftLastWriteTime)
		{
			m_ftLastWriteTime = ftLastWriteTime;
			return *this;
		}
	};
	using OnEnumSubkeyData = std::function<SResult(const EnumSubkeyData& oEnumSubkeyData)>;

	SResult EnumAllSubkeys(
		tzstring_view svzKeyName,
		const OnEnumSubkeyData& fOnEnumSubkeyData) const;

	SResult ReadAllSubkeysIntoVector(
		tzstring_view svzKeyName,
		std::vector<cpp::tstring>& arrSubkeyNames);

protected:
	SResult openKey(
		tzstring_view svzKeyName,
		DWORD dwAccessMask,
		HKEY& hKey_Result) const;
	DWORD getWow64RedirectionKeyAccessMask() const;

public:
	CRegistryAccess() = default;
	CRegistryAccess(HKEY hBaseKey)
		: m_hBaseKey{ hBaseKey }
	{}
};

} // namespace win32

} // namespace vlr
