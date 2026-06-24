#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO Includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// POSIX Includes
#include <unistd.h>
#include <pwd.h>
#include <sys/mount.h>

// Local Includes
#include "rcloud/fs.h"
#include "rcloud/loop.h"
#include "rcloud/drive.h"

// The maximum size the user's home path is allowed to be. This is calculated by using
// the formula below. Note that NAME_MAX is a macro defined in "linux/limits.h" with a
// value of 255. Let <padding> be the value 8.
//
// HOME_PATH_MAX = NAME_MAX + strlen("/home/") + strlen("rcloud.img") + <padding>
static constexpr size_t HOME_PATH_MAX = 280;

// The maximum size of the path to the backing file can be. This is calculated by
// using the formula below. Let <padding> be the value 1.
//
// BACKING_PATH_MAX = HOME_PATH_MAX + strlen(".local/share/rcloud/rcloud.img") + <padding>
static constexpr size_t BACKING_PATH_MAX = 312;

// The maximum size the mount path can be. This is calculated by using the formula
// below. Let <padding> be the value 2.
//
// MNT_PATH_MAX = HOME_PATH_MAX + strlen("rcloud") + <padding>
static constexpr size_t MNT_PATH_MAX = 288;

// The initialize size of the backing file (1 GiB).
static constexpr size_t BACKING_FILE_INIT_SIZE = 1074000000;

static int get_home_path(char* result_buf) {
    struct passwd pw;
    struct passwd* result;

    // Allocate temporary space for the database lookup
    char buffer[1024];

    if (getpwuid_r(getuid(), &pw, buffer, sizeof(buffer), &result) != 0 || result == nullptr) {
        return -1;
    }

    // Ensure the result buffer is large enough to hold the user's home directory path
    const size_t pw_dir_len = strlen(pw.pw_dir);
    if (pw_dir_len + 1 > HOME_PATH_MAX) {
        errno = ENAMETOOLONG;
        return -1;
    }

    // Copy the home directory into the result buffer
    memcpy(result_buf, pw.pw_dir, pw_dir_len);
    result_buf[pw_dir_len] = '\0';
    return EXIT_SUCCESS;
}

int create_drive(void) {
    // Get the absolute path to the user's home directory
    char home_path[HOME_PATH_MAX];
    if (get_home_path(home_path) == -1) {
        return -1;
    }

    // Construct the absolute path to the backing file
    char backing_path[BACKING_PATH_MAX];
    snprintf(backing_path, sizeof(backing_path), "%s/.local/share/rcloud/rcloud.img", home_path);

    // Initialize the backing file
    const int backing_fd = backing_file_init(backing_path, BACKING_FILE_INIT_SIZE);
    if (backing_fd == -1) {
        return -1;
    }

    // Initialize the loop device
    char loop_dev_path[LOOP_DEV_PATH_MAX];
    const int loop_fd = loop_device_init(backing_fd, loop_dev_path);
    if (loop_fd == -1) {
        backing_file_remove(backing_path);
        return -1;
    }

    // Create the filesystem
    if (mkfs_ext4(loop_dev_path) == -1) {
        loop_clr_fd(loop_fd);
        backing_file_remove(backing_path);
        return -1;
    }

    // Construct the absolute path to the mount point
    char mnt_path[MNT_PATH_MAX];
    snprintf(mnt_path, MNT_PATH_MAX, "%s/rcloud", backing_path);

    // Mount the drive
    return mount(loop_dev_path, mnt_path, "ext4",
        MS_NOATIME | MS_NOSUID | MS_NODEV, nullptr);
}

int destroy_drive(void) {
    // Get the absolute path to the user's home directory
    char home_path[HOME_PATH_MAX];
    if (get_home_path(home_path) == -1) {
        return -1;
    }

    // Construct the absolute path to the mount point
    char mnt_path[MNT_PATH_MAX];
    snprintf(mnt_path, MNT_PATH_MAX, "%s/rcloud", home_path);

    // Unmount the loop device
    if (umount2(mnt_path, 0) == -1) {
        return -1;
    }

    // Construct the absolute path to the backing file
    char backing_path[BACKING_PATH_MAX];
    snprintf(backing_path, BACKING_PATH_MAX, "%s/.local/share/rcloud/rcloud.img", backing_path);

    // Open the backing file
    const int backing_fd = open(backing_path, O_RDWR | O_CLOEXEC);
    if (backing_fd == -1) {
        return -1;
    }

    // Disassociate the loop device with its backing file
    if (loop_clr_fd(backing_fd) == -1) {
        close(backing_fd);
        return -1;
    }

    // Remove the backing file
    if (backing_file_remove(backing_path) == -1) {
        close(backing_fd);
        return -1;
    }

    return EXIT_SUCCESS;
}