#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <vlr-util/util.includes.h>
#include <vlr-util/util.IsBitSet.h>
#include <vlr-util/util.std_aliases.h>

#include <vlr-util-win32/security.SIDs.h>

namespace vlr {

namespace win32 {

namespace structure {

struct CAccessorFor_ACE_HEADER
	: public ACE_HEADER
{
	inline bool HasFlag_Inherited() const
	{
		return util::IsBitSet( AceFlags, INHERITED_ACE );
	}

	CAccessorFor_ACE_HEADER( const CAccessorFor_ACE_HEADER& ) = delete;
};

inline decltype(auto) MakeStructureAccessor( ACE_HEADER* pFindData )
{
	static_assert(sizeof( CAccessorFor_ACE_HEADER ) == sizeof( ACE_HEADER ));
	return *reinterpret_cast<CAccessorFor_ACE_HEADER*>(pFindData);
}

inline decltype(auto) MakeStructureAccessor( const ACE_HEADER* pFindData )
{
	static_assert(sizeof( CAccessorFor_ACE_HEADER ) == sizeof( ACE_HEADER ));
	return *reinterpret_cast<const CAccessorFor_ACE_HEADER*>(pFindData);
}

class IAccessControlEntry
{
public:
	virtual BYTE GetAceType() const = 0;
	virtual BYTE GetAceFlags() const = 0;

	virtual vlr::tstring GetUniqueDescriptor() const = 0;
	virtual vlr::tstring GetDisplayString_Description() const = 0;

