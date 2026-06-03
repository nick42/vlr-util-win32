#include "pch.h"

#include <vector>
#include <set>

#include "vlr-util/cpp_namespace.h"
#include "vlr-util/include.fmt.h"

#include "vlr-util-win32/RegistryAccess.h"
#include "vlr-util-win32/registry.iterator_RegEnumKey.h"
#include "vlr-util-win32/registry.enum_RegKeys.h"

using namespace vlr;
using namespace vlr::win32;
using namespace vlr::win32::registry;

// Test the iterator with HKEY_CLASSES_ROOT, which is a well-known registry hive that:
// - Is always accessible (no special privileges required)
// - Always has subkeys
// - Is stable across Windows versions
static const HKEY TEST_REGISTRY_HIVE = HKEY_CLASSES_ROOT;

TEST(registry_iterator_RegEnumKey, BasicIteratorConstruction)
{
	// Test default constructor
	auto iter = iterator_RegEnumKey(TEST_REGISTRY_HIVE);
	EXPECT_FALSE(iter.GetLastError().has_value());
}

TEST(registry_iterator_RegEnumKey, EndIteratorConstruction)
{
	// End iterator should not have a valid next index
	auto endIter = iterator_RegEnumKey(TEST_REGISTRY_HIVE);
	EXPECT_FALSE(endIter.GetLastError().has_value());
}

TEST(registry_iterator_RegEnumKey, IteratorAtIndexZero)
{
	// Create iterator at index 0
	auto iter = iterator_RegEnumKey(TEST_REGISTRY_HIVE, 0);
	// Should now be positioned at the first key
	// Note: We can't dereference if the hive has no keys, but HKEY_CLASSES_ROOT always has keys
	EXPECT_FALSE(iter.GetLastError().has_value());
}

TEST(registry_iterator_RegEnumKey, IteratorDereference)
{
	// Create iterator at index 0
	auto iter = iterator_RegEnumKey(TEST_REGISTRY_HIVE, 0);
	
	// Dereference should work
	const auto& result = *iter;
	EXPECT_FALSE(result.m_wsName.empty());
	EXPECT_GE(result.m_dwIndex, 0U);
}

TEST(registry_iterator_RegEnumKey, IteratorPointerAccess)
{
	// Create iterator at index 0
	auto iter = iterator_RegEnumKey(TEST_REGISTRY_HIVE, 0);
	
	// Pointer access should work
	const auto* pResult = iter.operator->();
	ASSERT_NE(pResult, nullptr);
	EXPECT_FALSE(pResult->m_wsName.empty());
	EXPECT_GE(pResult->m_dwIndex, 0U);
}

TEST(registry_iterator_RegEnumKey, PreIncrementOperator)
{
	auto iter1 = iterator_RegEnumKey(TEST_REGISTRY_HIVE, 0);
	auto iter2 = iterator_RegEnumKey(TEST_REGISTRY_HIVE, 0);
	
	// Get first key name
	const auto& result1 = *iter1;
	auto wsFirstKeyName = result1.m_wsName;
	
	// Pre-increment
	++iter1;
	
	// Second key should be different (if it exists)
	const auto& result2 = *iter1;
	// We can't assume there are multiple keys, but HKEY_CLASSES_ROOT definitely has them
	EXPECT_TRUE(result2.m_wsName.size() > 0);
}

TEST(registry_iterator_RegEnumKey, PostIncrementOperator)
{
	auto iter = iterator_RegEnumKey(TEST_REGISTRY_HIVE, 0);
	
	// Get first key name
	const auto& result1 = *iter;
	auto wsFirstKeyName = result1.m_wsName;
	
	// Post-increment returns old value
	auto oldIter = iter++;
	const auto& oldResult = *oldIter;
	EXPECT_EQ(oldResult.m_wsName, wsFirstKeyName);
	
	// Current iterator should now be at next position
	const auto& result2 = *iter;
	EXPECT_TRUE(result2.m_wsName.size() > 0);
}

TEST(registry_iterator_RegEnumKey, EqualityComparison)
{
	auto iter1 = iterator_RegEnumKey(TEST_REGISTRY_HIVE, 0);
	auto iter2 = iterator_RegEnumKey(TEST_REGISTRY_HIVE, 0);
	
	// Two iterators at the same position should be equal
	EXPECT_EQ(iter1, iter2);
	
	// After incrementing one, they should not be equal
	++iter1;
	EXPECT_NE(iter1, iter2);
}

TEST(registry_iterator_RegEnumKey, InequalityComparison)
{
	auto iter1 = iterator_RegEnumKey(TEST_REGISTRY_HIVE, 0);
	auto iter2 = iterator_RegEnumKey(TEST_REGISTRY_HIVE, 0);
	
	// Two iterators at the same position should not be unequal
	EXPECT_FALSE(iter1 != iter2);
	
	// After incrementing one, they should be unequal
	++iter1;
	EXPECT_TRUE(iter1 != iter2);
}

