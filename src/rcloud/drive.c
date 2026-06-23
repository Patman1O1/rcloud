#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

#ifdef _XOPEN_SOURCE
#define _XOPEN_SOURCE_ORIGINAL _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif // #ifdef _XOPEN_SOURCE

#define _XOPEN_SOURCE 500

// ISO Includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// POSIX Includes
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

// GNU Includes
#include <linux/loop.h>

// Local Includes
#include "rcloud/drive.h"

// The name of the backing file.
static constexpr char BACKING_FILE_NAME[] = "rcloud.img";

// The absolute path to the Loop Control device.
static constexpr char LOOP_CTL_PATH[] = "/dev/loop-control";

// The maximum size the path to the loop device can be. Let num_digits_in_uint32_max
// be the number of digits in the stdint.h macro UINT32_MAX and let padding equal 4.
// Then LOOP_DEV_PATH_MAX is calculated via the formula below.
// LOOP_DEV_PATH_MAX = sizeof("/dev/loop") + num_digits_in_uint32_max + padding
static constexpr size_t LOOP_DEV_PATH_MAX = 24;

// The maximum size the name of the loop device can be. Let num_digits_in_uint32_max
// be the number of digits in the stdint.h macro UINT32_MAX and let padding equal 1.
// Then LOOP_DEV_NAME_MAX is calculated via the formula below.
// LOOP_DEV_NAME_MAX = sizeof("loop") + num_digits_in_uint32_max + padding
static constexpr size_t LOOP_DEV_NAME_MAX = 16;

static inline int nftw_func(const char* path, const struct stat*, int, struct FTW*) { return remove(path); }

static inline int loop_ctl_get_free(const int ctl_fd) { return ioctl(ctl_fd, LOOP_CTL_GET_FREE); }

static inline int loop_ctl_add(const int ctl_fd, const unsigned long int lo_number) {
    return ioctl(ctl_fd, LOOP_CTL_ADD, lo_number);
}

static inline int loop_ctl_rm(const int ctl_fd, const unsigned long int lo_number) {
    return ioctl(ctl_fd, LOOP_CTL_REMOVE, lo_number);
}

static inline int loop_config(const int loop_fd, const struct loop_config* lo_cfg) {
    return ioctl(loop_fd, LOOP_CONFIGURE, lo_cfg);
}

static inline int loop_change_fd(const int loop_fd, const int bk_fd) {
    return ioctl(loop_fd, LOOP_CHANGE_FD, bk_fd);
}

static inline int loop_clr_fd(const int loop_fd) {
    const int status = ioctl(loop_fd, LOOP_CLR_FD, 0);
    close(loop_fd);
    return status;
}

static inline int loop_set_status64(const int loop_fd, const struct loop_info64* lo_info64) {
    return ioctl(loop_fd, LOOP_SET_STATUS64, lo_info64);
}

static inline int loop_get_status64(const int loop_fd, struct loop_info64* lo_info64) {
    return ioctl(loop_fd, LOOP_GET_STATUS64, lo_info64);
}

static inline int loop_set_block_size(const int loop_fd, const size_t block_size) {
    return ioctl(loop_fd, LOOP_SET_BLOCK_SIZE, block_size);
}

static inline int loop_set_direct_io(const int loop_fd, const bool direct_io_flag) {
    return ioctl(loop_fd, LOOP_SET_DIRECT_IO, direct_io_flag);
}

static inline int loop_set_capacity(const int loop_fd) { return ioctl(loop_fd, LOOP_SET_CAPACITY, 0); }

static inline ssize_t loop_block_size(const int loop_fd) {
    ssize_t block_size;
    return ioctl(loop_fd, BLKSSZGET, &block_size) >= 0 ? block_size : -1;
}

static int mkdirs(const char* path_p, const mode_t mode) {
    static struct stat stat_v;

    // Validate the pointer to the path and the path itself
    if (path_p == nullptr || *path_p == '\0') {
        errno = ENOENT;
        return -1;
    }

    // Create a copy of the path
    char* path_cpy_p = strdup(path_p);
    if (path_cpy_p == nullptr) {
        return -1;
    }

    // Drop trailing slashes to avoid creating empty components
    size_t path_len = strlen(path_p);
    while (path_len > 1 && path_cpy_p[path_len - 1] == '/') {
        path_cpy_p[path_len--] = '\0';
    }

    for (char* char_p = path_cpy_p + 1; ; char_p++) {
        if (*char_p != '/' && *char_p != '\0') {
            continue;
        }

        const char sep = *char_p;
        *char_p = '\0';

        if (mkdir(path_cpy_p, mode) == -1) {
            if (errno != EEXIST || stat(path_cpy_p, &stat_v) == -1) {
                free(path_cpy_p);
                return -1;
            }

            if (!S_ISDIR(stat_v.st_mode)) {
                errno = ENOTDIR;
                free(path_cpy_p);
                return -1;
            }
        }

        if (sep == '\0') {
            break;
        }
        *char_p = '/';
    }

    free(path_cpy_p);
    return EXIT_SUCCESS;
}

static bool is_mounted(const char* dev_path) {
    if (dev_path == nullptr) {
        errno = EFAULT;
        return false;
    }

    // Get the device's metadata
    struct stat dev_st;
    if (stat(dev_path, &dev_st) == -1) {
        return false;
    }

    // Open /proc/mounts
    const int fd = open("/proc/mounts", O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        return false;
    }

    char buffer[4096];
    ssize_t bytes_read;

    // Read chunks until the end of the file has been reached
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        // Null terminate
        buffer[bytes_read] = '\0';

        // Get the next line in the file
        char* line = strtok(buffer, "\n");
        while (line != nullptr) {
            // Extract the device column (first string)
            const char* mount_dev = strtok(line, " \t");
            if (mount_dev != nullptr) {
                // Use stat to verify the device node
                struct stat mount_st;
                if (stat(mount_dev, &mount_st) == EXIT_SUCCESS && dev_st.st_rdev == mount_st.st_rdev) {
                    close(fd);
                    return true;
                }
            }

            // Get the next line in the file
            line = strtok(nullptr, "\n");
        }
    }

    close(fd);
    return false;
}

static int mkfs_ext4(const char* dev_path) {
    const pid_t pid = fork();
    if (pid == -1) {
        return -1;
    }

    if (pid == 0) {
        execlp("mkfs.ext4", "mkfs.ext4", "-q", "-F", "-m", "0", "-b", "4096", dev_path, (char*)nullptr);
        _exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        return -1;
    }

    return WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS ? EXIT_SUCCESS : -1;
}

static char* gethome(char* result_buf, const size_t result_size) {
    if (result_buf == nullptr) {
        errno = EFAULT;
        return nullptr;
    }

    struct passwd pw;
    struct passwd* result;

    // Allocate temporary space for the database lookup
    char buffer[1024];

    if (getpwuid_r(getuid(), &pw, buffer, sizeof(buffer), &result) != 0 || result == nullptr) {
        return nullptr;
    }

    // Ensure the result buffer is large enough to hold the user's home directory path
    const size_t pw_dir_len = strlen(pw.pw_dir);
    if (pw_dir_len + 1 > result_size) {
        errno = ENAMETOOLONG;
        return nullptr;
    }

    // Copy the home directory into the result buffer
    memcpy(result_buf, pw.pw_dir, pw_dir_len);
    result_buf[pw_dir_len] = '\0';
    return result_buf;
}

