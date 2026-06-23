#ifndef BACKING_FILE_H
#define BACKING_FILE_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO Includes
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

// POSIX Includes
#include <fcntl.h>
#include <sys/stat.h>

// GNU Includes
#include <linux/loop.h>

#ifdef __cplusplus
extern "C" {
#endif // #ifndef __cplusplus

struct backing_file {
    char bk_path[LO_NAME_SIZE];
    off_t bk_size;
    int bk_fd;
    uint8_t _padding[4];
};

static inline bool backing_file_exists(const char* path) {
    struct stat st;
    return path != nullptr && stat(path, &st) == EXIT_SUCCESS;
}

static inline bool backing_file_is_reg(const char* path) {
    struct stat st;
    if (path == nullptr || stat(path, &st) == -1) {
        errno = ENOENT;
        return false;
    }

    return S_ISREG(st.st_mode);
}

extern int backing_file_create(struct backing_file* file, const char* path, off_t size);

extern int backing_file_remove(struct backing_file* file);

#ifdef __cplusplus
}
#endif // #ifndef __cplusplus

#endif // #ifndef BACKING_FILE_H