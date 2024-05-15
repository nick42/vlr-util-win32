#pragma once

#include <vlr-util/util.includes.h>
#include <vlr-util/enums.FormatEnum.h>
#include <vlr-util/enums.SmartEnum.h>

namespace vlr {

namespace win32 {

namespace RegistryAccess {

enum Wow64KeyAccessOption
{
	Unknown,
	UseDefault,
	UseExplicit32bit,
	UseExplicit64bit,
	UsePlatformNative,
};
constexpr auto MAX_VALUE = UsePlatformNative;

} // namespace RegistryAccess

} // namespace win32

namespace enums {

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
			VLR_ON_CASE_RETURN_STRING_OF_VALUE(win32::RegistryAccess::UsePlatformNative);

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

} // namespace enums

namespace win32 {

namespace RegistryAccess {

using SEWow64KeyAccessOption = enums::SmartEnum<Wow64KeyAccessOption, Unknown, enums::CFormatEnum<Wow64KeyAccessOption>, enums::RangeCheckForEnum<Wow64KeyAccessOption>>;

} // namespace RegistryAccess

} // namespace win32

} // namespace vlr
