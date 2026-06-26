#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO Includes
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

// POSIX Includes
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

// GNU Includes
#include <linux/limits.h>

// Third Party Includes
#include <libmount/libmount.h>

#include "rcloud/drive.h"

static constexpr char RCLOUD_FSTYPE[] = "ext4";
static constexpr off_t RCLOUD_IMG_MB = 1074000000;

static int mkdirs(const char* path, const mode_t mode) {
    if (path == nullptr || path[0] == '\0') {
        errno = EINVAL;
        return -1;
    }

    char* path_cpy = strdup(path);
    if (path_cpy == nullptr) {
        return -1;
    }

    int ret = 0;
    for (char* ch = path_cpy + 1; *ch != '\0'; ch++) {
        if (*ch == '/') {
            *ch = '\0';
            if (mkdir(path_cpy, mode) < 0 && errno != EEXIST) {
                ret = -1;
                break;
            }
            *ch = '/';
        }
    }
    if (ret == EXIT_SUCCESS && mkdir(path_cpy, mode) < 0 && errno != EEXIST) {
        ret = -1;
    }

    free(path_cpy);
    return ret;
}

static bool path_is_mountpoint(const char* path) {
    struct libmnt_table* table = mnt_new_table_from_file("/proc/self/mountinfo");
    if (table == nullptr) {
        return false;
    }

    struct libmnt_cache* cache = mnt_new_cache();
    if (cache != nullptr) {
        mnt_table_set_cache(table, cache);
    }

    const struct libmnt_fs* fs = mnt_table_find_target(table, path, MNT_ITER_BACKWARD);

    mnt_unref_cache(cache);
    mnt_unref_table(table);

    return fs != nullptr;
}

static int mkfs_ext4(const char* img_path) {
    char* const argv[] = {"/sbin/mkfs.ext4", "-F", "-q", "--", (char*)img_path, nullptr};
    constexpr char* const envp[] = { nullptr };

    if (access("/sbin/mkfs.ext4", F_OK) == -1) {
        errno = ENOENT;
        return -1;
    }

    const pid_t pid = fork();
    if (pid < 0) {
        return -1;
    }

    if (pid == 0) {
        execve("/sbin/mkfs.ext4", argv, envp);
        _exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0) {
        return -1;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
        errno = EIO;
        return -1;
    }
    return EXIT_SUCCESS;
}

static char* get_xdg_data_home(void) {
    // Allocate memory on the heap to store the path to XDG_DATA_HOME
    auto path = (char*)malloc(PATH_MAX);
    if (path == nullptr) {
        return nullptr;
    }

    // First check the environment variable
    const char* env = getenv("XDG_DATA_HOME");
    if (env == nullptr) {
        // Fall back to ~/.local/share
        const ssize_t buf_size = sysconf(_SC_GETPW_R_SIZE_MAX);
        const size_t pw_buf_size = buf_size == -1 ? 16384 : (size_t)buf_size;
        const auto pw_buf = (char*)malloc(pw_buf_size);
        if (pw_buf == nullptr) {
            free(path);
            return nullptr;
        }
        struct passwd pw;
        struct passwd* result;
        const int ret_val = getpwuid_r(getuid(), &pw, pw_buf, pw_buf_size, &result);
        if (ret_val != EXIT_SUCCESS ||
            result == nullptr ||
            snprintf(path, PATH_MAX, "%s/.local/share", pw.pw_dir) >= PATH_MAX) {
            free(pw_buf);
            free(path);
            return nullptr;
        }
        free(pw_buf);
    } else if (strlcpy(path, env, PATH_MAX) >= PATH_MAX) {
        free(path);
        return nullptr;
    }

    // Free access heap memory
    const size_t path_size = strnlen(path, PATH_MAX - 1) + 1;
    const auto tmp = (char*)realloc(path, path_size);
    if (tmp == nullptr) {
        return path;
    }
    path = tmp;

    // Ensure the path exists
    if (mkdirs(path, 0700) == -1) {
        free(path);
        return nullptr;
    }

    return path;
}

