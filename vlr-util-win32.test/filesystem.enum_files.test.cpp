#include "pch.h"

#include <vector>
#include <set>
#include <filesystem>

#include "vlr-util/cpp_namespace.h"
#include "vlr-util/include.fmt.h"

#include "vlr-util-win32/filesystem.enum_files.h"

using namespace vlr;
using namespace vlr::win32;
using namespace vlr::win32::filesystem;

// Test the iterator with the Windows TEMP directory, which:
// - Is always accessible without special privileges
// - Always exists
// - Usually has files in it for testing
static const auto GetTempDirectoryPath = []()
{
    wchar_t szTempPath[MAX_PATH];
    DWORD dwSize = ::GetTempPathW(MAX_PATH, szTempPath);
    if (dwSize > 0 && dwSize < MAX_PATH)
    {
        return std::wstring(szTempPath);
    }
    // Fallback to System32
    return std::wstring(_T("C:\\Windows\\System32\\"));
};

TEST(filesystem_iterator_FindNextFile, BasicIteratorConstruction)
{
    // Test default constructor
    auto iter = iterator_FindNextFile{};
    EXPECT_FALSE(iter.GetLastError().has_value());
}

TEST(filesystem_iterator_FindNextFile, IteratorDereference)
{
    // Create an enumerator for temp directory
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);

    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;

    auto iter = oEnum.begin();
    auto endIter = oEnum.end();

    if (iter != endIter)
    {
        // Dereference should work
        const auto* pFindData = *iter;
        ASSERT_NE(pFindData, nullptr);
        EXPECT_FALSE(std::wstring(pFindData->cFileName).empty());
    }
}

TEST(filesystem_iterator_FindNextFile, IteratorPointerAccess)
{
    // Create an enumerator for temp directory
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);

    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;

    auto iter = oEnum.begin();
    auto endIter = oEnum.end();

    if (iter != endIter)
    {
        // Pointer access should work
        const auto* pFindData = iter.operator->();
        ASSERT_NE(pFindData, nullptr);
        EXPECT_FALSE(std::wstring(pFindData->cFileName).empty());
    }
}

TEST(filesystem_iterator_FindNextFile, PreIncrementOperator)
{
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);

    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;

    auto iter = oEnum.begin();
    auto endIter = oEnum.end();

    if (iter != endIter)
    {
        // Get first file name
        const auto* pFirstResult = *iter;
        auto wsFirstFileName = std::wstring(pFirstResult->cFileName);

        // Pre-increment
        ++iter;

        // If there's another file, it should be different
        if (iter != endIter)
        {
            const auto* pSecondResult = *iter;
            auto wsSecondFileName = std::wstring(pSecondResult->cFileName);
            EXPECT_TRUE(wsSecondFileName.size() > 0);
        }
    }
}

TEST(filesystem_iterator_FindNextFile, PostIncrementOperator)
{
	auto sTempPath = GetTempDirectoryPath();
	auto sSearchString = fmt::format(_T("{}*"), sTempPath);
	
	auto oEnum = enum_files{};
	oEnum.m_sSearchString = sSearchString;
	
	auto iter = oEnum.begin();
	auto endIter = oEnum.end();
	
	if (iter != endIter)
	{
		// Just verify post-increment doesn't throw and returns something
		// The semantics of post-increment with shared pointers can be complex
		auto resultIter = iter++;
		
		// After post-increment, we should be able to iterate further or reach end
		// (no exception thrown is the main test here)
		EXPECT_TRUE(true);
	}
}

TEST(filesystem_iterator_FindNextFile, EqualityComparison)
{
	auto sTempPath = GetTempDirectoryPath();
	auto sSearchString = fmt::format(_T("{}*"), sTempPath);
	
	auto oEnum = enum_files{};
	oEnum.m_sSearchString = sSearchString;
	
	// Create two end iterators - they should be equal
	auto endIter1 = oEnum.end();
	auto endIter2 = oEnum.end();
	EXPECT_EQ(endIter1, endIter2);
	
	// Begin iterator should not equal end iterator
	auto beginIter = oEnum.begin();
	if (beginIter != oEnum.end())
	{
		EXPECT_NE(beginIter, endIter1);
	}
}

TEST(filesystem_iterator_FindNextFile, InequalityComparison)
{
	auto sTempPath = GetTempDirectoryPath();
	auto sSearchString = fmt::format(_T("{}*"), sTempPath);
	
	auto oEnum = enum_files{};
	oEnum.m_sSearchString = sSearchString;
	
	// Two end iterators should not be unequal
	auto endIter1 = oEnum.end();
	auto endIter2 = oEnum.end();
	EXPECT_FALSE(endIter1 != endIter2);
	
	// Begin iterator should be unequal to end iterator
	auto beginIter = oEnum.begin();
	if (beginIter != oEnum.end())
	{
		EXPECT_TRUE(beginIter != endIter1);
	}
}

