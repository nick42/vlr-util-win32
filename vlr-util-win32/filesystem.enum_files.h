#pragma once

#include <optional>
#include <iterator>

#include <vlr-util/util.includes.h>
#include <vlr-util/ActionOnDestruction.h>

namespace vlr {

namespace win32 {

namespace filesystem {

class enum_files;

class iterator_FindNextFile
{
	friend enum_files;

public:
	using difference_type = std::ptrdiff_t;
	using value_type = WIN32_FIND_DATA;
	using pointer = const WIN32_FIND_DATA*;
	using reference = const WIN32_FIND_DATA*;
	using iterator_category = std::forward_iterator_tag;

protected:
	struct RefCountedDataBlock
	{
		std::optional<HANDLE> m_ohFindHandle;

		RefCountedDataBlock(HANDLE hFindHandle)
			: m_ohFindHandle{ hFindHandle }
		{
		}
	};
	std::shared_ptr<RefCountedDataBlock> m_spRefCountedDataBlock;
	std::shared_ptr<BYTE[]> m_spResultDataBuffer;
	std::optional<DWORD> m_odwLastError;
	bool m_bSkipPseudoDirEntries = false;

protected:
	HRESULT OnAdaptorMethod_increment();
	static HRESULT OnDestroy_FindClose(RefCountedDataBlock* pRefCountedDataBlock);

	inline bool ShouldSkipEntry(const WIN32_FIND_DATA* pFindData) const
	{
		if (!pFindData)
			return true;

		if (m_bSkipPseudoDirEntries)
		{
			auto wsFileName = std::wstring(pFindData->cFileName);
			if (wsFileName == _T(".") || wsFileName == _T(".."))
			{
				return true;
			}
		}

		return false;
	}

public:
	inline const auto& GetLastError() const
	{
		return m_odwLastError;
	}

public:
	pointer operator*() const
	{
		if (!m_spResultDataBuffer)
		{
			throw std::exception{ "Invalid iterator state" };
		}
		return reinterpret_cast<const WIN32_FIND_DATA*>(m_spResultDataBuffer.get());
	}

	pointer operator->() const
	{
		return operator*();
	}

	iterator_FindNextFile& operator++()
	{
		if (!m_spRefCountedDataBlock || !m_spRefCountedDataBlock->m_ohFindHandle.has_value())
		{
			throw std::exception{ "Invalid iterator state" };
		}
		OnAdaptorMethod_increment();
		return *this;
	}

	iterator_FindNextFile operator++(int)
	{
		iterator_FindNextFile temp = *this;
		++(*this);
		return temp;
	}

	bool operator==(const iterator_FindNextFile& other) const
	{
		bool bInvalidIter_this = (!m_spRefCountedDataBlock || !m_spRefCountedDataBlock->m_ohFindHandle.has_value());
		bool bInvalidIter_other = (!other.m_spRefCountedDataBlock || !other.m_spRefCountedDataBlock->m_ohFindHandle.has_value());

		// If either is invalid, then they are equal IFF both are invalid
		if (bInvalidIter_this || bInvalidIter_other)
		{
			return bInvalidIter_this && bInvalidIter_other;
		}

		// Both valid; any appliable checks for validity
		return true
			&& (m_spRefCountedDataBlock->m_ohFindHandle.value() == other.m_spRefCountedDataBlock->m_ohFindHandle.value())
			;
	}

