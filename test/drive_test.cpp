#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO C++ Includes
#include <string>

// ISO C Includes
#include <cstdlib>
#include <cstring>
#include <cerrno>

// POSIX Includes
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

// GNU Includes
#include <linux/limits.h>

// Google Test Includes
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Local Includes
#include "rcloud/drive.h"

namespace rcloud_drive_testing {
    namespace {
        // ── Functions ────────────────────────────────────────────────────────────────────────────────────────────────
        bool path_exists(const char* path) noexcept {
            struct ::stat st;
            return ::stat(path, &st) == EXIT_SUCCESS;
        }

        bool is_file(const char* path) noexcept {
            struct ::stat st;
            return ::stat(path, &st) == EXIT_SUCCESS && S_ISREG(st.st_mode);
        }

        bool is_dir(const char* path) noexcept {
            struct ::stat st;
            return ::stat(path, &st) == 0 && S_ISDIR(st.st_mode);
        }

        // Returns the real home directory via getpwuid_r into a caller-owned
        // malloc'd buffer. Caller must free().
        char* real_home() noexcept {
            const ::ssize_t potential_size = ::sysconf(_SC_GETPW_R_SIZE_MAX);
            const std::size_t size = potential_size == -1 ? 16384 : static_cast<std::size_t>(potential_size);
            auto const buf = static_cast<char*>(std::malloc(size));
            if (buf == nullptr) {
                return nullptr;
            }

            struct ::passwd pw;
            struct ::passwd* result;
            const char* username = std::getenv("SUDO_USER");
            const int ret_val = username != nullptr ? ::getpwnam_r(username, &pw, buf, size, &result)
                                                    : ::getpwuid_r(::getuid(), &pw, buf, size, &result);
            if (ret_val != EXIT_SUCCESS || result == nullptr) {
                std::free(buf);
                return nullptr;
            }

            // pw.pw_dir points into buf — copy it before freeing buf
            auto const home = static_cast<char*>(std::malloc(std::strlen(pw.pw_dir) + 1));
            if (home != nullptr) {
                std::strcpy(home, pw.pw_dir);
            }
            std::free(buf);
            return home;
        }

        // ── Classes  ─────────────────────────────────────────────────────────────────────────────────────────────────
        class env_guard {
        private:
            const char* name_;

            char* value_;

        public:
            env_guard() noexcept = delete;

            env_guard(const char* name, const char* value) noexcept : name_(name), value_(nullptr) {
                const char* cur = std::getenv(name);
                if (cur != nullptr) {
                    this->value_ = static_cast<char*>(std::malloc(std::strlen(cur) + 1));
                    if (this->value_ != nullptr) {
                        std::strcpy(this->value_, cur);
                    }
                }

                value != nullptr ? ::setenv(name, value, 1) : unsetenv(name);
            }

            env_guard(const env_guard&) noexcept = delete;

            env_guard(env_guard&&) noexcept = delete;

            ~env_guard() noexcept {
                if (this->value_ != nullptr) {
                    ::setenv(this->name_, this->value_, true);
                    std::free(this->value_);
                } else {
                    ::unsetenv(this->name_);
                }
            }

            env_guard& operator=(const env_guard&) noexcept = delete;

            env_guard& operator=(env_guard&&) noexcept = delete;
        };

        class rcloud_drive_path_test : public ::testing::Test {
        protected:
            char* path_{nullptr};

            void TearDown() override {
                std::free(this->path_);
                this->path_ = nullptr;
            }
        };

        class rcloud_drive_img_path_test : public rcloud_drive_path_test {};

        class rcloud_drive_mnt_path_test : public rcloud_drive_path_test {};

        class rcloud_drive_test : public ::testing::Test {
        protected:
            char* img_path_{nullptr};
            char* mnt_path_{nullptr};

            void SetUp() noexcept override {
                this->img_path_ = ::rcloud_drive_img_path();
                this->mnt_path_ = ::rcloud_drive_mnt_path();
                // Clean up any leftovers from prior test runs
                ::unlink(this->img_path_);
                ::rmdir(this->mnt_path_);
            }

            void TearDown() noexcept override {
                ::rcloud_drive_destroy(this->img_path_, this->mnt_path_);
                ::unlink(this->img_path_);
                ::rmdir(this->mnt_path_);
                std::free(this->img_path_);
                std::free(this->mnt_path_);
                this->img_path_ = nullptr;
                this->mnt_path_ = nullptr;
            }
        };

        class rcloud_drive_create_test : public rcloud_drive_test {};

        class rcloud_drive_destroy_test : public rcloud_drive_test {};

    } // anonymous namespace

