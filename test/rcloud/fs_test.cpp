// ISO C++ Includes
#include <filesystem>

// ISO C Includes
#include <cstring>

// POSIX C Includes
#include <unistd.h>
#include <fcntl.h>

// Third Party Includes
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Local Includes
#include <rcloud/fs.h>

namespace fs_testing {
    namespace {
        constexpr ::mode_t DEFAULT_MODE = 0755;

        inline void touch(const char* path_p) { // throws std::system_error
            const int fd = ::open(path_p, O_RDWR | O_CREAT, 0600);
            if (fd == -1) {
                throw std::system_error(errno, std::system_category());
            }
            ::close(fd);
        }
    } // unnamed namespace

    // ── Function Tests (mkdirs) ──────────────────────────────────────────────────────────────────────────────────────
    TEST(mkdirs, nullptr_path) {
        EXPECT_EQ(-1, ::mkdirs(nullptr, 0));
        EXPECT_EQ(ENOENT, errno);
    }

    TEST(mkdirs, null_byte_path) {
        EXPECT_EQ(-1, ::mkdirs("\0", 0));
        EXPECT_EQ(ENOENT, errno);
    }

    TEST(mkdirs, no_exists) {
        // Ensure the test directory doesn't already exist
        if (std::filesystem::exists("/tmp/testdir")) {
            std::filesystem::remove_all("/tmp/testdir");
        }

        EXPECT_FALSE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_EQ(EXIT_SUCCESS, ::mkdirs("/tmp/testdir", DEFAULT_MODE));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir"));

        std::filesystem::remove("/tmp/testdir");
    }

    TEST(mkdirs, exists__empty_dir) {
        // Ensure the test directory exists and is empty
        if (std::filesystem::exists("/tmp/testdir")) {
            std::filesystem::remove_all("/tmp/testdir");
        }
        std::filesystem::create_directory("/tmp/testdir");

        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_TRUE(std::filesystem::is_empty("/tmp/testdir"));
        EXPECT_EQ(EXIT_SUCCESS, ::mkdirs("/tmp/testdir", DEFAULT_MODE));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_TRUE(std::filesystem::is_empty("/tmp/testdir"));

        std::filesystem::remove("/tmp/testdir");
    }

    TEST(mkdirs, exists__filled_dir) {
        // Ensure the test directory exists and contains files
        if (std::filesystem::exists("/tmp/testdir")) {
            std::filesystem::remove_all("/tmp/testdir");
        }
        std::filesystem::create_directory("/tmp/testdir");
        touch("/tmp/testdir/file.txt");
        touch("/tmp/testdir/file1.txt");
        touch("/tmp/testdir/file2.txt");

        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file2.txt"));

        EXPECT_EQ(EXIT_SUCCESS, ::mkdirs("/tmp/testdir", DEFAULT_MODE));

        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file2.txt"));

        std::filesystem::remove_all("/tmp/testdir");
    }

