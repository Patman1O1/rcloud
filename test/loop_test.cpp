#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO C++ Includes
#include <string>
#include <string_view>
#include <system_error>

// ISO C Includes
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

// POSIX Includes
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

// Google Test Includes
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Local Includes
#include "rcloud/loop.h"
#include "fd_handle.hpp"
#include "file_handle.hpp"
#include "dir_handle.hpp"

#define LOOP_TEST_TMPFILE "/tmp/rcloud_loop_test_XXXXXX"

namespace loop_testing {
    namespace {
        // Helper to detach the backing file from the loop device cleanly during teardown
        inline void clear_loop_device(int loop_fd) noexcept {
            if (loop_fd >= 0) {
                ::ioctl(loop_fd, LOOP_CLR_FD, 0);
            }
        }
    } // anonymous namespace

    // ── loop_test_base ───────────────────────────────────────────────────────────────────────────────────────────────
    class loop_test : public ::testing::Test {
    protected:
        std::string backing_path_;
        int backing_fd_ = -1;

        void SetUp() override {
            // Loop device administration (LOOP_CTL_GET_FREE, LOOP_CONFIGURE) requires root privileges
            if (::getuid() != 0) {
                GTEST_SKIP() << "Running as non-root; loop device configuration tests require CAP_SYS_ADMIN";
            }

            char tmpl[] = LOOP_TEST_TMPFILE;
            backing_fd_ = ::mkstemp(tmpl);
            ASSERT_NE(backing_fd_, -1) << "mkstemp failed: " << ::strerror(errno);
            backing_path_ = tmpl;

            // Allocate 16KB for the backing file
            ASSERT_EQ(::ftruncate64(backing_fd_, 4096 * 4), EXIT_SUCCESS) << ::strerror(errno);
        }

        void TearDown() override {
            if (backing_fd_ != -1) {
                ::close(backing_fd_);
            }
            if (!backing_path_.empty()) {
                ::unlink(backing_path_.c_str());
            }
        }
    };

    // ── loop_device_init ─────────────────────────────────────────────────────────────────────────────────────────────

    TEST_F(loop_test, InitBindsBackingFileToFreeLoopDevice) {
        char loop_path[LOOP_DEV_PATH_MAX] = {};

        // dup() the fd because loop_device_init takes ownership and closes it
        const int fd_copy = ::dup(backing_fd_);
        ASSERT_NE(fd_copy, -1);

        const int loop_fd = ::loop_device_init(fd_copy, loop_path);
        ASSERT_NE(loop_fd, -1) << "loop_device_init failed: " << ::strerror(errno);

        // Wrap in fd_handle for automatic closure
        const rcloud_testing::fd_handle handle{loop_fd};

        EXPECT_TRUE(std::string_view(loop_path).starts_with("/dev/loop"))
            << "Expected path to start with /dev/loop, got: " << loop_path;

        // Verify block size was successfully set to 4096 by loop_device_init

        const ssize_t blk_size = ::loop_block_size(static_cast<int>(handle));
        EXPECT_EQ(blk_size, 4096) << "";

        // Cleanup the loop mapping before RAII closes the file descriptor
        clear_loop_device(static_cast<int>(handle));
    }

    TEST_F(loop_test, InitFailsWithInvalidFileDescriptor) {
        char loop_path[LOOP_DEV_PATH_MAX] = {};

        // Pass -1 to simulate a bad file descriptor.
        // loop_device_init will fail to configure and safely close the fd.
        const int loop_fd = ::loop_device_init(-1, loop_path);
        EXPECT_EQ(loop_fd, -1);
    }

    // ── loop_device_open ─────────────────────────────────────────────────────────────────────────────────────────────

    TEST_F(loop_test, OpenRetrievesExistingLoopDeviceByBackingFilePath) {
        char loop_path[LOOP_DEV_PATH_MAX] = {};

        const int fd_copy = ::dup(backing_fd_);
        ASSERT_NE(fd_copy, -1);

        const int init_loop_fd = ::loop_device_init(fd_copy, loop_path);
        ASSERT_NE(init_loop_fd, -1) << "Prerequisite loop_device_init failed";
        const rcloud_testing::fd_handle init_handle{init_loop_fd};

        // Act: Attempt to resolve and open the loop device solely via the backing file path
        const int open_loop_fd = ::loop_device_open(backing_path_.c_str());
        EXPECT_NE(open_loop_fd, -1) << "Failed to find loop device for backing file";

        if (open_loop_fd != -1) {
            const rcloud_testing::fd_handle open_handle{open_loop_fd};

            // Verify both file descriptors refer to the exact same block device
            struct ::stat64 st_init, st_open;
            ASSERT_EQ(::fstat64(static_cast<int>(init_handle), &st_init), EXIT_SUCCESS);
            ASSERT_EQ(::fstat64(static_cast<int>(open_handle), &st_open), EXIT_SUCCESS);

            EXPECT_EQ(st_init.st_rdev, st_open.st_rdev) << "File descriptors do not point to the same device";
        }

        // Detach loop mapping
        clear_loop_device(static_cast<int>(init_handle));
    }

    TEST_F(loop_test, OpenReturnsErrorForNullptrPath) {
        const int fd = ::loop_device_open(nullptr);
        EXPECT_EQ(fd, -1);
        EXPECT_EQ(errno, EFAULT);
    }

    TEST_F(loop_test, OpenReturnsErrorForUnmappedBackingFile) {
        // Create an isolated file that is explicitly NOT mapped to any loop device
        rcloud_testing::file_handle dummy_file{"/tmp/rcloud_dummy_unmapped_XXXXXX"};
        const int dummy_fd = ::mkstemp(const_cast<char*>(static_cast<const char*>(dummy_file)));
        ASSERT_NE(dummy_fd, -1);
        ::close(dummy_fd);

        const int fd = ::loop_device_open(static_cast<const char*>(dummy_file));
        EXPECT_EQ(fd, -1);
    }

} // namespace loop_testing