    // ── rcloud_drive_img_path_test ───────────────────────────────────────────────────────────────────────────────────
    TEST_F(rcloud_drive_img_path_test, returns_non_null) {
        this->path_ = ::rcloud_drive_img_path();
        ASSERT_NE(this->path_, nullptr) << std::strerror(errno);
    }

    TEST_F(rcloud_drive_img_path_test, path_ends_with_rcloud_img) {
        this->path_ = ::rcloud_drive_img_path();
        ASSERT_NE(this->path_, nullptr) << std::strerror(errno);
        const std::size_t path_len = std::strlen(this->path_);
        EXPECT_GT(path_len, 11u);
        EXPECT_EQ(0, std::strncmp(this->path_ + path_len - 11, "/rcloud.img", 11))
            << "Actual path: " << this->path_;
    }

    TEST_F(rcloud_drive_img_path_test, path_within_xdg_data_home_when_set) {
        env_guard guard("XDG_DATA_HOME", "/tmp/rcloud_test_xdg");

        this->path_ = ::rcloud_drive_img_path();
        ASSERT_NE(this->path_, nullptr) << std::strerror(errno);

        EXPECT_STREQ("/tmp/rcloud_test_xdg/rcloud/rcloud.img", this->path_);
        ::rmdir("/tmp/rcloud_test_xdg/rcloud");
        ::rmdir("/tmp/rcloud_test_xdg");
    }

    TEST_F(rcloud_drive_img_path_test, path_within_default_xdg_when_env_unset) {
        env_guard guard("XDG_DATA_HOME", nullptr);

        this->path_ = ::rcloud_drive_img_path();
        ASSERT_NE(this->path_, nullptr) << std::strerror(errno);

        char* home = real_home();
        ASSERT_NE(nullptr, home) << std::strerror(errno);

        char expected_path[PATH_MAX];
        ASSERT_LT(std::snprintf(expected_path, PATH_MAX,
                                "%s/.local/share/rcloud/rcloud.img", home),
                  PATH_MAX);
        std::free(home);

        EXPECT_STREQ(expected_path, this->path_);
    }

    TEST_F(rcloud_drive_img_path_test, rcloud_directory_is_created) {
        env_guard guard("XDG_DATA_HOME", "/tmp/rcloud_imgdir_test");

        this->path_ = ::rcloud_drive_img_path();
        ASSERT_NE(this->path_, nullptr) << std::strerror(errno);

        EXPECT_TRUE(is_dir("/tmp/rcloud_imgdir_test/rcloud"));
        ::rmdir("/tmp/rcloud_imgdir_test/rcloud");
        ::rmdir("/tmp/rcloud_imgdir_test");
    }

    TEST_F(rcloud_drive_img_path_test, returned_pointer_is_freeable) {
        this->path_ = ::rcloud_drive_img_path();
        ASSERT_NE(nullptr, this->path_) << std::strerror(errno);
        // TearDown will free; test just verifies no crash/asan report on free
    }

    TEST_F(rcloud_drive_img_path_test, path_does_not_exceed_path_max) {
        this->path_ = ::rcloud_drive_img_path();
        ASSERT_NE(nullptr, this->path_) << std::strerror(errno);
        EXPECT_LT(std::strlen(this->path_), static_cast<std::size_t>(PATH_MAX));
    }

    // ── rcloud_drive_mnt_path_test ───────────────────────────────────────────────────────────────────────────────────
    TEST_F(rcloud_drive_mnt_path_test, returns_non_null) {
        this->path_ = ::rcloud_drive_mnt_path();
        ASSERT_NE(nullptr, this->path_) << std::strerror(errno);
    }

    TEST_F(rcloud_drive_mnt_path_test, path_ends_with_rcloud) {
        this->path_ = ::rcloud_drive_mnt_path();
        ASSERT_NE(nullptr, this->path_) << std::strerror(errno);
        const std::size_t path_len = std::strlen(this->path_);
        EXPECT_GT(path_len, 7u);
        EXPECT_EQ(0, std::strncmp(this->path_ + path_len - 7, "/rcloud", 7))
            << "Actual path: " << this->path_;
    }

    TEST_F(rcloud_drive_mnt_path_test, path_begins_with_home_dir) {
        char* home = real_home();
        ASSERT_NE(nullptr, home) << std::strerror(errno);

        this->path_ = ::rcloud_drive_mnt_path();
        ASSERT_NE(nullptr, this->path_) << std::strerror(errno);

        EXPECT_EQ(0, std::strncmp(this->path_, home, std::strlen(home)))
            << "Expected prefix " << home << " in " << this->path_;
        std::free(home);
    }

    TEST_F(rcloud_drive_mnt_path_test, returned_pointer_is_freeable) {
        this->path_ = ::rcloud_drive_mnt_path();
        ASSERT_NE(nullptr, this->path_) << std::strerror(errno);
        // TearDown will free
    }