static int create_image(const char* img_path) {
    const int fd = open(img_path, O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
    if (fd == -1) {
        return -1;
    }

    if (fallocate(fd, 0, 0, RCLOUD_IMG_MB) == -1) {
        const int errno_cpy = errno;
        close(fd);
        unlink(img_path);
        errno = errno_cpy;
        return -1;
    }
    close(fd);

    if (mkfs_ext4(img_path) == -1) {
        unlink(img_path);
        return -1;
    }

    return EXIT_SUCCESS;
}

static int add_to_fstab(const char* img_path, const char* mnt_path) {
    struct libmnt_table* table = mnt_new_table();
    if (table == nullptr) {
        return -1;
    }

    // Load the current fstab
    if (mnt_table_parse_fstab(table, "/etc/fstab") != EXIT_SUCCESS) {
        mnt_unref_table(table);
        return -1;
    }

    // Create a new entry object
    struct libmnt_fs* fs = mnt_new_fs();
    if (fs == nullptr) {
        mnt_unref_table(table);
        return -1;
    }

    // Set the filesystem attributes
    mnt_fs_set_source(fs, img_path);
    mnt_fs_set_target(fs, mnt_path);
    mnt_fs_set_fstype(fs, RCLOUD_FSTYPE);
    mnt_fs_set_options(fs, "loop,noauto,user,rw");
    mnt_fs_set_passno(fs, 0);
    mnt_fs_set_freq(fs, 0);

    //  Add the entry to the table
    if (mnt_table_add_fs(table, fs) != EXIT_SUCCESS) {
        mnt_unref_fs(fs); // Only free if it was NOT added to the table
        mnt_unref_table(table);
        return -1;
    }

    // Save the table back to the fstab_path
    if (mnt_table_replace_file(table, "/etc/fstab") != EXIT_SUCCESS) {
        mnt_unref_table(table); // This frees the table AND the contained fs
        return -1;
    }

    mnt_unref_table(table);
    return EXIT_SUCCESS;
}

static int rm_from_fstab(const char* img_path) {
    struct libmnt_table* table = mnt_new_table();

    if (mnt_table_parse_fstab(table, "/etc/fstab") != EXIT_SUCCESS) {  // was nullptr
        mnt_unref_table(table);
        return -1;
    }

    struct libmnt_fs* fs = mnt_table_find_srcpath(table, img_path, MNT_ITER_BACKWARD);

    if (fs != nullptr) {
        mnt_table_remove_fs(table, fs);

        if (mnt_table_replace_file(table, "/etc/fstab") != EXIT_SUCCESS) {
            mnt_unref_table(table);
            return -1;
        }
    }

    mnt_unref_table(table);
    return EXIT_SUCCESS;
}

char* rcloud_drive_img_path(void) {
    // Allocate memory on the heap to store the image path
    const auto img_path = (char*)malloc(PATH_MAX);
    if (img_path == nullptr) {
        return nullptr;
    }

    // Get the XDG_DATA_HOME path
    char* xdg_data_home = get_xdg_data_home();
    if (xdg_data_home == nullptr) {
        free(img_path);
        return nullptr;
    }

    // Build the directory path and ensure it exists
    if (snprintf(img_path, PATH_MAX, "%s/rcloud", xdg_data_home) >= PATH_MAX ||
        mkdirs(img_path, 0700) == -1) {
        free(img_path);
        free(xdg_data_home);
        return nullptr;
    }

    // Build the full image path using xdg_data_home to avoid buffer overlap
    if (snprintf(img_path, PATH_MAX, "%s/rcloud/rcloud.img", xdg_data_home) >= PATH_MAX) {
        free(img_path);
        free(xdg_data_home);
        return nullptr;
    }

    free(xdg_data_home);

    // Free access heap memory
    const auto tmp = (char*)realloc(img_path, strnlen(img_path, PATH_MAX - 1) + 1);
    return tmp != nullptr ? tmp : img_path;
}

char* rcloud_drive_mnt_path(void) {
    // Allocate memory on the heap to store the mount path
    const auto mnt_path = (char*)malloc(PATH_MAX);
    if (mnt_path == nullptr) {
        return nullptr;
    }

    // Get the user's home directory
    const ssize_t buf_size = sysconf(_SC_GETPW_R_SIZE_MAX);
    const size_t pw_buf_size = buf_size == -1 ? 16384 : (size_t)buf_size;
    const auto pw_buf = (char*)malloc(pw_buf_size);
    if (pw_buf == nullptr) {
        return nullptr;
    }
    struct passwd pw;
    struct passwd* result;
    const int ret_val = getpwuid_r(getuid(), &pw, pw_buf, pw_buf_size, &result);
    if (ret_val != EXIT_SUCCESS ||
        result == nullptr ||
        snprintf(mnt_path, PATH_MAX, "%s/rcloud", pw.pw_dir) >= PATH_MAX) {
        free(pw_buf);
        free(mnt_path);
        return nullptr;
    }
    free(pw_buf);

    // Free access heap memory
    const auto tmp = (char*)realloc(mnt_path, strnlen(mnt_path, PATH_MAX - 1) + 1);
    return tmp != nullptr ? tmp : mnt_path;
}

int rcloud_drive_create(const char* img_path, const char* mnt_path) {
    struct stat st;
    if (stat(img_path, &st) == -1) {
        if (errno != ENOENT || create_image(img_path) == -1) {
            return -1;
        }
    }

    bool mnt_dir_created = false;
    if (stat(mnt_path, &st) == -1) {
        if (errno != ENOENT || mkdir(mnt_path, 0700) < 0) {
            return -1;
        }
        mnt_dir_created = true;
    }

    if (path_is_mountpoint(mnt_path)) {
        return EXIT_SUCCESS;
    }

    int ret_val = -1;

    struct libmnt_context* cxt = mnt_new_context();
    if (cxt == nullptr) {
        errno = ENOMEM;
        goto ret;
    }

    if (add_to_fstab(img_path, mnt_path) != EXIT_SUCCESS) {
        goto ret;
    }

    if (mnt_context_set_source(cxt, img_path) != EXIT_SUCCESS ||
        mnt_context_set_target(cxt, mnt_path) != EXIT_SUCCESS ||
        mnt_context_set_fstype(cxt, RCLOUD_FSTYPE) != EXIT_SUCCESS ||
        mnt_context_set_user_mflags(cxt, MNT_MS_LOOP) != EXIT_SUCCESS) {
        errno = EINVAL;
        goto ret;
    }

    mnt_context_enable_loopdel(cxt, true);

    {
        int rc = mnt_context_mount(cxt);
        if (rc != EXIT_SUCCESS) {
            int sys_errno = mnt_context_get_syscall_errno(cxt);
            char buf[256];
            mnt_context_get_excode(cxt, rc, buf, sizeof(buf));
            fprintf(stderr, "libmount error (rc=%d): %s\n", rc, buf);
            if (sys_errno != 0) {
                fprintf(stderr, "System error: %s\n", strerror(sys_errno));
            }
            errno = (sys_errno != 0) ? sys_errno : EIO;
            goto ret;
        }
    }

    ret_val = EXIT_SUCCESS;

ret:
    mnt_free_context(cxt);
    if (ret_val != EXIT_SUCCESS && mnt_dir_created) {
        rmdir(mnt_path);
    }
    return ret_val;
}

int rcloud_drive_destroy(const char* img_path, const char* mnt_path) {
    if (!path_is_mountpoint(mnt_path)) {
        return unlink(img_path);
    }

    struct libmnt_context* cxt = mnt_new_context();
    if (cxt == nullptr) {
        errno = ENOMEM;
        return -1;
    }

    int ret_val = -1;

    mnt_context_set_target(cxt, mnt_path);
    mnt_context_enable_loopdel(cxt, true);

    if (mnt_context_umount(cxt) == EXIT_SUCCESS &&
        rm_from_fstab(img_path) == EXIT_SUCCESS &&
        unlink(img_path) == EXIT_SUCCESS) {
        ret_val = EXIT_SUCCESS;
    } else {
        errno = mnt_context_get_syscall_errno(cxt);
        if (errno == 0) {
            errno = EIO;
        }
    }

    mnt_free_context(cxt);
    return ret_val;
}
