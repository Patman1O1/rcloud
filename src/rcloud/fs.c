#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// ISO Includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// POSIX Includes
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

// Local Includes
#include "rcloud/fs.h"

int mkdirs(const char* path, const mode_t mode) {
    static struct stat stat_v;

    // Validate the pointer to the path and the path itself
    if (path == nullptr || *path == '\0') {
        errno = ENOENT;
        return -1;
    }

    // Create a copy of the path
    char* path_cpy_p = strdup(path);
    if (path_cpy_p == nullptr) {
        return -1;
    }

    // Drop trailing slashes to avoid creating empty components
    size_t path_len = strlen(path);
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

bool is_mounted(const char* dev_path) {
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

int mkfs_ext4(const char* dev_path) {
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