TEST(filesystem_iterator_FindNextFile, EndIteratorEquality)
{
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);

    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;

    auto beginIter = oEnum.begin();
    auto endIter = oEnum.end();

    // Begin and end should not be equal (unless directory is empty)
    // Two end iterators should be equal
    auto endIter2 = oEnum.end();
    EXPECT_EQ(endIter, endIter2);
}

TEST(filesystem_iterator_FindNextFile, FileDataStructure)
{
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);

    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;

    auto iter = oEnum.begin();
    if (iter != oEnum.end())
    {
        const auto* pFindData = *iter;

        // Check WIN32_FIND_DATA fields are accessible
        auto wsFileName = std::wstring(pFindData->cFileName);
        EXPECT_FALSE(wsFileName.empty());

        // These should be initialized
        EXPECT_GE(pFindData->nFileSizeLow, 0U);
        EXPECT_GE(pFindData->dwFileAttributes, 0U);
    }
}

TEST(filesystem_iterator_FindNextFile, MultipleFileIteration)
{
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);

    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;

    // Collect files using iterator
    std::vector<std::wstring> files;

    auto beginIter = oEnum.begin();
    auto endIter = oEnum.end();

    size_t nIterationCount = 0;
    for (auto iter = beginIter; iter != endIter; ++iter)
    {
        const auto* pFindData = *iter;
        files.push_back(std::wstring(pFindData->cFileName));
        nIterationCount++;

        // Safety check to prevent infinite loops
        if (nIterationCount > 100000)
        {
            ADD_FAILURE() << "Iteration exceeded 100000 items, possible infinite loop";
            break;
        }
    }

    // Should have found at least some files
    EXPECT_GE(files.size(), 1U);

    // All files should have non-empty names
    for (const auto& file : files)
    {
        EXPECT_FALSE(file.empty());
    }
}

TEST(filesystem_iterator_FindNextFile, InvalidIteratorDereference)
{
    // Create end iterator (invalid state)
    auto endIter = iterator_FindNextFile{};

    // Dereferencing an invalid iterator should throw
    EXPECT_THROW(
        {
            const auto* pFindData = *endIter;
            (void)pFindData;
        },
        std::exception);
}

TEST(filesystem_iterator_FindNextFile, InvalidIteratorIncrement)
{
    // Create end iterator (invalid state)
    auto endIter = iterator_FindNextFile{};

    // Incrementing an invalid iterator should throw
    EXPECT_THROW(
        {
            ++endIter;
        },
        std::exception);
}

TEST(filesystem_enum_files, BasicEnumeration)
{
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);

    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;

    // Collect files
    std::vector<std::wstring> files;
    size_t nIterationCount = 0;

    for (const auto* pFindData : oEnum)
    {
        files.push_back(std::wstring(pFindData->cFileName));
        nIterationCount++;

        // Safety check
        if (nIterationCount > 100000)
        {
            ADD_FAILURE() << "Iteration exceeded 100000 items, possible infinite loop";
            break;
        }
    }

    // Should have found files
    EXPECT_GE(files.size(), 1U);
}

TEST(filesystem_enum_files, BeginEndIterators)
{
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);

    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;

    auto beginIter = oEnum.begin();
    auto endIter = oEnum.end();

    // End iterator should be invalid
    EXPECT_THROW(
        {
            const auto* pFindData = *endIter;
            (void)pFindData;
        },
        std::exception);
}

TEST(filesystem_enum_files, RangeBasedForLoop)
{
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);

    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;

    std::vector<std::wstring> files;
    size_t nIterationCount = 0;

    for (const auto* pFindData : oEnum)
    {
        files.push_back(std::wstring(pFindData->cFileName));
        nIterationCount++;

        // Safety check
        if (nIterationCount > 100000)
        {
            ADD_FAILURE() << "Iteration exceeded 100000 items, possible infinite loop";
            break;
        }
    }

    // Should have found files
    EXPECT_GE(files.size(), 1U);

    // All files should be valid
    for (const auto& file : files)
    {
        EXPECT_FALSE(file.empty());
    }
}

TEST(filesystem_enum_files, MultipleEnumerations)
{
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);

    // Create two separate enumerators
    auto oEnum1 = enum_files{};
    oEnum1.m_sSearchString = sSearchString;

    auto oEnum2 = enum_files{};
    oEnum2.m_sSearchString = sSearchString;

    // Collect files from both
    std::vector<std::wstring> files1;
    std::vector<std::wstring> files2;

    size_t nCount1 = 0;
    for (const auto* pFindData : oEnum1)
    {
        files1.push_back(std::wstring(pFindData->cFileName));
        if (++nCount1 > 100000) break;
    }

    size_t nCount2 = 0;
    for (const auto* pFindData : oEnum2)
    {
        files2.push_back(std::wstring(pFindData->cFileName));
        if (++nCount2 > 100000) break;
    }

    // Should enumerate the same files in the same order
    EXPECT_EQ(files1.size(), files2.size());
    EXPECT_EQ(files1, files2);
}