    TEST(mkdirs, exists__subdirs__empty_dirs) {
        // Ensure the test directory exists and is empty
        if (std::filesystem::exists("/tmp/testdir")) {
            std::filesystem::remove_all("/tmp/testdir");
        }

        // Create the test directory and it's subdirectories
        std::filesystem::create_directory("/tmp/testdir");
        std::filesystem::create_directory("/tmp/testdir/subdir1");
        std::filesystem::create_directory("/tmp/testdir/subdir2");
        std::filesystem::create_directory("/tmp/testdir/subdir3");

        // Ensure the test directory exists and it's subdirectories exists
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_FALSE(std::filesystem::is_empty("/tmp/testdir"));

        // Ensure each subdirectory exists and is empty
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1"));
        EXPECT_TRUE(std::filesystem::is_empty("/tmp/testdir/subdir1"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2"));
        EXPECT_TRUE(std::filesystem::is_empty("/tmp/testdir/subdir2"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3"));
        EXPECT_TRUE(std::filesystem::is_empty("/tmp/testdir/subdir3"));

        // Run mkdirs()
        EXPECT_EQ(EXIT_SUCCESS, ::mkdirs("/tmp/testdir", DEFAULT_MODE));

        // Ensure the test directory exists and it's subdirectories exists
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_FALSE(std::filesystem::is_empty("/tmp/testdir"));

        // Ensure each subdirectory exists and is empty
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1"));
        EXPECT_TRUE(std::filesystem::is_empty("/tmp/testdir/subdir1"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2"));
        EXPECT_TRUE(std::filesystem::is_empty("/tmp/testdir/subdir2"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3"));
        EXPECT_TRUE(std::filesystem::is_empty("/tmp/testdir/subdir3"));

        // Clean up
        std::filesystem::remove_all("/tmp/testdir");
    }

    TEST(mkdirs, exists__subdirs__filled_dirs) {
        // Ensure the test directory exists and is empty
        if (std::filesystem::exists("/tmp/testdir")) {
            std::filesystem::remove_all("/tmp/testdir");
        }

        // Create the test directory and it's subdirectories
        std::filesystem::create_directory("/tmp/testdir");
        std::filesystem::create_directory("/tmp/testdir/subdir1");
        std::filesystem::create_directory("/tmp/testdir/subdir2");
        std::filesystem::create_directory("/tmp/testdir/subdir3");

        // Create files for the test directory
        touch("/tmp/testdir/file.txt");
        touch("/tmp/testdir/file1.txt");
        touch("/tmp/testdir/file2.txt");

        // Create files for the first subdirectory
        touch("/tmp/testdir/subdir1/file.txt");
        touch("/tmp/testdir/subdir1/file1.txt");
        touch("/tmp/testdir/subdir1/file2.txt");

        // Create files for the second subdirectory
        touch("/tmp/testdir/subdir2/file.txt");
        touch("/tmp/testdir/subdir2/file1.txt");
        touch("/tmp/testdir/subdir2/file2.txt");

        // Create files for the third subdirectory
        touch("/tmp/testdir/subdir3/file.txt");
        touch("/tmp/testdir/subdir3/file1.txt");
        touch("/tmp/testdir/subdir3/file2.txt");

        // Ensure the test directory exists and it's files exist
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file2.txt"));

        // Ensure the first subdirectory exists and it's files exist
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1/file2.txt"));

        // Ensure the second subdirectory exists and it's files exist
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2/file2.txt"));

        // Ensure the third subdirectory exists and it's files exist
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3/file2.txt"));

        // Run mkdirs()
        EXPECT_EQ(EXIT_SUCCESS, ::mkdirs("/tmp/testdir", DEFAULT_MODE));

        // Ensure the test directory exists and it's files exist
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file2.txt"));

        // Ensure the first subdirectory exists and it's files exist
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1/file2.txt"));

        // Ensure the second subdirectory exists and it's files exist
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2/file2.txt"));

        // Ensure the third subdirectory exists and it's files exist
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3/file2.txt"));

        // Clean up
        std::filesystem::remove_all("/tmp/testdir");
    }

    // ── Function Tests (rmdirs) ──────────────────────────────────────────────────────────────────────────────────────
    TEST(rmdirs, nullptr_path) {
        EXPECT_EQ(-1, ::rmdirs(nullptr));
        EXPECT_EQ(ENOENT, errno);
    }

    TEST(rmdirs, null_byte_path) {
        EXPECT_EQ(-1, ::rmdirs("\0"));
        EXPECT_EQ(ENOENT, errno);
    }

    TEST(rmdirs, no_exists) {
        // Ensure the test directory doesn't already exist
        if (std::filesystem::exists("/tmp/testdir")) {
            std::filesystem::remove_all("/tmp/testdir");
        }

        EXPECT_FALSE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_EQ(-1, ::rmdirs("/tmp/testdir"));
        EXPECT_FALSE(std::filesystem::exists("/tmp/testdir"));
    }

    TEST(rmdirs, exists__empty_dir) {
        // Ensure the test directory exists and is empty
        if (std::filesystem::exists("/tmp/testdir")) {
            std::filesystem::remove_all("/tmp/testdir");
        }
        std::filesystem::create_directory("/tmp/testdir");

        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_TRUE(std::filesystem::is_empty("/tmp/testdir"));
        EXPECT_EQ(EXIT_SUCCESS, ::rmdirs("/tmp/testdir"));
        EXPECT_FALSE(std::filesystem::exists("/tmp/testdir"));
    }

    TEST(rmdirs, exists__filled_dir) {
        // Ensure the test directory exists and contains files
        if (std::filesystem::exists("/tmp/testdir")) {
            std::filesystem::remove_all("/tmp/testdir");
        }
        std::filesystem::create_directory("/tmp/testdir");
        touch("/tmp/testdir/file.txt");
        touch("/tmp/testdir/file1.txt");
        touch("/tmp/testdir/file2.txt");

        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file2.txt"));

        EXPECT_EQ(EXIT_SUCCESS, ::rmdirs("/tmp/testdir"));

        EXPECT_FALSE(std::filesystem::exists("/tmp/testdir"));
    }

    TEST(rmdirs, exists__subdirs__empty_dirs) {
        // Ensure the test directory exists and is empty
        if (std::filesystem::exists("/tmp/testdir")) {
            std::filesystem::remove_all("/tmp/testdir");
        }

        // Create the test directory and it's subdirectories
        std::filesystem::create_directory("/tmp/testdir");
        std::filesystem::create_directory("/tmp/testdir/subdir1");
        std::filesystem::create_directory("/tmp/testdir/subdir2");
        std::filesystem::create_directory("/tmp/testdir/subdir3");

        // Ensure the test directory exists and it's subdirectories exists
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_FALSE(std::filesystem::is_empty("/tmp/testdir"));

        // Ensure each subdirectory exists and is empty
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1"));
        EXPECT_TRUE(std::filesystem::is_empty("/tmp/testdir/subdir1"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2"));
        EXPECT_TRUE(std::filesystem::is_empty("/tmp/testdir/subdir2"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3"));
        EXPECT_TRUE(std::filesystem::is_empty("/tmp/testdir/subdir3"));

        // Run rmdirs()
        EXPECT_EQ(EXIT_SUCCESS, ::rmdirs("/tmp/testdir"));

        // Ensure the test directory no longer exists
        EXPECT_FALSE(std::filesystem::exists("/tmp/testdir"));
    }

    TEST(rmdirs, exists__subdirs__filled_dirs) {
        // Ensure the test directory exists and is empty
        if (std::filesystem::exists("/tmp/testdir")) {
            std::filesystem::remove_all("/tmp/testdir");
        }

        // Create the test directory and it's subdirectories
        std::filesystem::create_directory("/tmp/testdir");
        std::filesystem::create_directory("/tmp/testdir/subdir1");
        std::filesystem::create_directory("/tmp/testdir/subdir2");
        std::filesystem::create_directory("/tmp/testdir/subdir3");

        // Create files for the test directory
        touch("/tmp/testdir/file.txt");
        touch("/tmp/testdir/file1.txt");
        touch("/tmp/testdir/file2.txt");

        // Create files for the first subdirectory
        touch("/tmp/testdir/subdir1/file.txt");
        touch("/tmp/testdir/subdir1/file1.txt");
        touch("/tmp/testdir/subdir1/file2.txt");

        // Create files for the second subdirectory
        touch("/tmp/testdir/subdir2/file.txt");
        touch("/tmp/testdir/subdir2/file1.txt");
        touch("/tmp/testdir/subdir2/file2.txt");

        // Create files for the third subdirectory
        touch("/tmp/testdir/subdir3/file.txt");
        touch("/tmp/testdir/subdir3/file1.txt");
        touch("/tmp/testdir/subdir3/file2.txt");

        // Ensure the test directory exists and it's files exist
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/file2.txt"));

        // Ensure the first subdirectory exists and it's files exist
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir1/file2.txt"));

        // Ensure the second subdirectory exists and it's files exist
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir2/file2.txt"));

        // Ensure the third subdirectory exists and it's files exist
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3/file.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3/file1.txt"));
        EXPECT_TRUE(std::filesystem::exists("/tmp/testdir/subdir3/file2.txt"));

        // Run rmdirs()
        EXPECT_EQ(EXIT_SUCCESS, ::rmdirs("/tmp/testdir"));

        // Ensure the test directory no longer exists
        EXPECT_FALSE(std::filesystem::exists("/tmp/testdir"));
    }
} // namespace fs_testing

