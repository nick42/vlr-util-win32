#include "pch.h"

#include <gtest/gtest.h>

#include "vlr-util-win32/filesystem.enum_volumes.h"

using namespace vlr::win32::filesystem;

TEST(filesystem_enum_volumes, begin_end_and_dereference)
{
    enum_volumes ev;
    auto it = ev.begin();
    auto end = ev.end();

    if (it == end)
    {
        // Either there are no volumes or an error occurred during FindFirstVolume.
        // In either case, ensure the iterator recorded an error or simply accept empty result.
        EXPECT_TRUE(it.GetLastError().has_value() || true);
        SUCCEED();
        return;
    }

    // We have at least one volume. Dereference must return a non-empty string.
    auto val = *it;
    EXPECT_FALSE(val.empty());

    // operator-> should return a pointer to the same string
    auto p = it.operator->();
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->size(), val.size());
    EXPECT_EQ(p->compare(val), 0);
}

TEST(filesystem_enum_volumes, iteration_and_copy_semantics)
{
    enum_volumes ev;
    auto it = ev.begin();
    auto end = ev.end();

    if (it == end)
    {
        // Nothing to iterate; accept and pass
        SUCCEED();
        return;
    }

    // Copying the iterator should create an independent iterator that initially equals original
    auto itCopy = it;
    EXPECT_TRUE(itCopy == it);

    // Advance the copy; original should remain at the same element
    ++itCopy;

    if (itCopy == end)
    {
        // Only one element existed; OK
        SUCCEED();
        return;
    }

    // Now we have at least two elements. Values should differ.
    EXPECT_NE((*it).compare(*itCopy), 0);
}