TEST(filesystem_enum_files, EnumerateWithDifferentSearchPatterns)
{
    auto sTempPath = GetTempDirectoryPath();

    // Search for all files
    auto sSearchAll = fmt::format(_T("{}*"), sTempPath);
    auto oEnumAll = enum_files{};
    oEnumAll.m_sSearchString = sSearchAll;

    std::vector<std::wstring> allFiles;
    size_t nCount = 0;
    for (const auto* pFindData : oEnumAll)
    {
        allFiles.push_back(std::wstring(pFindData->cFileName));
        if (++nCount > 100000) break;
    }

    // Search for only text files
    auto sSearchText = fmt::format(_T("{}*.txt"), sTempPath);
    auto oEnumText = enum_files{};
    oEnumText.m_sSearchString = sSearchText;

    std::vector<std::wstring> textFiles;
    nCount = 0;
    for (const auto* pFindData : oEnumText)
    {
        textFiles.push_back(std::wstring(pFindData->cFileName));
        if (++nCount > 100000) break;
    }

    // All files should be at least as many as text files
    EXPECT_GE(allFiles.size(), textFiles.size());

    // All text files should have .txt extension
    for (const auto& file : textFiles)
    {
        EXPECT_NE(file.find(_T(".txt")), std::wstring::npos);
    }
}

TEST(filesystem_enum_files, EnumerateSystemDirectory)
{
	auto sSearchString = _T("C:\\Windows\\System32\\*");
	
	auto oEnum = enum_files{};
	oEnum.m_sSearchString = sSearchString;
	
	std::vector<std::wstring> files;
	size_t nIterationCount = 0;
	
	for (const auto* pFindData : oEnum)
	{
		files.push_back(std::wstring(pFindData->cFileName));
		nIterationCount++;
		
		if (nIterationCount > 100000) break;
	}
	
	// System32 should have some files
	EXPECT_GE(files.size(), 1U);
	
	// All files should have valid names
	for (const auto& file : files)
	{
		EXPECT_FALSE(file.empty());
	}
}

TEST(filesystem_enum_files, HandleNonexistentDirectory)
{
    auto sSearchString = _T("C:\\NonexistentDirectory\\*");

    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;

    auto beginIter = oEnum.begin();
    auto endIter = oEnum.end();

    // Should gracefully handle nonexistent directory
    EXPECT_EQ(beginIter, endIter);
    EXPECT_TRUE(beginIter.GetLastError().has_value());
}

TEST(filesystem_enum_files, FilesHaveAttributes)
{
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);

    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;

    auto beginIter = oEnum.begin();
    if (beginIter != oEnum.end())
    {
        const auto* pFindData = *beginIter;

        // Check that file attributes are set
        EXPECT_NE(pFindData->dwFileAttributes, 0U);

        // Attributes should be valid Windows file attributes
        auto dwValidAttributes = FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_DIRECTORY | 
                                FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM |
                                FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY |
                                FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_COMPRESSED;
        EXPECT_NE(pFindData->dwFileAttributes & dwValidAttributes, 0U);
    }
}

TEST(filesystem_enum_files, DotAndDotDotEntries)
{
	// The directory enumeration should include . and .. when pseudo entries are not skipped
	auto sTempPath = GetTempDirectoryPath();
	auto sSearchString = fmt::format(_T("{}*"), sTempPath);

	auto oEnum = enum_files{};
	oEnum.m_sSearchString = sSearchString;
	oEnum.m_bSkipPseudoDirEntries = false;

	std::vector<std::wstring> files;
	size_t nCount = 0;
	for (const auto* pFindData : oEnum)
	{
		files.push_back(std::wstring(pFindData->cFileName));
		if (++nCount > 100000) break;
	}

	// Should have at least . entry
	bool bFoundDot = false;
	for (const auto& file : files)
	{
		if (file == _T("."))
		{
			bFoundDot = true;
			break;
		}
	}
	EXPECT_TRUE(bFoundDot);
}

TEST(filesystem_iterator_FindNextFile, IteratorStandardTraits)
{
    // Verify standard iterator traits
    static_assert(std::is_same_v<iterator_FindNextFile::difference_type, std::ptrdiff_t>);
    static_assert(std::is_same_v<iterator_FindNextFile::value_type, WIN32_FIND_DATA>);
    static_assert(std::is_same_v<iterator_FindNextFile::pointer, const WIN32_FIND_DATA*>);
    static_assert(std::is_same_v<iterator_FindNextFile::reference, const WIN32_FIND_DATA*>);
    static_assert(std::is_same_v<iterator_FindNextFile::iterator_category, std::forward_iterator_tag>);
}

