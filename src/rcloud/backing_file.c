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

static inline int remove_cb(const char* path_p, const struct stat*, int, struct FTW*) { return remove(path_p); }

int backing_file_create(struct backing_file* file_p, const char* path_p, const off_t size) {
    // Ensure all pointers are non-null
    if (file_p == nullptr || path_p == nullptr) {
        errno = EFAULT;
        return -1;
    }

    if (size <= 0) {
        errno = EINVAL;
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
    if (backing_file_exists(path_p)) {
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

    // Set the size field
    file_p->bk_size = size;

    return EXIT_SUCCESS;
}

int backing_file_remove(struct backing_file* file_p) {
    if (file_p == nullptr || !backing_file_exists(file_p->bk_path)) {
        errno = ENOENT;
        return -1;
    }

    // Close the file if it is open
    if (file_p->bk_fd >= 0) {
        close(file_p->bk_fd);
        file_p->bk_fd = -1;
    }

    // Remove the file
    if (nftw(file_p->bk_path, remove_cb, 64, FTW_DEPTH | FTW_PHYS) == -1) {
        return -1;
    }

    // Zero out all the bytes in the path string
    memset(file_p->bk_path, 0x00, sizeof(file_p->bk_path));

    // Set the file size to -1 (since it no longer exists)
    file_p->bk_size = -1;

    return EXIT_SUCCESS;
}

int backing_file_init(struct backing_file* file_p, const char* path_p) {
    // Ensure all pointers are non-null
    if (file_p == nullptr || path_p == nullptr) {
        errno = EFAULT;
        return -1;
    }

    // Ensure the backing file actually exists
    struct stat st;
    if (stat(path_p, &st) == -1) {
        errno = ENOENT;
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        errno = EINVAL;
        return -1;
    }

    // Set the file path
    snprintf(file_p->bk_path, sizeof(file_p->bk_path), "%s", path_p);

    // Set the file descriptor
    file_p->bk_fd = open(file_p->bk_path, O_RDWR | O_DIRECT | O_CLOEXEC, 0644);
    if (file_p->bk_fd == -1) {
        return -1;
    }

    // Set the file size
    file_p->bk_size = st.st_size;

    return EXIT_SUCCESS;
}

void backing_file_destroy(struct backing_file* file_p) {
    if (file_p == nullptr) {
        return;
    }

    // Close the file if it is still open
    if (file_p->bk_fd >= 0) {
        close(file_p->bk_fd);
        file_p->bk_fd = -1;
    }
}

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#ifdef _XOPEN_SOURCE_ORIGINAL
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE _XOPEN_SOURCE_ORIGINAL
#undef _XOPEN_SOURCE_ORIGINAL
#endif // #ifdef _XOPEN_SOURCE_ORIGINAL

