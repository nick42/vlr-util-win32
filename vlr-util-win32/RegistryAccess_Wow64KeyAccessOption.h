#pragma once

#include <vlr-util/util.includes.h>
#include <vlr-util/enums.FormatEnum.h>
#include <vlr-util/enums.SmartEnum.h>

VLR_NAMESPACE_BEGIN(vlr)

VLR_NAMESPACE_BEGIN(win32)

VLR_NAMESPACE_BEGIN(RegistryAccess)

enum Wow64KeyAccessOption
{
	Unknown,
	UseDefault,
	UseExplicit32bit,
	UseExplicit64bit,
};
constexpr auto MAX_VALUE = UseExplicit64bit;

VLR_NAMESPACE_END //(RegistryAccess)

VLR_NAMESPACE_END //(win32)

VLR_NAMESPACE_BEGIN(enums)

template<>
class CFormatEnum<win32::RegistryAccess::Wow64KeyAccessOption>
{
	using TEnum = win32::RegistryAccess::Wow64KeyAccessOption;
	using this_type = CFormatEnum<TEnum>;
	using base_type = CFormatEnumBase<DWORD_PTR>;

public:
	[[nodiscard]]
	static inline auto FormatValue(TEnum eValue)
		-> vlr::tstring
	{
		switch (eValue)
		{
			VLR_ON_CASE_RETURN_STRING_OF_VALUE(win32::RegistryAccess::Unknown);
			VLR_ON_CASE_RETURN_STRING_OF_VALUE(win32::RegistryAccess::UseDefault);
			VLR_ON_CASE_RETURN_STRING_OF_VALUE(win32::RegistryAccess::UseExplicit32bit);
			VLR_ON_CASE_RETURN_STRING_OF_VALUE(win32::RegistryAccess::UseExplicit64bit);

		default:
			return base_type::FormatAsNumber(eValue);
		}
	}
};

template<>
class RangeCheckForEnum<win32::RegistryAccess::Wow64KeyAccessOption>
	: CRangeInfoSequential_DWORD_PTR<win32::RegistryAccess::MAX_VALUE>
{
	using TBaseEnum = win32::RegistryAccess::Wow64KeyAccessOption;
	using ThisType = RangeCheckForEnum<TBaseEnum>;
	using TParentType = CRangeInfoSequential_DWORD_PTR<win32::RegistryAccess::MAX_VALUE>;

public:
	using TParentType::IsValueInRange;
	//static bool IsValueInRange(DWORD_PTR dwValue)
	//{
	//	return ThisType::IsValueInRange(DWORD_PTR dwValue);
	//}
	static TBaseEnum CheckedEnumCast(DWORD_PTR dwValue)
	{
		if (IsValueInRange(dwValue))
		{
			return (TBaseEnum)dwValue;
		}
		else
		{
			return (TBaseEnum)0;
		}
	}
};

VLR_NAMESPACE_END //(enums)

VLR_NAMESPACE_BEGIN(win32)

VLR_NAMESPACE_BEGIN(RegistryAccess)

using SEWow64KeyAccessOption = enums::SmartEnum<Wow64KeyAccessOption, Unknown, enums::CFormatEnum<Wow64KeyAccessOption>, enums::RangeCheckForEnum<Wow64KeyAccessOption>>;

VLR_NAMESPACE_END //(RegistryAccess)

VLR_NAMESPACE_END //(win32)

VLR_NAMESPACE_END //(vlr)
