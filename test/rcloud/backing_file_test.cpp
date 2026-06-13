#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO C++ Includes
#include <filesystem>

// ISO C Includes

// POSIX Includes

// GNU Includes

// Third Party Includes
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Local Includes
#include <rcloud/backing_file.h>

namespace backing_file_testing {
    TEST(backing_file_create, backing_file_nullptr) {
        EXPECT_EQ(-1, ::backing_file_create(nullptr, "no/such/path", 0));
        EXPECT_EQ(EFAULT, errno);
    }

    TEST(backing_file_create, path_nullptr) {
        struct ::backing_file file;
        EXPECT_EQ(-1, ::backing_file_create(&file, nullptr, 0));
        EXPECT_EQ(EFAULT, errno);
    }

    TEST(backing_file_create, non_positive_size) {
        struct ::backing_file file;
        EXPECT_EQ(-1, ::backing_file_create(&file, "no/such/path", -1));
        EXPECT_EQ(EINVAL, errno);

        // Reset errno
        errno = 0;

        EXPECT_EQ(-1, ::backing_file_create(&file, "no/such/path", 0));
        EXPECT_EQ(EINVAL, errno);
    }

    
} // namespace backing_file_testing

