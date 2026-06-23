#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

#ifdef _XOPEN_SOURCE
#define _XOPEN_SOURCE_ORIGINAL _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif // #ifdef _XOPEN_SOURCE

#define _XOPEN_SOURCE 500

// ISO Includes
#include <stdio.h>
#include <string.h>

// POSIX Includes
#include <sys/stat.h>
#include <ftw.h>

// GNU Includes

// Local Includes
#include <rcloud/util.h>
#include <rcloud/backing_file.h>

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

static inline int nftw_func(const char* path, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
    return remove(path);
}

int backing_file_create(const char* path, const off_t size) {
    if (path == nullptr || size <= 0) {
        errno = path == nullptr ? EFAULT : EINVAL;
        return -1;
    }

    // Isolate parent directory path and create if necessary
    const char* last_slash = strrchr(path, '/');
    if (last_slash != nullptr) {
        const size_t dir_len = last_slash - path;
        char dir_path[dir_len + 1];
        memcpy(dir_path, path, dir_len);
        dir_path[dir_len] = '\0';

        if (mkdirs(dir_path, 0755) == -1) {
            return -1;
        }
    }

    // Create the file only if it does not exist.
    const int fd = open(path, O_CREAT | O_RDWR | O_EXCL | O_CLOEXEC, 0644);
    if (fd == -1) {
        return -1; // errno is correctly set to EEXIST if file exists
    }

    // Allocate the requested size
    if (ftruncate(fd, size) == -1) {
        const int saved_errno = errno;
        close(fd);
        unlink(path);
        errno = saved_errno;
        return -1;
    }

    // Return the open file descriptor to the caller
    return fd;
}

int backing_file_remove(const char* path) {
    if (path == nullptr || !backing_file_exists(path)) {
        errno = ENOENT;
        return -1;
    }

    // Remove the file
    if (nftw(path, nftw_func, 64, FTW_DEPTH | FTW_PHYS) == -1) {
        return -1;
    }

    return EXIT_SUCCESS;
}

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#ifdef _XOPEN_SOURCE_ORIGINAL
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE _XOPEN_SOURCE_ORIGINAL
#undef _XOPEN_SOURCE_ORIGINAL
#endif // #ifdef _XOPEN_SOURCE_ORIGINAL