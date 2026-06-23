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
#include <expected>

// POSIX Includes
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

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

        std::expected<void, std::string> create_file(std::string&& path) noexcept {
            const int fd = ::open(path.c_str(), O_CREAT, 0644);
            if (fd == -1) {
                return std::unexpected<std::string>(std::strerror(errno));
            }

            ::close(fd);
            return {};
        }

    } // unnamed namespace

    // ── Function Tests (backing_file_exists) ─────────────────────────────────────────────────────────────────────────
    TEST(backing_file_exists, backing_file_nullptr) { EXPECT_FALSE(::backing_file_exists(nullptr)); }

    TEST(backing_file_exists, does_not_exist) {
        constexpr struct ::backing_file file = {
            .bk_path = "/no/such/path/to/file.img",
            .bk_size = TEST_FILE_SIZE,
            .bk_fd = -1
        };

        // Ensure the path does not exist
        std::filesystem::remove_all(file.bk_path);

        EXPECT_FALSE(::backing_file_exists(file.bk_path));
    }

    TEST(backing_file_exists, exists) {
        // Ensure the file doesn't already exist
        std::filesystem::remove_all(TEST_FILE_PATH);

        std::fstream file{TEST_FILE_PATH, std::ios::out};
        EXPECT_TRUE(file.is_open());
        std::print(file, "");
        file.close();

        EXPECT_TRUE(::backing_file_exists(TEST_FILE_PATH));

        // Remove the file
        std::filesystem::remove(TEST_FILE_PATH);
    }

    // ── Function Tests (backing_file_is_reg) ─────────────────────────────────────────────────────────────────────────
    TEST(backing_file_is_reg, backing_file_nullptr) {
        EXPECT_FALSE(::backing_file_is_reg(nullptr));
        EXPECT_EQ(ENOENT, errno);
    }

    TEST(backing_file_is_reg, is_directory) {
        // Ensure the directory doesn't already exist
        std::filesystem::remove_all(TEST_FILE_PATH);

        EXPECT_NE(-1, ::mkdir(TEST_FILE_PATH, 0755));
        EXPECT_FALSE(::backing_file_is_reg(TEST_FILE_PATH));

        // Remove the directory
        std::filesystem::remove_all(TEST_FILE_PATH);
    }

    TEST(backing_file_is_reg, is_symlink) {
        // Ensure the symlink doesn't already exist
        std::filesystem::remove(TEST_FILE_PATH);

        // Create the symlink
        const int fd = ::symlink(std::getenv("HOME"), TEST_FILE_PATH);
        EXPECT_NE(-1, fd);
        ::close(fd);

        EXPECT_FALSE(::backing_file_is_reg(TEST_FILE_PATH));

        // Remove the symlink
        std::filesystem::remove(TEST_FILE_PATH);
    }

    TEST(backing_file_is_reg, is_pipe) {
        // Ensure the pipe doesn't already exist
        std::filesystem::remove(TEST_FILE_PATH);

        // Create the pipe
        const int fd = ::mkfifo(TEST_FILE_PATH, 0666);
        EXPECT_NE(-1, fd);
        ::close(fd);

        EXPECT_FALSE(::backing_file_is_reg(TEST_FILE_PATH));

        // Remove the pipe
        std::filesystem::remove(TEST_FILE_PATH);
    }

    TEST(backing_file_is_reg, is_socket) {
        // Ensure the socket doesn't already exist
        std::filesystem::remove(TEST_FILE_PATH);

        // Get a file descriptor to the UNIX socket file
        const int sock_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        EXPECT_NE(-1, sock_fd);

        // Initialize the socket
        struct ::sockaddr_un unix_sockaddr;
        unix_sockaddr.sun_family = AF_UNIX;
        strncpy(unix_sockaddr.sun_path, TEST_FILE_PATH, sizeof(TEST_FILE_PATH) - 1);
        unix_sockaddr.sun_path[sizeof(TEST_FILE_PATH)] = '\0';

        // Bind the UNIX socket to the test file
        EXPECT_NE(-1, ::bind(sock_fd, reinterpret_cast<struct ::sockaddr*>(&unix_sockaddr), sizeof(struct ::sockaddr_un)));
        ::close(sock_fd);

        EXPECT_FALSE(::backing_file_is_reg(TEST_FILE_PATH));

        // Remove the socket
        std::filesystem::remove(TEST_FILE_PATH);
    }

    TEST(backing_file_is_reg, is_regular_file) {
        // Ensure the file doesn't already exist
        std::filesystem::remove(TEST_FILE_PATH);

        // Create the file
        const int fd = ::open(TEST_FILE_PATH, O_RDONLY | O_CREAT, 0644);
        EXPECT_NE(-1, fd);
        ::close(fd);

        EXPECT_TRUE(::backing_file_is_reg(TEST_FILE_PATH));

        // Remove the file
        std::filesystem::remove(TEST_FILE_PATH);
    }

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

    // ── Function Tests (backing_file_remove) ─────────────────────────────────────────────────────────────────────────
    TEST(backing_file_remove, backing_file_nullptr) {
        EXPECT_EQ(-1, ::backing_file_remove(nullptr));
        EXPECT_EQ(ENOENT, errno);
    }

    TEST(backing_file_remove, file_does_not_exist) {
        struct ::backing_file file = {
            .bk_path = "/no/such/path/to/file.img",
            .bk_size = TEST_FILE_SIZE,
            .bk_fd = -1,
        };

        // Ensure the path actually does not exist
        std::filesystem::remove_all(file.bk_path);

        EXPECT_EQ(-1, ::backing_file_remove(&file));
        EXPECT_EQ(ENOENT, errno);
    }

    TEST(backing_file_remove, file_exists) {
        struct ::backing_file file;

        // Ensure the file actually exists
        EXPECT_EQ(EXIT_SUCCESS, ::backing_file_create(&file, TEST_FILE_PATH, TEST_FILE_SIZE));
        EXPECT_TRUE(std::filesystem::exists(TEST_FILE_PATH));

        file.bk_fd = ::open(file.bk_path, O_WRONLY, 0644);

        EXPECT_NE(-1, file.bk_fd);
        EXPECT_EQ(EXIT_SUCCESS, ::backing_file_remove(&file));
        EXPECT_FALSE(std::filesystem::exists(TEST_FILE_PATH));
        EXPECT_EQ(-1, file.bk_fd);
    }

} // namespace backing_file_testing

