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

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace {

// RAII wrapper: unsets an env var on destruction and restores the old value.
struct EnvGuard {
    std::string key;
    std::string old_val;
    bool        had_old;

    EnvGuard(const char* k, const char* v) : key(k) {
        const char* cur = getenv(k);
        had_old = (cur != nullptr);
        if (had_old) old_val = cur;
        if (v) setenv(k, v, 1);
        else   unsetenv(k);
    }
    ~EnvGuard() {
        if (had_old) setenv(key.c_str(), old_val.c_str(), 1);
        else         unsetenv(key.c_str());
    }
};

// Returns true if path exists (any file type).
bool path_exists(const char* p) {
    struct stat st;
    return stat(p, &st) == 0;
}

// Returns true if path is a regular file.
bool is_file(const char* p) {
    struct stat st;
    return stat(p, &st) == 0 && S_ISREG(st.st_mode);
}

// Returns true if path is a directory.
bool is_dir(const char* p) {
    struct stat st;
    return stat(p, &st) == 0 && S_ISDIR(st.st_mode);
}

// Retrieves the real home directory via getpwuid_r (no env fallback).
std::string real_home() {
    const long sz   = sysconf(_SC_GETPW_R_SIZE_MAX);
    const size_t n  = (sz == -1) ? 16384 : static_cast<size_t>(sz);
    std::string buf(n, '\0');
    struct passwd pw, *res;
    getpwuid_r(getuid(), &pw, buf.data(), n, &res);
    return res ? std::string(pw.pw_dir) : std::string();
}

} // namespace

// ===========================================================================
// rcloud_drive_img_path
// ===========================================================================

class DriveImgPathTest : public ::testing::Test {
protected:
    void TearDown() override {
        // Best-effort cleanup of any directory created under XDG_DATA_HOME.
        const char* xdg = getenv("XDG_DATA_HOME");
        if (xdg) {
            std::string dir = std::string(xdg) + "/rcloud";
            rmdir(dir.c_str());
        }
    }
};

TEST_F(DriveImgPathTest, ReturnsNonNull) {
    char* p = rcloud_drive_img_path();
    ASSERT_NE(p, nullptr);
    free(p);
}

TEST_F(DriveImgPathTest, PathEndsWithRcloudImg) {
    char* p = rcloud_drive_img_path();
    ASSERT_NE(p, nullptr);
    const std::string s(p);
    free(p);
    EXPECT_TRUE(s.size() > 11 &&
                s.compare(s.size() - 11, 11, "/rcloud.img") == 0)
        << "path was: " << s;
}

TEST_F(DriveImgPathTest, PathWithinXdgDataHomeWhenSet) {
    EnvGuard g("XDG_DATA_HOME", "/tmp/rcloud_test_xdg");

    char* p = rcloud_drive_img_path();
    ASSERT_NE(p, nullptr);
    const std::string s(p);
    free(p);

    EXPECT_EQ(s, "/tmp/rcloud_test_xdg/rcloud/rcloud.img");
    rmdir("/tmp/rcloud_test_xdg/rcloud");
    rmdir("/tmp/rcloud_test_xdg");
}

TEST_F(DriveImgPathTest, PathWithinDefaultXdgWhenEnvUnset) {
    EnvGuard g("XDG_DATA_HOME", nullptr);

    char* p = rcloud_drive_img_path();
    ASSERT_NE(p, nullptr);
    const std::string s(p);
    free(p);

    const std::string home = real_home();
    ASSERT_FALSE(home.empty());
    const std::string expected = home + "/.local/share/rcloud/rcloud.img";
    EXPECT_EQ(s, expected);
}

TEST_F(DriveImgPathTest, RcloudDirectoryIsCreated) {
    EnvGuard g("XDG_DATA_HOME", "/tmp/rcloud_imgdir_test");

    char* p = rcloud_drive_img_path();
    ASSERT_NE(p, nullptr);
    free(p);

    EXPECT_TRUE(is_dir("/tmp/rcloud_imgdir_test/rcloud"));
    rmdir("/tmp/rcloud_imgdir_test/rcloud");
    rmdir("/tmp/rcloud_imgdir_test");
}

TEST_F(DriveImgPathTest, ReturnedPointerIsFreeable) {
    char* p = rcloud_drive_img_path();
    ASSERT_NE(p, nullptr);
    // If double-free or corruption occurs this will crash/asan-report.
    free(p);
}

TEST_F(DriveImgPathTest, PathDoesNotExceedPathMax) {
    char* p = rcloud_drive_img_path();
    ASSERT_NE(p, nullptr);
    const size_t len = strlen(p);
    free(p);
    EXPECT_LT(len, static_cast<size_t>(PATH_MAX));
}

// ===========================================================================
// rcloud_drive_mnt_path
// ===========================================================================

TEST(DriveMntPathTest, ReturnsNonNull) {
    char* p = rcloud_drive_mnt_path();
    ASSERT_NE(p, nullptr);
    free(p);
}

TEST(DriveMntPathTest, PathEndsWithRcloud) {
    char* p = rcloud_drive_mnt_path();
    ASSERT_NE(p, nullptr);
    const std::string s(p);
    free(p);
    EXPECT_TRUE(s.size() > 7 &&
                s.compare(s.size() - 7, 7, "/rcloud") == 0)
        << "path was: " << s;
}

