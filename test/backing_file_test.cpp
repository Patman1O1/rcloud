#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// ISO C++ Includes
#include <optional>
#include <string>

// ISO C Includes
#include <cerrno>
#include <cstring>

// POSIX Includes
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

// Google Test Includes
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Local Includes
#include "rcloud/backing_file.h"
#include "fd_handle.hpp"
#include "file_handle.hpp"
#include "dir_handle.hpp"

#define BACKING_FILE_TEST_TMPDIR "/tmp/rcloud_backing_file_test_XXXXXX"

namespace backing_file_testing {
    namespace {
        inline ::off64_t file_size_on_disk(const char* path) noexcept {
            struct ::stat64 st64;
            return ::stat64(path, &st64) == EXIT_SUCCESS ? st64.st_size : -1;
        }
    } // anonymous namespace

    // ── backing_file_test_base ───────────────────────────────────────────────────────────────────────────────────────
    class backing_file_test_base : public ::testing::Test {
    protected:
        std::optional<rcloud_testing::dir_handle> test_dir_;

        void SetUp() override {
            char tmpl[] = BACKING_FILE_TEST_TMPDIR;
            const char* dir = ::mkdtemp(tmpl);
            ASSERT_NE(dir, nullptr) << "mkdtemp failed: " << ::strerror(errno);
            test_dir_.emplace(dir);
        }

        std::string path(const std::string& rel) const {
            return static_cast<const char*>(*test_dir_) + std::string("/") + rel;
        }
    };

    // ── backing_file_create ──────────────────────────────────────────────────────────────────────────────────────────

    class backing_file_create_test : public backing_file_test_base {};

    // ── Happy paths ──────────────────────────────────────────────────────────────────────────────────────────────────

    TEST_F(backing_file_create_test, create_files_in_existing_directory) {
        const std::string p = path("flat.img");

        const rcloud_testing::fd_handle handle{ ::backing_file_create(p.c_str()) };

        ASSERT_NE(static_cast<int>(handle), -1) << "backing_file_create failed: " << std::strerror(errno);
        EXPECT_EQ(::access(p.c_str(), F_OK), EXIT_SUCCESS) << "File not found after creation";
    }

    TEST_F(backing_file_create_test, has_o_rdwr_flag) {
        const std::string p = path("rw.img");

        const rcloud_testing::fd_handle handle{ ::backing_file_create(p.c_str()) };
        ASSERT_NE(static_cast<int>(handle), -1);

        const char w = 0x42;
        ASSERT_EQ(::write(static_cast<int>(handle), &w, 1), 1);
        ASSERT_EQ(::lseek(static_cast<int>(handle), 0, SEEK_SET), 0);
        char r = 0;
        ASSERT_EQ(::read(static_cast<int>(handle), &r, 1), 1);
        EXPECT_EQ(r, w);
    }

    TEST_F(backing_file_create_test, FdHasCloseOnExecSet) {
        const std::string p = path("cloexec.img");

        const rcloud_testing::fd_handle handle{ backing_file_create(p.c_str()) };
        ASSERT_NE(static_cast<int>(handle), -1);

        const int flags = ::fcntl(static_cast<int>(handle), F_GETFD);
        ASSERT_NE(flags, -1);
        EXPECT_TRUE(flags & FD_CLOEXEC) << "O_CLOEXEC not set on returned fd";
    }

    TEST_F(backing_file_create_test, CreatesIntermediateDirectories) {
        const std::string p = path("a/b/c/deep.img");

        const rcloud_testing::fd_handle handle{ ::backing_file_create(p.c_str()) };

        ASSERT_NE(static_cast<int>(handle), -1) << "backing_file_create with nested dirs failed: " << ::strerror(errno);
        EXPECT_EQ(::access(p.c_str(), F_OK), EXIT_SUCCESS);
    }

    TEST_F(backing_file_create_test, NewFileIsEmpty) {
        const std::string p = path("empty.img");

        const rcloud_testing::fd_handle handle{ backing_file_create(p.c_str()) };
        ASSERT_NE(static_cast<int>(handle), -1);

        EXPECT_EQ(file_size_on_disk(p.c_str()), 0);
    }

    // ── Failure paths ────────────────────────────────────────────────────────────────────────────────────────────────

    TEST_F(backing_file_create_test, FailsWhenFileAlreadyExists) {
        const std::string p = path("exists.img");

        const rcloud_testing::fd_handle existing{ ::open(p.c_str(), O_CREAT | O_WRONLY, 0644) };
        ASSERT_NE(static_cast<int>(existing), -1);

        int raw_fd = backing_file_create(p.c_str());
        EXPECT_EQ(raw_fd, -1);
        EXPECT_EQ(errno, EEXIST);
    }

    TEST_F(backing_file_create_test, FailsOnReadOnlyParentDirectory) {
        if (::getuid() == 0) {
            GTEST_SKIP() << "Running as root; permission test not meaningful";
        }

        const std::string ro_dir = path("readonly");
        ASSERT_EQ(::mkdir(ro_dir.c_str(), 0555), EXIT_SUCCESS);

        int raw_fd = backing_file_create((ro_dir + "/denied.img").c_str());
        EXPECT_EQ(raw_fd, -1);
    }

    // ── backing_file_init ────────────────────────────────────────────────────────────────────────────────────────────