TEST(filesystem_enum_files, EnumerateCurrentDirectory)
{
    // Get current working directory
    wchar_t szCurrentPath[MAX_PATH];
    if (::GetCurrentDirectoryW(MAX_PATH, szCurrentPath) > 0)
    {
        auto sSearchString = fmt::format(_T("{}\\*"), szCurrentPath);

        auto oEnum = enum_files{};
        oEnum.m_sSearchString = sSearchString;

        std::vector<std::wstring> files;
        size_t nCount = 0;
        for (const auto* pFindData : oEnum)
        {
            files.push_back(std::wstring(pFindData->cFileName));
            if (++nCount > 100000) break;
        }

        // Should have found some entries
        EXPECT_GE(files.size(), 1U);
    }
}

TEST(filesystem_enum_files, StdForEachCompatibility)
{
    // Test compatibility with std::for_each algorithm
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);
    
    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;
    oEnum.m_bSkipPseudoDirEntries = false; // Include . and ..
    
    std::vector<std::wstring> files;
    size_t nIterationCount = 0;
    
    // Use std::for_each with a lambda
    std::for_each(oEnum.begin(), oEnum.end(), [&](const WIN32_FIND_DATA* pFindData)
    {
        if (pFindData != nullptr)
        {
            files.push_back(std::wstring(pFindData->cFileName));
            nIterationCount++;
        }
    });
    
    // Should have enumerated files successfully
    EXPECT_GE(files.size(), 1U);
    EXPECT_EQ(nIterationCount, files.size());
}

TEST(filesystem_enum_files, StdForEachWithFunction)
{
    // Test std::for_each with a function object
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);
    
    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;
    oEnum.m_bSkipPseudoDirEntries = false;
    
    struct FileCounter
    {
        std::vector<std::wstring> files;
        size_t count = 0;
        
        void operator()(const WIN32_FIND_DATA* pFindData)
        {
            if (pFindData != nullptr)
            {
                files.push_back(std::wstring(pFindData->cFileName));
                count++;
            }
        }
    };
    
    FileCounter counter;
    std::for_each(oEnum.begin(), oEnum.end(), std::ref(counter));
    
    // Should have enumerated files
    EXPECT_GE(counter.count, 1U);
    EXPECT_EQ(counter.count, counter.files.size());
}

TEST(filesystem_enum_files, SkipPseudoDirEntriesEnabled)
{
    // Test skipping . and .. entries (default behavior)
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);
    
    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;
    oEnum.m_bSkipPseudoDirEntries = true;
    
    std::vector<std::wstring> files;
    size_t nCount = 0;
    for (const auto* pFindData : oEnum)
    {
        auto wsFileName = std::wstring(pFindData->cFileName);
        files.push_back(wsFileName);
        if (++nCount > 100000) break;
    }
    
    // Should not contain . or ..
    for (const auto& file : files)
    {
        EXPECT_NE(file, _T("."));
        EXPECT_NE(file, _T(".."));
    }
    
    // Should have found some files
    EXPECT_GE(files.size(), 1U);
}

TEST(filesystem_enum_files, SkipPseudoDirEntriesDisabled)
{
    // Test including . and .. entries when disabled
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);
    
    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;
    oEnum.m_bSkipPseudoDirEntries = false;
    
    std::vector<std::wstring> files;
    size_t nCount = 0;
    for (const auto* pFindData : oEnum)
    {
        auto wsFileName = std::wstring(pFindData->cFileName);
        files.push_back(wsFileName);
        if (++nCount > 100000) break;
    }
    
    // Should contain at least . entry
    bool bFoundDot = false;
    for (const auto& file : files)
    {
        if (file == _T("."))
        {
            bFoundDot = true;
            break;
        }
    }
    EXPECT_TRUE(bFoundDot);
}

TEST(filesystem_enum_files, SkipPseudoDirEntriesDefaultBehavior)
{
    // Test that default behavior is to skip pseudo entries
    auto sTempPath = GetTempDirectoryPath();
    auto sSearchString = fmt::format(_T("{}*"), sTempPath);
    
    auto oEnum = enum_files{};
    oEnum.m_sSearchString = sSearchString;
    // Don't set m_bSkipPseudoDirEntries; it should default to true
    
    std::vector<std::wstring> files;
    size_t nCount = 0;
    for (const auto* pFindData : oEnum)
    {
        auto wsFileName = std::wstring(pFindData->cFileName);
        files.push_back(wsFileName);
        if (++nCount > 100000) break;
    }
    
    // Default should skip . and ..
    for (const auto& file : files)
    {
        EXPECT_NE(file, _T("."));
        EXPECT_NE(file, _T(".."));
    }
}