TEST(DriveMntPathTest, PathBeginsWithHomeDir) {
    const std::string home = real_home();
    ASSERT_FALSE(home.empty());

    char* p = rcloud_drive_mnt_path();
    ASSERT_NE(p, nullptr);
    const std::string s(p);
    free(p);

    EXPECT_EQ(s.compare(0, home.size(), home), 0)
        << "expected prefix " << home << " in " << s;
}

TEST(DriveMntPathTest, ReturnedPointerIsFreeable) {
    char* p = rcloud_drive_mnt_path();
    ASSERT_NE(p, nullptr);
    free(p);
}

TEST(DriveMntPathTest, PathDoesNotExceedPathMax) {
    char* p = rcloud_drive_mnt_path();
    ASSERT_NE(p, nullptr);
    const size_t len = strlen(p);
    free(p);
    EXPECT_LT(len, static_cast<size_t>(PATH_MAX));
}

// ===========================================================================
// rcloud_drive_create / rcloud_drive_destroy
// (These require root to actually mount; the structural / pre-mount paths are
//  testable without root.)
// ===========================================================================

// ===========================================================================
// rcloud_drive_create / rcloud_drive_destroy
// ===========================================================================

class DriveCreateDestroyTest : public ::testing::Test {
protected:
    std::string img_path;
    std::string mnt_path;

    void SetUp() override {
        img_path = "/tmp/rcloud_test.img";
        mnt_path = "/tmp/rcloud_test_mnt";
        unlink(img_path.c_str());
        rmdir(mnt_path.c_str());
    }

    void TearDown() override {
        rcloud_drive_destroy(img_path.c_str(), mnt_path.c_str());
        unlink(img_path.c_str());
        rmdir(mnt_path.c_str());
    }
};

// --- rcloud_drive_create ---

TEST_F(DriveCreateDestroyTest, FailsWithNonExistentMntParent) {
    const int ret = rcloud_drive_create(
        img_path.c_str(),
        "/tmp/no_such_parent/rcloud_mnt");
    EXPECT_EQ(ret, -1);
}

TEST_F(DriveCreateDestroyTest, CreatesImageFile) {
    ASSERT_EQ(rcloud_drive_create(img_path.c_str(), mnt_path.c_str()), 0) << std::strerror(errno);
    EXPECT_TRUE(is_file(img_path.c_str()));
}

TEST_F(DriveCreateDestroyTest, CreatesMountDirectory) {
    ASSERT_EQ(rcloud_drive_create(img_path.c_str(), mnt_path.c_str()), 0) << std::strerror(errno);
    EXPECT_TRUE(is_dir(mnt_path.c_str()));
}

TEST_F(DriveCreateDestroyTest, IdempotentWhenAlreadyMounted) {
    ASSERT_EQ(rcloud_drive_create(img_path.c_str(), mnt_path.c_str()), 0) << std::strerror(errno);
    EXPECT_EQ(rcloud_drive_create(img_path.c_str(), mnt_path.c_str()), 0);
}

TEST_F(DriveCreateDestroyTest, MntDirRemovedOnFailure) {
    {
        int fd = open(img_path.c_str(), O_RDWR | O_CREAT | O_EXCL, 0644);
        ASSERT_NE(fd, -1) << std::strerror(errno);
        char zeros[512] = {};
        write(fd, zeros, sizeof(zeros));
        close(fd);
    }

    const int ret = rcloud_drive_create(img_path.c_str(), mnt_path.c_str());
    EXPECT_EQ(ret, -1);
    EXPECT_FALSE(path_exists(mnt_path.c_str()));
}

// --- rcloud_drive_destroy ---

TEST_F(DriveCreateDestroyTest, DestroyRemovesImageWhenNotMounted) {
    {
        int fd = open(img_path.c_str(), O_RDWR | O_CREAT | O_EXCL, 0644);
        ASSERT_NE(fd, -1) << std::strerror(errno);
        close(fd);
    }

    const int ret = rcloud_drive_destroy(img_path.c_str(), mnt_path.c_str());
    EXPECT_EQ(ret, 0);
    EXPECT_FALSE(path_exists(img_path.c_str()));
}

TEST_F(DriveCreateDestroyTest, DestroyFailsIfImageMissingAndNotMounted) {
    const int ret = rcloud_drive_destroy(img_path.c_str(), mnt_path.c_str());
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(errno, ENOENT);
}

TEST_F(DriveCreateDestroyTest, DestroyUnmountsAndRemovesImage) {
    ASSERT_EQ(rcloud_drive_create(img_path.c_str(), mnt_path.c_str()), 0) << std::strerror(errno);
    ASSERT_EQ(rcloud_drive_destroy(img_path.c_str(), mnt_path.c_str()), 0) << std::strerror(errno);
    EXPECT_FALSE(path_exists(img_path.c_str()));
}

TEST_F(DriveCreateDestroyTest, DestroyLeavesNoMountAfterTeardown) {
    ASSERT_EQ(rcloud_drive_create(img_path.c_str(), mnt_path.c_str()), 0) << std::strerror(errno);
    ASSERT_EQ(rcloud_drive_destroy(img_path.c_str(), mnt_path.c_str()), 0) << std::strerror(errno);

    FILE* f = fopen("/proc/self/mountinfo", "r");
    ASSERT_NE(f, nullptr);
    char line[4096];
    bool still_mounted = false;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, mnt_path.c_str())) {
            still_mounted = true;
            break;
        }
    }
    fclose(f);
    EXPECT_FALSE(still_mounted);
}