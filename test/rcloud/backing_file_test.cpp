#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO C++ Includes
#include <filesystem>
#include <fstream>

// ISO C Includes

// POSIX Includes
#include <unistd.h>
#include <sys/stat.h>

// GNU Includes

// Third Party Includes
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Local Includes
#include <rcloud/backing_file.h>

namespace backing_file_testing {
    namespace {
        constexpr char TEST_FILE_PATH[] = "/tmp/test.img";
        constexpr ::off_t TEST_FILE_SIZE =  1074000000L; // 1 Gibibyte (GiB)
    } // unnamed namespace

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

    TEST(backing_file_create, path_size_too_large) {
        struct ::backing_file file;
        constexpr char path[] = "/helloworld/helloworld/helloworld/helloworld/helloworld/helloworld/helloworld";
        EXPECT_EQ(-1, ::backing_file_create(&file, path, 4096));
        EXPECT_EQ(ENAMETOOLONG, errno);
    }

    TEST(backing_file_create, file_already_exists) {
        struct ::backing_file file;

        // Ensure the file doesn't already exist
        std::filesystem::remove(TEST_FILE_PATH);

        // Create the file
        if (std::fstream backing_file(TEST_FILE_PATH, std::ios::out);backing_file.is_open()) {
            backing_file.close();
        }

        EXPECT_EQ(-1, ::backing_file_create(&file, TEST_FILE_PATH, TEST_FILE_SIZE));
        EXPECT_EQ(EEXIST, errno);

        // Remove the file
        std::filesystem::remove(TEST_FILE_PATH);
    }

    TEST(backing_file_create, everything_valid) {
        struct ::backing_file file;
        struct ::stat st;

        EXPECT_EQ(EXIT_SUCCESS, ::backing_file_create(&file, TEST_FILE_PATH, TEST_FILE_SIZE));
        EXPECT_TRUE(std::filesystem::exists(TEST_FILE_PATH));
        EXPECT_EQ(EXIT_SUCCESS, ::stat(TEST_FILE_PATH, &st));
        EXPECT_EQ(TEST_FILE_SIZE, st.st_size);

        // Remove the file
        std::filesystem::remove(TEST_FILE_PATH);
    }

} // namespace backing_file_testing