TEST(registry_iterator_RegEnumKey, EndIteratorEquality)
{
	auto beginIter = iterator_RegEnumKey(TEST_REGISTRY_HIVE, 0);
	auto endIter = iterator_RegEnumKey(TEST_REGISTRY_HIVE);
	
	// Begin and end should not be equal
	EXPECT_NE(beginIter, endIter);
	
	// Two end iterators should be equal
	auto endIter2 = iterator_RegEnumKey(TEST_REGISTRY_HIVE);
	EXPECT_EQ(endIter, endIter2);
}

TEST(registry_iterator_RegEnumKey, EnumKeyResultStructure)
{
	auto iter = iterator_RegEnumKey(TEST_REGISTRY_HIVE, 0);
	const auto& result = *iter;
	
	// Check all fields are populated
	EXPECT_FALSE(result.m_wsName.empty());
	EXPECT_GE(result.m_dwIndex, 0U);
	// m_wsClass may or may not be populated, that's OK
	// m_oLastWriteTime may or may not be populated, that's OK
}

TEST(registry_iterator_RegEnumKey, MultipleKeyIteration)
{
	// Collect keys using iterator
	std::vector<std::wstring> keys;
	
	auto beginIter = iterator_RegEnumKey(TEST_REGISTRY_HIVE, 0);
	auto endIter = iterator_RegEnumKey(TEST_REGISTRY_HIVE);
	
	size_t nIterationCount = 0;
	for (auto iter = beginIter; iter != endIter; ++iter)
	{
		const auto& result = *iter;
		keys.push_back(result.m_wsName);
		nIterationCount++;
		
		// Safety check to prevent infinite loops
		if (nIterationCount > 10000)
		{
			ADD_FAILURE() << "Iteration exceeded 10000 items, possible infinite loop";
			break;
		}
	}
	
	// Should have found at least some keys
	EXPECT_GE(keys.size(), 1U);
	
	// All keys should be non-empty
	for (const auto& key : keys)
	{
		EXPECT_FALSE(key.empty());
	}
	
	// Keys should not have duplicates (within reason)
	std::set<std::wstring> uniqueKeys(keys.begin(), keys.end());
	EXPECT_EQ(uniqueKeys.size(), keys.size());
}

TEST(registry_iterator_RegEnumKey, InvalidIteratorDereference)
{
	// Create end iterator (invalid state)
	auto endIter = iterator_RegEnumKey(TEST_REGISTRY_HIVE);
	
	// Dereferencing an invalid iterator should throw
	EXPECT_THROW(
		{
			const auto& result = *endIter;
			(void)result;
		},
		std::exception);
}

TEST(registry_iterator_RegEnumKey, InvalidIteratorPointerAccess)
{
	// Create end iterator (invalid state)
	auto endIter = iterator_RegEnumKey(TEST_REGISTRY_HIVE);
	
	// Pointer access on an invalid iterator should throw
	EXPECT_THROW(
		{
			const auto* pResult = endIter.operator->();
			(void)pResult;
		},
		std::exception);
}

TEST(registry_iterator_RegEnumKey, InvalidIteratorIncrement)
{
	// Create end iterator (invalid state)
	auto endIter = iterator_RegEnumKey(TEST_REGISTRY_HIVE);
	
	// Incrementing an invalid iterator should throw
	EXPECT_THROW(
		{
			++endIter;
		},
		std::exception);
}

TEST(registry_enum_RegKeys, BasicEnumeration)
{
	// Create enumerator
	auto oEnum = enum_RegKeys(TEST_REGISTRY_HIVE);
	
	// Collect keys
	std::vector<std::wstring> keys;
	size_t nIterationCount = 0;
	
	for (const auto& result : oEnum)
	{
		keys.push_back(result.m_wsName);
		nIterationCount++;
		
		// Safety check
		if (nIterationCount > 10000)
		{
			ADD_FAILURE() << "Iteration exceeded 10000 items, possible infinite loop";
			break;
		}
	}
	
	// Should have found keys
	EXPECT_GE(keys.size(), 1U);
}

TEST(registry_enum_RegKeys, BeginEndIterators)
{
	auto oEnum = enum_RegKeys(TEST_REGISTRY_HIVE);
	
	auto beginIter = oEnum.begin();
	auto endIter = oEnum.end();
	
	// Begin and end should not be equal
	EXPECT_NE(beginIter, endIter);
	
	// End iterator should be invalid
	EXPECT_THROW(
		{
			const auto& result = *endIter;
			(void)result;
		},
		std::exception);
}

