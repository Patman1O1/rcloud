#ifndef BACKING_FILE_H
#define BACKING_FILE_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO Includes
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

// POSIX Includes
#include <fcntl.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif // #ifndef __cplusplus

static inline bool backing_file_exists(const char* path) {
    struct stat64 st64;
    return stat64(path, &st64) == EXIT_SUCCESS;
}

static inline bool backing_file_is_reg(const char* path) {
    struct stat64 st64;
    if (stat64(path, &st64) == -1) {
        errno = ENOENT;
        return false;
    }
    return S_ISREG(st64.st_mode);
}

extern int backing_file_create(const char* path, off_t size);

extern int backing_file_remove(const char* path);

#ifdef __cplusplus
}
#endif // #ifndef __cplusplus

#endif // #ifndef BACKING_FILE_H