	virtual bool IsIdentical( const IAccessControlEntry* pOther ) const = 0;

public:
	virtual ~IAccessControlEntry() = default;
};
using SPIAccessControlEntry = std::shared_ptr<IAccessControlEntry>;

class CAccessControlEntryBase
	: public IAccessControlEntry
{
public:
	std::vector<BYTE> m_oEntryData;

public:
	template< typename TStructure, typename std::enable_if<std::is_same_v<std::remove_cv_t<TStructure>, ACE_HEADER>>::type* = nullptr >
	static auto& AccessMember_AceType( TStructure* pAceHeader )
	{
		return pAceHeader->AceType;
	}
	template< typename TStructure, typename std::enable_if<std::is_same_v<std::remove_cv_t<TStructure>, ACE_HEADER>>::type* = nullptr >
	static auto& AccessMember_AceFlags( TStructure* pAceHeader )
	{
		return pAceHeader->AceFlags;
	}
	template< typename TStructure, typename std::enable_if<std::is_same_v<std::remove_cv_t<TStructure>, ACE_HEADER>>::type* = nullptr >
	static auto& AccessMember_AceSize( TStructure* pAceHeader )
	{
		return pAceHeader->AceSize;
	}

public:
	inline auto BufferPtrAs_ACE_HEADER()
	{
		return reinterpret_cast<ACE_HEADER*>(m_oEntryData.data());
	}
	inline auto BufferPtrAs_ACE_HEADER() const
	{
		return reinterpret_cast<const ACE_HEADER*>(m_oEntryData.data());
	}
	inline auto& AceType()
	{
		return AccessMember_AceType( BufferPtrAs_ACE_HEADER() );
	}
	inline auto& AceType() const
	{
		return AccessMember_AceType( BufferPtrAs_ACE_HEADER() );
	}
	inline auto& AceFlags()
	{
		return AccessMember_AceFlags( BufferPtrAs_ACE_HEADER() );
	}
	inline auto& AceFlags() const
	{
		return AccessMember_AceFlags( BufferPtrAs_ACE_HEADER() );
	}
	inline auto& AceSize()
	{
		return AccessMember_AceSize( BufferPtrAs_ACE_HEADER() );
	}
	inline auto& AceSize() const
	{
		return AccessMember_AceSize( BufferPtrAs_ACE_HEADER() );
	}

public:
	bool IsIdentical_Hacky( const CAccessControlEntryBase& oOther ) const
	{
		return true
			&& (AceType() == oOther.AceType())
			&& (AceSize() == oOther.AceSize())
			&& (memcmp( m_oEntryData.data() + sizeof( ACE_HEADER ), oOther.m_oEntryData.data() + sizeof( ACE_HEADER ), AceSize() - sizeof( ACE_HEADER ) ) == 0)
			;
	}

public:
	virtual BYTE GetAceType() const
	{
		return AceType();
	}
	virtual BYTE GetAceFlags() const
	{
		return AceFlags();
	}

	virtual vlr::tstring GetUniqueDescriptor() const;
	virtual vlr::tstring GetDisplayString_Description() const;

	virtual bool IsIdentical( const IAccessControlEntry* pOther ) const
	{
		auto pOtherTyped = dynamic_cast<const CAccessControlEntryBase*>(pOther);
		if (!pOtherTyped)
		{
			return false;
		}

		// Default is hacky; type-specific subclass can do better
		return IsIdentical_Hacky( *pOtherTyped );
	}

public:
	virtual std::optional<vlr::win32::security::SIDs::CSidInfo> GetAsApplicable_SidInfo() const
	{
		return {};
	}
	virtual std::optional<vlr::tstring> GetAsApplicable_StringSid() const
	{
		return {};
	}
	virtual vlr::win32::security::SIDs::SPCSidNameLookupResult GetAsApplicable_SidNameLookupResult() const
	{
		return {};
	}
	virtual std::optional<DWORD> GetAsApplicable_AccessMask() const
	{
		return {};
	}

protected:
	HRESULT Initialize( LPVOID pvACE )
	{
		auto pACE_HEADER = reinterpret_cast<const ACE_HEADER*>(pvACE);
		auto nSize = AccessMember_AceSize( pACE_HEADER );
		m_oEntryData.resize( nSize );
		memcpy_s( m_oEntryData.data(), m_oEntryData.size(), pvACE, nSize );

		return S_OK;
	}

public:
	CAccessControlEntryBase( LPVOID pvACE )
	{
		Initialize( pvACE );
	}
	virtual ~CAccessControlEntryBase() = default;
};
using SPCAccessControlEntryBase = std::shared_ptr<CAccessControlEntryBase>;

class CAccessControlEntry_Unknown
	: public CAccessControlEntryBase
{
public:
	CAccessControlEntry_Unknown( LPVOID pvACE )
		: CAccessControlEntryBase{ pvACE }
	{}
};

class CAccessControlEntry_AccessAllowed
	: public CAccessControlEntryBase
{
public:
	template< typename TStructure, typename std::enable_if<std::is_same_v<std::remove_cv_t<TStructure>, ACCESS_ALLOWED_ACE>>::type* = nullptr >
	static auto& AccessMember_AccessMask( TStructure* pAceHeader )
	{
		return pAceHeader->Mask;
	}
	template< typename TStructure, typename std::enable_if<std::is_same_v<std::remove_cv_t<TStructure>, ACCESS_ALLOWED_ACE>>::type* = nullptr >
	static auto& AccessMember_SidStart( TStructure* pAceHeader )
	{
		return pAceHeader->SidStart;
	}

public:
	inline auto BufferPtrAs_ACCESS_ALLOWED_ACE()
	{
		return reinterpret_cast<ACCESS_ALLOWED_ACE*>(m_oEntryData.data());
	}
	inline auto BufferPtrAs_ACCESS_ALLOWED_ACE() const
	{
		return reinterpret_cast<const ACCESS_ALLOWED_ACE*>(m_oEntryData.data());
	}
	inline auto& AccessMask()
	{
		return AccessMember_AccessMask( BufferPtrAs_ACCESS_ALLOWED_ACE() );
	}
	inline auto& AccessMask() const
	{
		return AccessMember_AccessMask( BufferPtrAs_ACCESS_ALLOWED_ACE() );
	}
	inline auto SidPtr()
	{
		auto& SidStart = AccessMember_SidStart( BufferPtrAs_ACCESS_ALLOWED_ACE() );
		return reinterpret_cast<PSID>(&SidStart);
	}
	inline auto SidPtr() const
	{
		auto& SidStart = AccessMember_SidStart( BufferPtrAs_ACCESS_ALLOWED_ACE() );
		return reinterpret_cast<LPCVOID>(&SidStart);
	}

public:
	vlr::win32::security::SIDs::CSidInfo m_oSidInfo;

public:
	HRESULT PopulateStringSid( std::optional<vlr::tstring>& osStringSid ) const;
	HRESULT PopulateSidNameLookupResult( vlr::win32::security::SIDs::SPCSidNameLookupResult& spSidNameLookupResult_Result ) const;
	vlr::tstring GetStringSid();
	vlr::tstring GetStringSid() const;

public:
	virtual bool IsIdentical( const CAccessControlEntryBase& oOther ) const
	{
		return true
			&& (GetDisplayString_Description() == oOther.GetDisplayString_Description())
			;
	}

public:
	//virtual vlr::tstring GetUniqueDescriptor() const;
	virtual vlr::tstring GetDisplayString_Description() const;

public:
	virtual std::optional<vlr::win32::security::SIDs::CSidInfo> GetAsApplicable_SidInfo() const
	{
		return m_oSidInfo;
	}
	virtual std::optional<vlr::tstring> GetAsApplicable_StringSid() const
	{
		return GetStringSid();
	}
	virtual vlr::win32::security::SIDs::SPCSidNameLookupResult GetAsApplicable_SidNameLookupResult() const
	{
		return m_oSidInfo.m_spSidNameLookupResult;
	}
	virtual std::optional<DWORD> GetAsApplicable_AccessMask() const
	{
		return AccessMask();
	}

protected:
	HRESULT Initialize()
	{
		m_oSidInfo = vlr::win32::security::SIDs::CSidInfo{ SidPtr() };
		m_oSidInfo.PopulateData_StringSid();
		m_oSidInfo.PopulateData_SidNameLookupResult();

		return S_OK;
	}

public:
	CAccessControlEntry_AccessAllowed( LPVOID pvACE )
		: CAccessControlEntryBase{ pvACE }
	{
		Initialize();
	}
};

SPCAccessControlEntryBase MakeStructureSP_AccessControlEntry( LPVOID pvACE );

} // namespace structure

} // namespace win32

} // namespace vlr