    TEST_F(rcloud_drive_mnt_path_test, path_does_not_exceed_path_max) {
        this->path_ = ::rcloud_drive_mnt_path();
        ASSERT_NE(nullptr, this->path_) << std::strerror(errno);
        EXPECT_LT(std::strlen(this->path_), static_cast<std::size_t>(PATH_MAX));
    }

    // ── rcloud_drive_create_test ─────────────────────────────────────────────────────────────────────────────────────
    TEST_F(rcloud_drive_create_test, fails_with_non_existent_mnt_parent) {
        const int ret = ::rcloud_drive_create(
            this->img_path_,
            "/tmp/no_such_parent/rcloud_mnt");
        EXPECT_EQ(-1, ret);
    }

    TEST_F(rcloud_drive_create_test, creates_image_file) {
        ASSERT_EQ(EXIT_SUCCESS, ::rcloud_drive_create(this->img_path_, this->mnt_path_))
            << std::strerror(errno);
        EXPECT_TRUE(is_file(this->img_path_));
    }

    TEST_F(rcloud_drive_create_test, creates_mount_directory) {
        ASSERT_EQ(EXIT_SUCCESS, ::rcloud_drive_create(this->img_path_, this->mnt_path_))
            << std::strerror(errno);
        EXPECT_TRUE(is_dir(this->mnt_path_));
    }

    TEST_F(rcloud_drive_create_test, idempotent_when_already_mounted) {
        ASSERT_EQ(EXIT_SUCCESS, ::rcloud_drive_create(this->img_path_, this->mnt_path_))
            << std::strerror(errno);
        EXPECT_EQ(EXIT_SUCCESS, ::rcloud_drive_create(this->img_path_, this->mnt_path_));
    }

    TEST_F(rcloud_drive_create_test, mnt_dir_removed_on_failure) {
        // Create a corrupt (non-ext4) image file
        {
            const int fd = ::open(this->img_path_, O_RDWR | O_CREAT | O_EXCL, 0644);
            ASSERT_NE(-1, fd) << std::strerror(errno);
            constexpr char zeros[512] = {};
            ::write(fd, zeros, sizeof(zeros));
            ::close(fd);
        }

        const int ret = ::rcloud_drive_create(this->img_path_, this->mnt_path_);
        EXPECT_EQ(-1, ret);
        EXPECT_FALSE(path_exists(this->mnt_path_));
    }

    // ── rcloud_drive_destroy_test ────────────────────────────────────────────────────────────────────────────────────
    TEST_F(rcloud_drive_destroy_test, destroy_removes_image_when_not_mounted) {
        {
            const int fd = ::open(this->img_path_, O_RDWR | O_CREAT | O_EXCL, 0644);
            ASSERT_NE(-1, fd) << std::strerror(errno);
            ::close(fd);
        }

        EXPECT_EQ(EXIT_SUCCESS, ::rcloud_drive_destroy(this->img_path_, this->mnt_path_));
        EXPECT_FALSE(path_exists(this->img_path_));
    }

    TEST_F(rcloud_drive_destroy_test, destroy_fails_if_image_missing_and_not_mounted) {
        EXPECT_EQ(-1, ::rcloud_drive_destroy(this->img_path_, this->mnt_path_));
        EXPECT_EQ(ENOENT, errno);
    }

    TEST_F(rcloud_drive_destroy_test, destroy_unmounts_and_removes_image) {
        ASSERT_EQ(EXIT_SUCCESS, ::rcloud_drive_create(this->img_path_, this->mnt_path_))
            << std::strerror(errno);
        ASSERT_EQ(EXIT_SUCCESS, ::rcloud_drive_destroy(this->img_path_, this->mnt_path_))
            << std::strerror(errno);
        EXPECT_FALSE(path_exists(this->img_path_));
    }

    TEST_F(rcloud_drive_destroy_test, destroy_leaves_no_mount_after_teardown) {
        ASSERT_EQ(EXIT_SUCCESS, ::rcloud_drive_create(this->img_path_, this->mnt_path_))
            << std::strerror(errno);
        ASSERT_EQ(EXIT_SUCCESS, ::rcloud_drive_destroy(this->img_path_, this->mnt_path_))
            << std::strerror(errno);

        // Read mountinfo line by line and check mnt_path_ is absent
        FILE* f = std::fopen("/proc/self/mountinfo", "r");
        ASSERT_NE(nullptr, f) << std::strerror(errno);
        char line[4096];
        bool still_mounted = false;
        while (std::fgets(line, sizeof(line), f) != nullptr) {
            if (std::strstr(line, this->mnt_path_) != nullptr) {
                still_mounted = true;
                break;
            }
        }
        std::fclose(f);
        EXPECT_FALSE(still_mounted);
    }
} // namespace rcloud_drive_testing
