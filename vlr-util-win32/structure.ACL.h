#pragma once

#include <memory>
#include <list>
#include <vector>

#include <vlr-util/util.includes.h>
#include <vlr-util/logging.MessageContext.h>

#include <vlr-util-win32/structure.ACE.h>

namespace vlr {

namespace win32 {

namespace structure {

struct CAccessorFor_ACL
	: public ACL
{
	bool HasMetaValue_EntirelyInherited() const;

	CAccessorFor_ACL( const CAccessorFor_ACL& ) = delete;
};

inline decltype(auto) MakeStructureAccessor( ACL* pFindData )
{
	static_assert(sizeof( CAccessorFor_ACL ) == sizeof( ACL ));
	return *reinterpret_cast<CAccessorFor_ACL*>(pFindData);
}

inline decltype(auto) MakeStructureAccessor( const ACL* pFindData )
{
	static_assert(sizeof( CAccessorFor_ACL ) == sizeof( ACL ));
	return *reinterpret_cast<const CAccessorFor_ACL*>(pFindData);
}

class CAccessControlList
{
public:
	std::vector<SPCAccessControlEntryBase> m_oAccessControlEntryList;

public:
	bool IsEffectivelyIdenticalTo( const CAccessControlList& oOther );

public:
	HRESULT LogData( const logging::CMessageContext& oMessageContext ) const;

protected:
	HRESULT Initialize( const ACL* pACL );

public:
	CAccessControlList() = default;
	CAccessControlList( const ACL* pACL )
	{
		Initialize( pACL );
	}
	CAccessControlList( const CAccessControlList& ) = default;
};
using SPCAccessControlList = std::shared_ptr<CAccessControlList>;

} // namespace structure

} // namespace win32

} // namespace vlr
