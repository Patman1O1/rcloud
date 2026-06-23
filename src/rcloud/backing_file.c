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

int backing_file_create(struct backing_file* file, const char* path, const off_t size) {
    // Ensure all pointers are non-null
    if (file == nullptr || path == nullptr) {
        errno = EFAULT;
        return -1;
    }

    if (size <= 0) {
        errno = EINVAL;
        return -1;
    }

    // Ensure the length of the path does not exceed the buffer size
    const size_t path_len = strlen(path);
    if (sizeof(file->bk_path) <= path_len + 1) {
        errno = ENAMETOOLONG;
        return -1;
    }

    // Initialize all bytes in the struct to 0
    memset(file, 0x00, sizeof(struct backing_file));

    // Check if the backing file already exists
    if (backing_file_exists(path)) {
        errno = EEXIST;
        return -1;
    }

    // Exit the function if errno is not "No such file or directory"
    if (errno != ENOENT) {
        return -1;
    }

    // Isolate parent directory path safely
    const char* last_slash = strrchr(path, '/');
    if (last_slash != nullptr) {
        // Get the length of the path to backing file's directory
        const size_t dir_len = last_slash - path;

        // Extract the directory path
        char dir_path[dir_len + 1];
        memcpy(dir_path, path, dir_len);
        dir_path[dir_len] = '\0';

        // Ensure the directory path exists
        if (mkdirs(dir_path, 0755) == -1) {
            return -1;
        }
    }

    // Create the backing file and get a file descriptor to it
    const int fd = open(path, O_CREAT | O_WRONLY | O_EXCL | O_CLOEXEC, 0644);
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

    // Copy the entire path into the backing file struct's bk_path field
    memcpy(file->bk_path, path, path_len + 1);

    // Set the size field
    file->bk_size = size;

    return EXIT_SUCCESS;
}

int backing_file_remove(struct backing_file* file) {
    if (file == nullptr || !backing_file_exists(file->bk_path)) {
        errno = ENOENT;
        return -1;
    }

    // Close the file if it is open
    if (file->bk_fd >= 0) {
        close(file->bk_fd);
        file->bk_fd = -1;
    }

    // Remove the file
    if (nftw(file->bk_path, nftw_func, 64, FTW_DEPTH | FTW_PHYS) == -1) {
        return -1;
    }

    // Zero out all the bytes in the path string
    memset(file->bk_path, 0x00, sizeof(file->bk_path));

    // Set the file size to -1 (since it no longer exists)
    file->bk_size = -1;

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