TEST(registry_enum_RegKeys, RangeBasedForLoop)
{
	auto oEnum = enum_RegKeys(TEST_REGISTRY_HIVE);
	
	std::vector<std::wstring> keys;
	size_t nIterationCount = 0;
	
	for (const auto& result : oEnum)
	{
		keys.push_back(result.m_wsName);
		nIterationCount++;
		
		// Safety check
		if (nIterationCount > 10000)
		{
			ADD_FAILURE() << "Iteration exceeded 10000 items, possible infinite loop";
			break;
		}
	}
	
	// Should have found keys
	EXPECT_GE(keys.size(), 1U);
	
	// All keys should be valid
	for (const auto& key : keys)
	{
		EXPECT_FALSE(key.empty());
	}
}

TEST(registry_enum_RegKeys, KeyIndexValues)
{
	auto oEnum = enum_RegKeys(TEST_REGISTRY_HIVE);
	
	DWORD dwExpectedIndex = 0;
	size_t nIterationCount = 0;
	
	for (const auto& result : oEnum)
	{
		// Index should increment
		EXPECT_EQ(result.m_dwIndex, dwExpectedIndex);
		dwExpectedIndex++;
		nIterationCount++;
		
		if (nIterationCount > 10000)
		{
			ADD_FAILURE() << "Iteration exceeded 10000 items";
			break;
		}
	}
}

TEST(registry_enum_RegKeys, MultipleEnumerations)
{
	// Create two separate enumerators
	auto oEnum1 = enum_RegKeys(TEST_REGISTRY_HIVE);
	auto oEnum2 = enum_RegKeys(TEST_REGISTRY_HIVE);
	
	// Collect keys from both
	std::vector<std::wstring> keys1;
	std::vector<std::wstring> keys2;
	
	size_t nCount1 = 0;
	for (const auto& result : oEnum1)
	{
		keys1.push_back(result.m_wsName);
		if (++nCount1 > 10000) break;
	}
	
	size_t nCount2 = 0;
	for (const auto& result : oEnum2)
	{
		keys2.push_back(result.m_wsName);
		if (++nCount2 > 10000) break;
	}
	
	// Should enumerate the same keys in the same order
	EXPECT_EQ(keys1.size(), keys2.size());
	EXPECT_EQ(keys1, keys2);
}

TEST(registry_enum_RegKeys, EnumerateWithACL)
{
	// Test with SOFTWARE\Microsoft which exists and has stable subkeys
	ATL::CRegKey regKey;
	LONG lStatus = regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft"), KEY_READ);
	
	if (lStatus == ERROR_SUCCESS)
	{
		auto oEnum = enum_RegKeys((HKEY)regKey);
		
		std::vector<std::wstring> keys;
		size_t nIterationCount = 0;
		
		for (const auto& result : oEnum)
		{
			keys.push_back(result.m_wsName);
			nIterationCount++;
			
			if (nIterationCount > 10000)
			{
				ADD_FAILURE() << "Iteration exceeded 10000 items";
				break;
			}
		}
		
		// Should have found keys
		EXPECT_GE(keys.size(), 1U);
		
		regKey.Close();
	}
}

TEST(registry_iterator_RegEnumKey, IteratorStandardTraits)
{
	// Verify standard iterator traits
	static_assert(std::is_same_v<iterator_RegEnumKey::difference_type, std::ptrdiff_t>);
	static_assert(std::is_same_v<iterator_RegEnumKey::value_type, RegEnumKeyResult>);
	static_assert(std::is_same_v<iterator_RegEnumKey::pointer, const RegEnumKeyResult*>);
	static_assert(std::is_same_v<iterator_RegEnumKey::reference, const RegEnumKeyResult&>);
	static_assert(std::is_same_v<iterator_RegEnumKey::iterator_category, std::forward_iterator_tag>);
}

TEST(registry_enum_RegKeys, EmptyEnumerationIterator)
{
	// Test with a registry key that has no subkeys (if we can find one)
	// For this test, we'll create a temporary key and verify it can be enumerated (empty result)
	SResult sr;
	CRegistryAccess oReg(HKEY_CURRENT_USER);
	const auto sTestKey = fmt::format(_T("{}\\{}"), _T("SOFTWARE\\vlr-test"), _T("testEmptyEnum"));
	
	sr = oReg.EnsureKeyExists(sTestKey);
	if (sr == SResult::Success)
	{
		ATL::CRegKey regKey;
		LONG lStatus = regKey.Open(HKEY_CURRENT_USER, sTestKey.c_str(), KEY_READ);
		
		if (lStatus == ERROR_SUCCESS)
		{
			auto oEnum = enum_RegKeys((HKEY)regKey);
			
			int nKeyCount = 0;
			for (const auto& result : oEnum)
			{
				(void)result;
				nKeyCount++;
			}
			
			// Should have no subkeys
			EXPECT_EQ(nKeyCount, 0);
			
			regKey.Close();
		}
		
		// Clean up
		const auto oDeleteKeyOptions = CRegistryAccess::Options_DeleteKeysOrValues{}
			.withSafeDeletePath(_T("SOFTWARE\\vlr-test"));
		oReg.DeleteKey(sTestKey, oDeleteKeyOptions);
	}
}
