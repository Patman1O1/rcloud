#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO C++ Includes
#include <iostream>

// ISO C Includes
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

// POSIX Includes
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

// Google Test Includes
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Local Includes
#include "rcloud/loop.h"

namespace loop_testing {
    namespace {
        constexpr char BACKING_FILE[] = "/tmp/test_image.img";

        class LoopTest : public ::testing::Test {
        protected:
            void SetUp() noexcept override {
                // Create a child process
                if (const ::pid_t pid = ::fork(); pid == 0) {
                    // Open "/dev/null"
                    const int fd = ::open("/dev/null", O_WRONLY | O_CLOEXEC);
                    if (fd == -1) {
                        ::_exit(-1);
                    }

                    // Redirect stdout to the file descriptor
                    if (::dup2(fd, STDOUT_FILENO) == -1) {
                        ::_exit(EXIT_FAILURE);
                    }

                    // Redirect stderr to the file descriptor
                    if (::dup2(fd, STDERR_FILENO) == -1) {
                        ::_exit(EXIT_FAILURE);
                    }

                    // Close the original file descriptor
                    ::close(fd);

                    // Execute the command
                    ::execlp("dd", "dd", "if=/dev/zero", "of=/tmp/test_image.img", "bs=1M", "count=1", nullptr);

                    // Terminate the child process with exit code 1 to indicate a failure
                    ::_exit(EXIT_FAILURE);
                } else if (pid > 0) {
                    int status;
                    if (::waitpid(pid, &status, 0) == -1) {
                        GTEST_FAIL() << "waitpid: " << std::strerror(errno) << '\n';
                    }

                    if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS) {
                        GTEST_FAIL() << "waitpid: " << std::strerror(WEXITSTATUS(status));
                    }

                    if (WIFSIGNALED(status)) {
                        GTEST_FAIL() << "Child process terminated: " << ::strsignal(WTERMSIG(status)) << '\n';
                    }
                } else {
                    GTEST_FAIL() << "fork: " << std::strerror(errno);
                }
            }

            void TearDown() noexcept override {
                // Cleanup: Ensure device is detached if a test failed
                if (const int fd = ::loop_device_open(BACKING_FILE); fd >= 0) {
                    ::loop_device_destroy(fd);
                }
                ::unlink(BACKING_FILE);
            }
        };
    } // unnamed namespace

    TEST_F(LoopTest, InitAndOpenTest) {
        // Test Initialization
        const int fd = ::loop_device_init(BACKING_FILE);
        ASSERT_GE(fd, 0) << "Failed to initialize loop device: " << std::strerror(errno);

        // Test Lookup (Open)
        const int lookup_fd = ::loop_device_open(BACKING_FILE);
        EXPECT_GE(lookup_fd, 0) << "Failed to find existing loop device";

        ::close(lookup_fd);
        ::close(fd);
    }

    TEST_F(LoopTest, DestroyTest) {
        const int fd = ::loop_device_init(BACKING_FILE);
        ASSERT_GE(fd, 0);

        // Test Destruction
        const int res = ::loop_device_destroy(fd);
        EXPECT_EQ(res, 0) << "Failed to destroy loop device";

        // Verify it can no longer be opened
        const int lookup_fd = ::loop_device_open(BACKING_FILE);
        EXPECT_LT(lookup_fd, 0) << "Loop device still found after destruction";
    }

    TEST_F(LoopTest, InvalidPathTest) {
        const int fd = ::loop_device_init("/non/existent/path.img");
        EXPECT_LT(fd, 0);
    }
} // namespace loop_testing

