/// Unit tests for utilities::Platform.

#include <gtest/gtest.h>

#include "stagehand/utilities/platform.h"

TEST(Platform, GetThreadCountReturnsAtLeastOne) {
    unsigned int count = utilities::Platform::get_thread_count();
    ASSERT_GE(count, 1u);
}

TEST(Platform, GetThreadCountDoesNotExceed64) {
    unsigned int count = utilities::Platform::get_thread_count();
    ASSERT_LE(count, 64u);
}

TEST(Platform, GetThreadCountIsConsistentAcrossCalls) {
    unsigned int first = utilities::Platform::get_thread_count();
    unsigned int second = utilities::Platform::get_thread_count();
    ASSERT_EQ(first, second);
}
