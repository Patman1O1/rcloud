#ifndef BACKING_FILE_H
#define BACKING_FILE_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

#ifdef __cplusplus
extern "C" {
#endif // #ifndef __cplusplus

#ifdef _XOPEN_SOURCE
#define _XOPEN_SOURCE_ORIGINAL _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif // #ifdef _XOPEN_SOURCE

#define _XOPEN_SOURCE 500

// ISO Includes
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

// POSIX Includes
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ftw.h>

static inline int backing_file_nftw_func(const char* path, const struct stat*, int, struct FTW*) {
    return remove(path);
}

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

static inline off64_t backing_file_size(const char* path) {
    struct stat64 st64;
    return stat64(path, &st64) == EXIT_SUCCESS ? st64.st_size : -1;
}

static inline int backing_file_remove(const char* path) {
    return nftw(path, backing_file_nftw_func, 64, FTW_DEPTH | FTW_PHYS) != -1 ? EXIT_SUCCESS : -1;
}

extern int backing_file_create(const char* path);

extern int backing_file_init(const char* path, off64_t size);

#ifdef _XOPEN_SOURCE_ORIGINAL
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE _XOPEN_SOURCE_ORIGINAL
#undef _XOPEN_SOURCE_ORIGINAL
#endif // #ifdef _XOPEN_SOURCE_ORIGINAL

#ifdef __cplusplus
}
#endif // #ifndef __cplusplus

#endif // #ifndef BACKING_FILE_H