#pragma once

#include <optional>

#include <boost/iterator/iterator_facade.hpp>

#include <vlr-util/util.includes.h>

#include <vlr-util-win32/registry.iterator_RegEnumValue.h>

VLR_NAMESPACE_BEGIN( vlr )

VLR_NAMESPACE_BEGIN( win32 )

VLR_NAMESPACE_BEGIN( registry )

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

VLR_NAMESPACE_END //( registry )

VLR_NAMESPACE_END //( win32 )

VLR_NAMESPACE_END //( vlr )
