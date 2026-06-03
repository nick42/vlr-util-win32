#pragma once

#include <optional>
#include <iterator>

#include <vlr-util/util.includes.h>
#include <vlr-util/ActionOnDestruction.h>

#include <vlr-util-win32/registry.RegValue.h>

namespace vlr {

namespace win32 {

namespace registry {

struct RegEnumValueResult
	: public CRegValue
{
	DWORD m_dwIndex = 0;
};

class enum_RegValues;

class iterator_RegEnumValue
{
	friend enum_RegValues;

public:
	using difference_type = std::ptrdiff_t;
	using value_type = RegEnumValueResult;
	using pointer = const RegEnumValueResult*;
	using reference = const RegEnumValueResult&;
	using iterator_category = std::forward_iterator_tag;

protected:
	HKEY m_hParentKey = {};
	std::optional<DWORD> m_odwNextIndex;
	cpp::shared_ptr<RegEnumValueResult> m_spCurrentResult;
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

	iterator_RegEnumValue& operator++()
	{
		if (!HaveValidIndexForIteration())
		{
			throw std::exception{ "Invalid iterator state" };
		}
		OnAdaptorMethod_increment();
		return *this;
	}

	iterator_RegEnumValue operator++(int)
	{
		iterator_RegEnumValue temp = *this;
		++(*this);
		return temp;
	}

	bool operator==(const iterator_RegEnumValue& other) const
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

	bool operator!=(const iterator_RegEnumValue& other) const
	{
		return !(*this == other);
	}

public:
	constexpr iterator_RegEnumValue(
		HKEY hParentKey)
		: m_hParentKey{ hParentKey }
	{
	}
	iterator_RegEnumValue(
		HKEY hParentKey,
		DWORD dwIndex)
		: m_hParentKey{ hParentKey }
		, m_odwNextIndex{ dwIndex }
	{
		increment();
	}
	~iterator_RegEnumValue() = default;

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

HRESULT iterator_RegEnumValue::OnAdaptorMethod_increment()
{
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(m_hParentKey);
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(m_odwNextIndex.has_value());

	DWORD dwValueNameLength = 1024;
	DWORD dwValueLength = 2048;

	auto spCurrentResult = cpp::make_shared<RegEnumValueResult>();
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(spCurrentResult);

	spCurrentResult->m_dwIndex = m_odwNextIndex.value();

	spCurrentResult->m_wsName.resize(dwValueNameLength);
	spCurrentResult->m_oData.resize(dwValueLength);

	do
	{
		DWORD dwValueNameLength = static_cast<DWORD>(spCurrentResult->m_wsName.size());
		DWORD dwValueLength = static_cast<DWORD>(spCurrentResult->m_oData.size());

		auto lStatus = ::RegEnumValueW(
			m_hParentKey,
			m_odwNextIndex.value(),
			spCurrentResult->m_wsName.data(),
			&dwValueNameLength,
			NULL,
			&spCurrentResult->m_dwType,
			spCurrentResult->m_oData.data(),
			&dwValueLength);
		if (lStatus == ERROR_SUCCESS)
		{
			spCurrentResult->m_wsName.resize(dwValueNameLength);
			spCurrentResult->m_oData.resize(dwValueLength);
			m_spCurrentResult = spCurrentResult;
			m_odwNextIndex = ++m_odwNextIndex.value();

			return S_OK;
		}
		if (lStatus == ERROR_NO_MORE_ITEMS)
		{
			m_odwNextIndex = {};
			m_spCurrentResult = {};
			m_odwLastError = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);

			return S_FALSE;
		}
		if (lStatus == ERROR_MORE_DATA)
		{
			// Hard capping buffer sizes to 64k chars/bytes, as this is the max size of a registry value; 
			// if this is still not enough, then something is very wrong and we should just fail instead of looping indefinitely
			if ((dwValueNameLength >= 65536) || (dwValueLength >= 65536))
			{
				m_odwLastError = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
				return m_odwLastError.value();
			}

			// dwValueLength now holds required data bytes; name size is NOT updated
			spCurrentResult->m_wsName.resize(spCurrentResult->m_wsName.size() * 2);
			spCurrentResult->m_oData.resize(dwValueLength);

			continue;
		}

		// No other handled cases; return error
		m_odwLastError = HRESULT_FROM_WIN32(lStatus);
		return m_odwLastError.value();
	} while (true);

	VLR_HANDLE_ASSERTION_FAILURE__AND_RETURN_EXPRESSION(E_UNEXPECTED);
}

} // namespace registry

} // namespace win32

} // namespace vlr
