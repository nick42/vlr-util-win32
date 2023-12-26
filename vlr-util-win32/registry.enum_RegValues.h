#pragma once

#include <optional>

#include <boost/iterator/iterator_facade.hpp>

#include <vlr-util/util.includes.h>

#include <vlr-util-win32/registry.iterator_RegEnumValue.h>

namespace vlr {

namespace win32 {

namespace registry {

class enum_RegValues
{
protected:
	HKEY m_hKey = {};

public:
	inline auto begin() const
	{
		return iterator_RegEnumValue{ m_hKey, 0 };
	}
	inline auto end() const
	{
		return iterator_RegEnumValue{ m_hKey };
	}

public:
	constexpr enum_RegValues( HKEY hKey )
		: m_hKey{ hKey }
	{}
};

} // namespace registry

} // namespace win32

} // namespace vlr
