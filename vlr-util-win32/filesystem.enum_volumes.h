#pragma once

#include <optional>
#include <iterator>

#include <vlr-util/util.includes.h>
#include <vlr-util/ActionOnDestruction.h>

namespace vlr {

namespace win32 {

namespace filesystem {

class iterator_volumes
{
public:
	using difference_type = std::ptrdiff_t;
	using value_type = vlr::tstring;
	using pointer = const vlr::tstring*;
	using reference = const vlr::tstring&;
	using iterator_category = std::forward_iterator_tag;

protected:
	struct RefCountedDataBlock
	{
		std::optional<HANDLE> m_ohFindVolume;
	};
	std::shared_ptr<RefCountedDataBlock> m_spRefCountedDataBlock;
	std::optional<vlr::tstring> m_osCurrentResult;
	std::optional<DWORD> m_odwLastError;

protected:
	HRESULT OnIterationBegin();
	HRESULT OnAdaptorMethod_increment();
	static HRESULT OnDestroy_CloseFindVolume( RefCountedDataBlock* pRefCountedDataBlock );

public:
	inline auto& withBeginIteration()
	{
		OnIterationBegin();
		return *this;
	}
	constexpr const auto& GetLastError() const noexcept
	{
		return m_odwLastError;
	}

public:
	reference operator*() const
	{
		if (!m_osCurrentResult.has_value())
		{
			throw std::exception{ "Invalid iterator state" };
		}
		return m_osCurrentResult.value();
	}

	pointer operator->() const
	{
		if (!m_osCurrentResult.has_value())
		{
			throw std::exception{ "Invalid iterator state" };
		}
		return &m_osCurrentResult.value();
	}

	iterator_volumes& operator++()
	{
		if (!m_spRefCountedDataBlock || !m_spRefCountedDataBlock->m_ohFindVolume.has_value())
		{
			throw std::exception{ "Invalid iterator state" };
		}
		OnAdaptorMethod_increment();
		return *this;
	}

	iterator_volumes operator++(int)
	{
		iterator_volumes temp = *this;
		++(*this);
		return temp;
	}

	bool operator==(const iterator_volumes& other) const
	{
		bool bInvalidIter_this = (!m_spRefCountedDataBlock || !m_spRefCountedDataBlock->m_ohFindVolume.has_value());
		bool bInvalidIter_other = (!other.m_spRefCountedDataBlock || !other.m_spRefCountedDataBlock->m_ohFindVolume.has_value());

		// If either is invalid, then they are equal IFF both are invalid
		if (bInvalidIter_this || bInvalidIter_other)
		{
			return bInvalidIter_this && bInvalidIter_other;
		}

		// Both valid; any appliable checks for validity
		return true
			&& (m_spRefCountedDataBlock->m_ohFindVolume.value() == other.m_spRefCountedDataBlock->m_ohFindVolume.value())
			;
	}

	bool operator!=(const iterator_volumes& other) const
	{
		return !(*this == other);
	}

public:
	constexpr iterator_volumes() = default;
	~iterator_volumes() = default;
};

HRESULT iterator_volumes::OnIterationBegin()
{
	vlr::tstring sValue;
	sValue.resize( MAX_PATH );
	auto hFindVolume = ::FindFirstVolume(
		sValue.data(),
		MAX_PATH );
	if (hFindVolume == INVALID_HANDLE_VALUE)
	{
		m_odwLastError = ::GetLastError();
		return E_UNEXPECTED;
	}

	m_spRefCountedDataBlock = std::shared_ptr<RefCountedDataBlock>{ new RefCountedDataBlock, &iterator_volumes::OnDestroy_CloseFindVolume };
	VLR_ASSERT_ALLOCATED_OR_RETURN_STANDARD_ERROR( m_spRefCountedDataBlock );

	m_spRefCountedDataBlock->m_ohFindVolume = hFindVolume;
	// Note: string length will be longer than actual value; need to explicitly truncate at NULL terminator
	m_osCurrentResult = vlr::tstring{ sValue.c_str() };

	return S_OK;
}

HRESULT iterator_volumes::OnAdaptorMethod_increment()
{
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED( m_spRefCountedDataBlock );
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED( m_spRefCountedDataBlock->m_ohFindVolume.has_value() );

	m_osCurrentResult = {};

	BOOL bSuccess;

	vlr::tstring sValue;
	sValue.resize( MAX_PATH );
	bSuccess = ::FindNextVolume(
		m_spRefCountedDataBlock->m_ohFindVolume.value(),
		sValue.data(),
		MAX_PATH );
	if (bSuccess)
	{
		// Note: string length will be longer than actual value; need to explicitly truncate at NULL terminator
		m_osCurrentResult = vlr::tstring{ sValue.c_str() };
		return S_OK;
	}

	m_odwLastError = ::GetLastError();
	if (m_odwLastError.value() != ERROR_NO_MORE_FILES)
	{
		return E_FAIL;
	}

	// Done with the iteration; clear the handle holder (will close here if last reference)
	m_spRefCountedDataBlock = {};

	return S_OK;
}

HRESULT iterator_volumes::OnDestroy_CloseFindVolume( RefCountedDataBlock* pRefCountedDataBlock )
{
	if (!pRefCountedDataBlock)
	{
		return S_FALSE;
	}
	if (!pRefCountedDataBlock->m_ohFindVolume.has_value())
	{
		return S_FALSE;
	}

	auto oOnDestroy_DeleteBlock = MakeActionOnDestruction([&] { delete pRefCountedDataBlock; });

	BOOL bSuccess;

	bSuccess = ::FindVolumeClose(
		pRefCountedDataBlock->m_ohFindVolume.value() );
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED( bSuccess );

	return S_OK;
}

struct enum_volumes
{
	inline auto begin() const
	{
		return iterator_volumes{}.withBeginIteration();
	}
	inline auto end() const
	{
		return iterator_volumes{};
	}
};

} // namespace filesystem

} // namespace win32

} // namespace vlr
