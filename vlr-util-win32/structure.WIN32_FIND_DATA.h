#pragma once

#include <vlr-util/util.includes.h>
#include <vlr-util/util.IsBitSet.h>

VLR_NAMESPACE_BEGIN( vlr )

VLR_NAMESPACE_BEGIN( win32 )

VLR_NAMESPACE_BEGIN( structure )

struct CAccessorFor_WIN32_FIND_DATA
	: public WIN32_FIND_DATA
{
	constexpr bool HasAttributeFlag( DWORD dwFlag ) const
	{
		return util::IsBitSet( this->dwFileAttributes, dwFlag );
	}
	constexpr bool HasAttribute_Compressed() const
	{
		return HasAttributeFlag( FILE_ATTRIBUTE_COMPRESSED );
	}
	constexpr bool HasAttribute_Directory() const
	{
		return HasAttributeFlag( FILE_ATTRIBUTE_DIRECTORY );
	}
	constexpr bool HasAttribute_Offline() const
	{
		return HasAttributeFlag( FILE_ATTRIBUTE_OFFLINE );
	}
	constexpr bool HasAttribute_ReparsePoint() const
	{
		return HasAttributeFlag( FILE_ATTRIBUTE_REPARSE_POINT );
	}

	CAccessorFor_WIN32_FIND_DATA( const CAccessorFor_WIN32_FIND_DATA& ) = delete;
};

inline decltype(auto) MakeStructureAccessor( WIN32_FIND_DATA* pFindData )
{
	static_assert(sizeof( CAccessorFor_WIN32_FIND_DATA ) == sizeof( WIN32_FIND_DATA ));
	return *reinterpret_cast<CAccessorFor_WIN32_FIND_DATA*>(pFindData);
}

inline decltype(auto) MakeStructureAccessor( const WIN32_FIND_DATA* pFindData )
{
	static_assert(sizeof( CAccessorFor_WIN32_FIND_DATA ) == sizeof( WIN32_FIND_DATA ));
	return *reinterpret_cast<const CAccessorFor_WIN32_FIND_DATA*>(pFindData);
}

VLR_NAMESPACE_END //( structure )

VLR_NAMESPACE_END //( win32 )

VLR_NAMESPACE_END //( vlr )
