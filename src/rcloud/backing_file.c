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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <ftw.h>

// GNU Includes

// Local Includes
#include <rcloud/util.h>
#include <rcloud/backing_file.h>

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

static inline bool backing_file_exists(const char* path_p, struct stat* st_p) {
    return stat(path_p, st_p) == EXIT_SUCCESS;
}

static inline bool backing_file_is_regular(const struct stat* st_p) {
    return S_ISREG(st_p->st_mode) == EXIT_SUCCESS;
}

static inline bool backing_file_is_valid_size(const struct stat* st_p, const off_t size) {
    return st_p->st_size > size;
}

enum backing_file_state backing_file_get_state(const char* path_p) {
    if (path_p == nullptr) {
        errno = EFAULT;
        return DOES_NOT_EXIST;
    }

    struct stat st;
    if (!backing_file_exists(path_p, &st)) {
        return DOES_NOT_EXIST;
    }

    if (!backing_file_is_regular(&st)) {
        return NOT_REGULAR;
    }

    struct statfs sf;
    if (statfs(path_p, &sf) == -1) {
        return UNKNOWN;
    }

    if (!backing_file_is_valid_size(&st, (off_t)sf.f_bavail * sf.f_bsize)) {
        return errno == 0 ? INVALID_SIZE : UNKNOWN;
    }

    return VALID;
}

int backing_file_create(struct backing_file* file_p, const char* path_p, const off_t size) {
    // Ensure all pointers are non-null
    if (file_p == nullptr || path_p == nullptr) {
        errno = EFAULT;
        return -1;
    }

    // Ensure the length of the path does not exceed the buffer size
    const size_t path_len = strlen(path_p);
    if (sizeof(file_p->bk_path) <= path_len + 1) {
        errno = ENAMETOOLONG;
        return -1;
    }

    // Initialize all bytes in the struct to 0
    memset(file_p, 0x00, sizeof(struct backing_file));

    // Check if the backing file already exists
    struct stat st;
    if (backing_file_exists(path_p, &st)) {
        errno = EEXIST;
        return -1;
    }

    // Exit the function if errno is not "No such file or directory"
    if (errno != ENOENT) {
        return -1;
    }

    // Isolate parent directory path safely
    const char* last_slash_p = strrchr(path_p, '/');
    if (last_slash_p != nullptr) {
        // Get the length of the path to backing file's directory
        const size_t dir_len = last_slash_p - path_p;

        // Extract the directory path
        memcpy(file_p->bk_path, path_p, dir_len);
        file_p->bk_path[dir_len] = '\0';

        // Ensure the directory path exists
        if (mkdirs(file_p->bk_path, 0755) == -1) {
            return -1;
        }
    }

    // Create the backing file and get a file descriptor to it
    const int fd = open(path_p, O_CREAT | O_WRONLY | O_EXCL | O_CLOEXEC, 0644);
    if (fd == -1) {
        return -1;
    }

    // Allocate the requested size for the backing file
    if (ftruncate(fd, size) == -1) {
        close(fd);
        return -1;
    }

    // Close the backing file
    close(fd);

    // Copy the entire path into the backing file struct's f_path field
    memcpy(file_p->bk_path, path_p, path_len + 1);

    // Set the f_size field
    file_p->bk_size = size;

    return EXIT_SUCCESS;
}


#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