    class backing_file_init_test : public backing_file_test_base {
    protected:
        static void create_file_of_size(const std::string& p, ::off64_t size) {
            const rcloud_testing::fd_handle handle{ ::open(p.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644) };
            ASSERT_NE(static_cast<int>(handle), -1) << ::strerror(errno);
            if (size > 0) {
                ASSERT_EQ(::ftruncate64(static_cast<int>(handle), size), EXIT_SUCCESS) << ::strerror(errno);
            }
        }
    };

    TEST_F(backing_file_init_test, CreatesAndSizesNewFile) {
        const std::string p = path("new.img");
        constexpr ::off64_t target = 4096;

        const rcloud_testing::fd_handle handle{ ::backing_file_init(p.c_str(), target) };

        ASSERT_NE(static_cast<int>(handle), -1) << "backing_file_init failed: " << std::strerror(errno);
        EXPECT_EQ(::access(p.c_str(), F_OK), EXIT_SUCCESS);
        EXPECT_GE(file_size_on_disk(p.c_str()), target);
    }

    TEST_F(backing_file_init_test, NewFileReturnedFdIsReadWrite) {
        const std::string p = path("rw_new.img");

        const rcloud_testing::fd_handle handle{ backing_file_init(p.c_str(), 512) };
        ASSERT_NE(static_cast<int>(handle), -1);

        const char w = 0xAB;
        ASSERT_EQ(::write(static_cast<int>(handle), &w, 1), 1);
        ASSERT_EQ(::lseek(static_cast<int>(handle), 0, SEEK_SET), 0);
        char r = 0;
        ASSERT_EQ(::read(static_cast<int>(handle), &r, 1), 1);
        EXPECT_EQ(r, w);
    }

    TEST_F(backing_file_init_test, NewFileFdHasCloseOnExecSet) {
        const std::string p = path("cloexec_new.img");

        const rcloud_testing::fd_handle handle{ backing_file_init(p.c_str(), 512) };
        ASSERT_NE(static_cast<int>(handle), -1);

        const int flags = ::fcntl(static_cast<int>(handle), F_GETFD);
        ASSERT_NE(flags, -1);
        EXPECT_TRUE(flags & FD_CLOEXEC);
    }

    TEST_F(backing_file_init_test, ExistingFileLargerThanTargetIsReturnedAsIs) {
        const std::string p = path("big.img");
        constexpr ::off64_t existing = 8192;
        constexpr ::off64_t target   = 4096;

        create_file_of_size(p, existing);

        const rcloud_testing::fd_handle handle{ backing_file_init(p.c_str(), target) };
        ASSERT_NE(static_cast<int>(handle), -1);

        EXPECT_GE(file_size_on_disk(p.c_str()), existing);
    }

    TEST_F(backing_file_init_test, ExistingFileEqualToTargetIsReturnedAsIs) {
        const std::string p = path("exact.img");
        constexpr ::off64_t target = 4096;

        create_file_of_size(p, target);

        const rcloud_testing::fd_handle handle{ backing_file_init(p.c_str(), target) };
        ASSERT_NE(static_cast<int>(handle), -1);

        EXPECT_EQ(file_size_on_disk(p.c_str()), target);
    }

    TEST_F(backing_file_init_test, ExistingFileSmallerThanTargetIsExtended) {
        const std::string p = path("small.img");
        constexpr ::off64_t existing = 1024;
        constexpr ::off64_t target   = 4096;

        create_file_of_size(p, existing);

        const rcloud_testing::fd_handle handle{ backing_file_init(p.c_str(), target) };
        ASSERT_NE(static_cast<int>(handle), -1);

        EXPECT_GE(file_size_on_disk(p.c_str()), target);
    }

    TEST_F(backing_file_init_test, ExistingEmptyFileIsExtendedToTarget) {
        const std::string p = path("zero.img");
        constexpr ::off64_t target = 512;

        create_file_of_size(p, 0);

        const rcloud_testing::fd_handle handle{ backing_file_init(p.c_str(), target) };
        ASSERT_NE(static_cast<int>(handle), -1);

        EXPECT_GE(file_size_on_disk(p.c_str()), target);
    }

    TEST_F(backing_file_init_test, ExistingFileFdHasCloseOnExecSet) {
        const std::string p = path("cloexec_existing.img");
        create_file_of_size(p, 4096);

        const rcloud_testing::fd_handle handle{ backing_file_init(p.c_str(), 4096) };
        ASSERT_NE(static_cast<int>(handle), -1);

        const int flags = ::fcntl(static_cast<int>(handle), F_GETFD);
        ASSERT_NE(flags, -1);
        EXPECT_TRUE(flags & FD_CLOEXEC);
    }

    TEST_F(backing_file_init_test, FailsWhenParentDirectoryUnwritable) {
        if (::getuid() == 0) {
            GTEST_SKIP() << "Running as root; permission test not meaningful";
        }

        const std::string ro_dir = path("ro");
        ASSERT_EQ(::mkdir(ro_dir.c_str(), 0555), EXIT_SUCCESS);

        int raw_fd = backing_file_init((ro_dir + "/denied.img").c_str(), 4096);
        EXPECT_EQ(raw_fd, -1);
    }

    TEST_F(backing_file_init_test, FailsWhenExistingFileIsUnreadable) {
        if (::getuid() == 0) {
            GTEST_SKIP() << "Running as root; permission test not meaningful";
        }

        const std::string p = path("noperm.img");
        create_file_of_size(p, 4096);
        ASSERT_EQ(::chmod(p.c_str(), 0000), EXIT_SUCCESS);

        int raw_fd = backing_file_init(p.c_str(), 4096);
        EXPECT_EQ(raw_fd, -1);

        ::chmod(p.c_str(), 0644);
    }
} // namespace backing_file_testing