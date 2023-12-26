#include "pch.h"
#include "structure.ACL.h"

#include <vlr-util/util.includes.h>
#include <vlr-util/logging.LogMessage.h>

#include <vlr-util-win32/structure.ACE.h>

namespace vlr {

namespace win32 {

namespace structure {

bool CAccessorFor_ACL::HasMetaValue_EntirelyInherited() const
{
	static constexpr auto _tFailureValue = false;

	for (decltype(AceCount) nAceIndex = 0; nAceIndex < AceCount; ++nAceIndex)
	{
		LPVOID pvAce = nullptr;
		auto bResult = ::GetAce( const_cast<ACL*>(static_cast<const ACL*>(this)), nAceIndex, &pvAce );
		VLR_ASSERT_NONZERO_OR_RETURN_FAILURE_VALUE( bResult );

		auto&& oACE_HEADER = vlr::win32::structure::MakeStructureAccessor( reinterpret_cast<const ACE_HEADER*>(pvAce) );

		if (!oACE_HEADER.HasFlag_Inherited())
		{
			return false;
		}
	}

	return true;
}

bool CAccessControlList::IsEffectivelyIdenticalTo( const CAccessControlList& oOther )
{
	auto fPred_Compare = []( const SPCAccessControlEntryBase& lhs, const SPCAccessControlEntryBase& rhs )
	{
		return lhs->IsIdentical( rhs.get() );
	};
	return std::equal(
		begin( m_oAccessControlEntryList ),
		end( m_oAccessControlEntryList ),
		begin( oOther.m_oAccessControlEntryList ),
		end( oOther.m_oAccessControlEntryList ),
		fPred_Compare );
	//auto&& lhsIter = m_oAccessControlEntryList.begin();
	//auto&& lhsEnd = m_oAccessControlEntryList.end();
	//auto&& rhsIter = oOther.m_oAccessControlEntryList.begin();
	//auto&& rhsEnd = oOther.m_oAccessControlEntryList.end();

	//do
	//{
	//	if (lhsIter == lhsEnd)
	//	{
	//		break;
	//	}
	//	if (rhsIter == rhsEnd)
	//	{
	//		break;
	//	}
	//	const auto& lhsACE = *(*lhsIter++);
	//	const auto& rhsACE = *(*rhsIter++);
	//	if (!lhsACE.IsIdentical( rhsACE ))
	//	{
	//		return false;
	//	}
	//} while (true);

	//if (lhsIter != lhsEnd)
	//{
	//	return false;
	//}
	//if (rhsIter != rhsEnd)
	//{
	//	return false;
	//}

	//return true;
}

HRESULT CAccessControlList::LogData( const logging::CMessageContext& oMessageContext ) const
{
	for (const auto& spAccessControlEntry : m_oAccessControlEntryList)
	{
		logging::LogMessage( oMessageContext,
			spAccessControlEntry->GetDisplayString_Description() );
	}

	return S_OK;
}

HRESULT CAccessControlList::Initialize( const ACL* pACL )
{
	static constexpr auto _tFailureValue = E_FAIL;

	auto&& oACL = MakeStructureAccessor( pACL );

	m_oAccessControlEntryList.reserve( oACL.AceCount );

	for (decltype(oACL.AceCount) nAceIndex = 0; nAceIndex < oACL.AceCount; ++nAceIndex)
	{
		LPVOID pvAce = nullptr;
		auto bResult = ::GetAce( const_cast<ACL*>(pACL), nAceIndex, &pvAce );
		VLR_ASSERT_NONZERO_OR_RETURN_FAILURE_VALUE( bResult );

		auto spStructure = MakeStructureSP_AccessControlEntry( pvAce );
		VLR_ASSERT_NONZERO_OR_RETURN_FAILURE_VALUE( spStructure );

		m_oAccessControlEntryList.push_back( spStructure );
	}

	return S_OK;
}

} // namespace structure

} // namespace win32

} // namespace vlr
