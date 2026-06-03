#pragma once

#include <optional>
#include <iterator>

#include <vlr-util/util.includes.h>
#include <vlr-util/ActionOnDestruction.h>

#include <vlr-util-win32/registry.RegKey.h>

namespace vlr {

namespace win32 {

namespace registry {

struct RegEnumKeyResult
	: public CRegKey
{
	DWORD m_dwIndex = 0;
	std::wstring m_wsName;
	std::wstring m_wsClass;
	FILETIME m_oLastWriteTime = {};
};

class enum_RegKeys;

class iterator_RegEnumKey
{
	friend enum_RegKeys;

public:
	using difference_type = std::ptrdiff_t;
	using value_type = RegEnumKeyResult;
	using pointer = const RegEnumKeyResult*;
	using reference = const RegEnumKeyResult&;
	using iterator_category = std::forward_iterator_tag;

protected:
	HKEY m_hParentKey = {};
	std::optional<DWORD> m_odwNextIndex;
	cpp::shared_ptr<RegEnumKeyResult> m_spCurrentResult;
	std::optional<DWORD> m_odwLastError;

protected:
	inline bool HaveValidItem() const
	{
		return true
			&& m_odwNextIndex.has_value()
			&& m_spCurrentResult
			;
	}
	inline bool HaveValidIndexForIteration() const
	{
		return true
			&& m_odwNextIndex.has_value()
			;
	}
	HRESULT OnAdaptorMethod_increment();

public:
	inline const auto& GetLastError() const
	{
		return m_odwLastError;
	}

public:
	reference operator*() const
	{
		if (!HaveValidItem())
		{
			throw std::exception{ "Invalid iterator state" };
		}
		return *m_spCurrentResult;
	}

	pointer operator->() const
	{
		if (!HaveValidItem())
		{
			throw std::exception{ "Invalid iterator state" };
		}
		return m_spCurrentResult.get();
	}

	iterator_RegEnumKey& operator++()
	{
		if (!HaveValidIndexForIteration())
		{
			throw std::exception{ "Invalid iterator state" };
		}
		OnAdaptorMethod_increment();
		return *this;
	}

	iterator_RegEnumKey operator++(int)
	{
		iterator_RegEnumKey temp = *this;
		++(*this);
		return temp;
	}

	bool operator==(const iterator_RegEnumKey& other) const
	{
		bool bInvalidIter_this = (!HaveValidIndexForIteration());
		bool bInvalidIter_other = (!other.HaveValidIndexForIteration());

		// If either is invalid, then they are equal IFF both are invalid
		if (bInvalidIter_this || bInvalidIter_other)
		{
			return bInvalidIter_this && bInvalidIter_other;
		}

		// Both valid; any appliable checks for validity
		return true
			&& (m_hParentKey == other.m_hParentKey)
			&& (m_odwNextIndex.value() == other.m_odwNextIndex.value())
			;
	}

	bool operator!=(const iterator_RegEnumKey& other) const
	{
		return !(*this == other);
	}

public:
	constexpr iterator_RegEnumKey(
		HKEY hParentKey)
		: m_hParentKey{ hParentKey }
	{
	}
	iterator_RegEnumKey(
		HKEY hParentKey,
		DWORD dwIndex)
		: m_hParentKey{ hParentKey }
		, m_odwNextIndex{ dwIndex }
	{
		increment();
	}
	~iterator_RegEnumKey() = default;

private:
	void increment()
	{
		if (!HaveValidIndexForIteration())
		{
			throw std::exception{ "Invalid iterator state" };
		}
		OnAdaptorMethod_increment();
	}
};

HRESULT iterator_RegEnumKey::OnAdaptorMethod_increment()
{
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(m_hParentKey);
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(m_odwNextIndex.has_value());

	DWORD dwValueNameLength = 256;
	DWORD dwClassNameLength = 256;

	auto spCurrentResult = cpp::make_shared<RegEnumKeyResult>();
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(spCurrentResult);

	spCurrentResult->m_dwIndex = m_odwNextIndex.value();

	spCurrentResult->m_wsName.resize(dwValueNameLength);
	spCurrentResult->m_wsClass.resize(dwClassNameLength);

	do
	{
		auto lStatus = ::RegEnumKeyExW(
			m_hParentKey,
			m_odwNextIndex.value(),
			spCurrentResult->m_wsName.data(),
			&dwValueNameLength,
			NULL,
			spCurrentResult->m_wsClass.data(),
			&dwClassNameLength,
			&spCurrentResult->m_oLastWriteTime);
		if (lStatus == ERROR_SUCCESS)
		{
			spCurrentResult->m_wsName.resize(dwValueNameLength);
			spCurrentResult->m_wsClass.resize(dwClassNameLength);
			m_spCurrentResult = spCurrentResult;
			m_odwNextIndex = ++m_odwNextIndex.value();

			return S_OK;
		}
		if (lStatus == ERROR_NO_MORE_ITEMS)
		{
			m_odwNextIndex = {};
			spCurrentResult = {};
			m_odwLastError = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);

			return S_FALSE;
		}
		// Note: May also be ERROR_MORE_DATA, if the name/class is too long for the buffer; but we set the 
		// buffer to 256, which is the max length for these values, so this should not occur normally. Handle 
		// as an error if it does occur, since it indicates an unexpected condition (e.g. registry corruption).

		// No other handled cases; return error
		m_odwLastError = HRESULT_FROM_WIN32(lStatus);
		return m_odwLastError.value();
	} while (true);

	VLR_HANDLE_ASSERTION_FAILURE__AND_RETURN_EXPRESSION(E_UNEXPECTED);
}

} // namespace registry

} // namespace win32

} // namespace vlr
