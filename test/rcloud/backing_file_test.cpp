#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO C Includes
#include <cstdlib>
#include <cstring>
#include <cerrno>

// ISO C++ Includes
#include <string>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <expected>

// POSIX Includes
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/sysmacros.h>

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

        inline std::error_code get_error() noexcept { return std::error_code{errno, std::system_category()}; }

        std::expected<void, std::error_code> create_regular_file(const char* path_p) noexcept {
            const int fd = ::open(path_p, O_WRONLY | O_CREAT | O_EXCL, 0644);
            if (fd == -1) [[unlikely]] {
                return std::unexpected{get_error()};
            }
            ::close(fd);
            return {};
        }

        std::expected<void, std::error_code> create_directory(const char* path_p) noexcept {
            if (::mkdir(path_p, 0755) == -1) [[unlikely]] {
                return std::unexpected{get_error()};
            }
            return {};
        }

        std::expected<void, std::error_code> create_symlink(const char* from_path_p, const char* to_path_p) noexcept {
            if (::symlink(from_path_p, to_path_p) == -1) [[unlikely]] {
                return std::unexpected(get_error());
            }
            return {};
        }

        std::expected<void, std::error_code> create_pipe(const char* path_p) noexcept {
            if (::mkfifo(path_p, 0666) == -1) [[unlikely]] {
                return std::unexpected{get_error()};
            }
            return {};
        }

        std::expected<void, std::error_code> create_socket(const char* path_p) noexcept {
            const int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
            if (fd == -1) [[unlikely]] {
                return std::unexpected{get_error()};
            }

            const std::size_t path_len = std::strlen(path_p);
            struct ::sockaddr_un addr;
            addr.sun_family = AF_UNIX;
            std::strncpy(addr.sun_path, path_p, path_len);
            addr.sun_path[path_len] = '\0';

            // Binding assigns the socket to the file system path
            ::bind(fd, reinterpret_cast<struct ::sockaddr*>(&addr), sizeof(struct ::sockaddr));
            ::close(fd);

            return {};
        }

        std::expected<void, std::error_code> create_char_device(const char* path_p) noexcept {
            if (::mknod(path_p, S_IFCHR | 0666, makedev(1, 3)) == -1) [[unlikely]] {
                return std::unexpected{get_error()};
            }
            return {};
        }

        std::expected<void, std::error_code> create_block_device(const char* path_p) noexcept {
            if (::mknod(path_p, S_IFBLK | 0660, makedev(1, 3)) == -1) [[unlikely]] {
                return std::unexpected{get_error()};
            }
            return {};
        }
    } // unnamed namespace

    // ── Function Tests (backing_file_create) ─────────────────────────────────────────────────────────────────────────
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

    // ── Function Tests (backing_file_destroy) ────────────────────────────────────────────────────────────────────────
    TEST(backing_file_destroy, backing_file_nullptr) {
        EXPECT_EQ(-1, ::backing_file_destroy(nullptr));
        EXPECT_EQ(ENOENT, errno);
    }

    TEST(backing_file_destroy, file_does_not_exist) {
        struct ::backing_file file = {
            .bk_path = "/no/such/path/to/file.img",
            .bk_size = TEST_FILE_SIZE,
            .bk_fd = -1,
        };

        // Ensure the path actually does not exist
        std::filesystem::remove_all(file.bk_path);

        EXPECT_EQ(-1, ::backing_file_destroy(&file));
        EXPECT_EQ(ENOENT, errno);
    }

    TEST(backing_file_destroy, file_exists) {
        struct ::backing_file file;

        // Ensure the file actually exists
        EXPECT_EQ(EXIT_SUCCESS, ::backing_file_create(&file, TEST_FILE_PATH, TEST_FILE_SIZE));
        EXPECT_TRUE(std::filesystem::exists(TEST_FILE_PATH));

        file.bk_fd = ::backing_file_open(&file, O_WRONLY, 0644);

        EXPECT_NE(-1, file.bk_fd);
        EXPECT_EQ(EXIT_SUCCESS, ::backing_file_destroy(&file));
        EXPECT_FALSE(std::filesystem::exists(TEST_FILE_PATH));
        EXPECT_EQ(-1, file.bk_fd);
    }

    // ── Function Tests (backing_file_get_state) ──────────────────────────────────────────────────────────────────────
    TEST(backing_file_get_state, backing_file_nullptr) {
        EXPECT_EQ(DOES_NOT_EXIST, ::backing_file_get_state(nullptr));
    }

    TEST(backing_file_get_state, backing_file_does_not_exist) {
        constexpr struct ::backing_file file = {
            .bk_path = "/no/such/path/to/file.img",
            .bk_size = TEST_FILE_SIZE,
            .bk_fd = -1
        };

        // Ensure the path does not exist
        std::filesystem::remove_all(file.bk_path);

        EXPECT_EQ(DOES_NOT_EXIST, ::backing_file_get_state(file.bk_path));
    }

    TEST(backing_file_get_state, exists_but_is_directory) {
        // Ensure the directory doesn't already exist
        std::filesystem::remove_all("/tmp/test");

        create_directory("/tmp/test");

        EXPECT_EQ(NOT_REGULAR, ::backing_file_get_state("/tmp/test"));

        // Remove the directory
        std::filesystem::remove_all("/tmp/test");
    }

    TEST(backing_file_get_state, exists_but_is_symlink) {
        // Ensure the directory doesn't already exist
        std::filesystem::remove_all("/tmp/test");

        create_symlink("/tmp", "/tmp/test");

        EXPECT_EQ(NOT_REGULAR, ::backing_file_get_state("/tmp/test"));

        // Remove the symlink
        std::filesystem::remove_all("/tmp/test");
    }

    TEST(backing_file_get_state, exists_but_is_pipe) {
        // Ensure the directory doesn't already exist
        std::filesystem::remove_all("/tmp/test");

        create_pipe("/tmp/test");

        EXPECT_EQ(NOT_REGULAR, ::backing_file_get_state("/tmp/test"));

        // Remove the symlink
        std::filesystem::remove_all("/tmp/test");
    }

} // namespace backing_file_testing