	bool operator!=(const iterator_FindNextFile& other) const
	{
		return !(*this == other);
	}

public:
	constexpr iterator_FindNextFile() = default;
	~iterator_FindNextFile() = default;
};

HRESULT iterator_FindNextFile::OnAdaptorMethod_increment()
{
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(m_spRefCountedDataBlock);
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(m_spRefCountedDataBlock->m_ohFindHandle.has_value());

	do
	{
		// Copy buffer and clear internal to do operation; will copy back on success
		auto spResultDataBuffer = m_spResultDataBuffer;
		m_spResultDataBuffer = {};

		BOOL bSuccess = ::FindNextFile(
			m_spRefCountedDataBlock->m_ohFindHandle.value(),
			reinterpret_cast<WIN32_FIND_DATA*>(spResultDataBuffer.get()));
		if (bSuccess)
		{
			auto pFindData = reinterpret_cast<const WIN32_FIND_DATA*>(spResultDataBuffer.get());
			
			if (!ShouldSkipEntry(pFindData))
			{
				m_spResultDataBuffer = spResultDataBuffer;
				return S_OK;
			}
			// Otherwise, loop and get next file
			continue;
		}

		m_odwLastError = ::GetLastError();
		if (m_odwLastError.value() != ERROR_NO_MORE_FILES)
		{
			return E_FAIL;
		}

		// Done with the iteration; clear the handle holder (will close here if last reference)
		m_spRefCountedDataBlock = {};

		return S_OK;
	} while (true);
}

HRESULT iterator_FindNextFile::OnDestroy_FindClose(RefCountedDataBlock* pRefCountedDataBlock)
{
	if (!pRefCountedDataBlock)
	{
		return S_FALSE;
	}
	if (!pRefCountedDataBlock->m_ohFindHandle.has_value())
	{
		return S_FALSE;
	}

	auto oOnDestroy_DeleteBlock = MakeActionOnDestruction([&] { delete pRefCountedDataBlock; });

	BOOL bSuccess;

	bSuccess = ::FindClose(
		pRefCountedDataBlock->m_ohFindHandle.value());
	VLR_ASSERT_NONZERO_OR_RETURN_EUNEXPECTED(bSuccess);

	return S_OK;
}

class enum_files
{
public:
	vlr::tstring m_sSearchString;
	FINDEX_SEARCH_OPS m_dwSearchOps = FindExSearchNameMatch;
	DWORD m_dwAdditionalFlags = 0;
	bool m_bSkipPseudoDirEntries = true;

protected:
	HRESULT OnBegin(iterator_FindNextFile& iter) const
	{
		auto spResultDataBuffer = std::shared_ptr<BYTE[]>{ new BYTE[sizeof(WIN32_FIND_DATA)] };

		auto hFindHandle = ::FindFirstFileEx(
			vlr::tzstring_view{ m_sSearchString },
			FindExInfoBasic,
			spResultDataBuffer.get(),
			m_dwSearchOps,
			nullptr,
			m_dwAdditionalFlags);
		if (hFindHandle == INVALID_HANDLE_VALUE)
		{
			iter.m_odwLastError = ::GetLastError();
			return S_FALSE;
		}

		auto spRefCountedDataBlock = std::shared_ptr<iterator_FindNextFile::RefCountedDataBlock>{ new iterator_FindNextFile::RefCountedDataBlock(hFindHandle), &iterator_FindNextFile::OnDestroy_FindClose };
		VLR_ASSERT_ALLOCATED_OR_RETURN_STANDARD_ERROR(spRefCountedDataBlock);
		iter.m_spRefCountedDataBlock = spRefCountedDataBlock;
		iter.m_spResultDataBuffer = spResultDataBuffer;
		iter.m_bSkipPseudoDirEntries = m_bSkipPseudoDirEntries;

		// Check if the first entry from FindFirstFileEx should be skipped
		auto pFindData = reinterpret_cast<const WIN32_FIND_DATA*>(spResultDataBuffer.get());
		if (iter.ShouldSkipEntry(pFindData))
		{
			// Need to manually advance to the first non-skipped entry
			do
			{
				BOOL bSuccess = ::FindNextFile(
					iter.m_spRefCountedDataBlock->m_ohFindHandle.value(),
					reinterpret_cast<WIN32_FIND_DATA*>(spResultDataBuffer.get()));
				if (!bSuccess)
				{
					DWORD dwError = ::GetLastError();
					if (dwError == ERROR_NO_MORE_FILES)
					{
						// No more files - mark as done
						iter.m_spRefCountedDataBlock = {};
						return S_OK;
					}
					// Real error
					iter.m_odwLastError = dwError;
					return E_FAIL;
				}
				
				pFindData = reinterpret_cast<const WIN32_FIND_DATA*>(spResultDataBuffer.get());
				if (!iter.ShouldSkipEntry(pFindData))
				{
					// Found an entry that shouldn't be skipped
					iter.m_spResultDataBuffer = spResultDataBuffer;
					return S_OK;
				}
				// Continue looping to find next entry
			} while (true);
		}

		return S_OK;
	}

public:
	inline auto begin() const
	{
		auto iter = iterator_FindNextFile{};
		OnBegin(iter);
		return iter;
	}
	inline auto end() const
	{
		return iterator_FindNextFile{};
	}
};

} // namespace filesystem

} // namespace win32

} // namespace vlr
