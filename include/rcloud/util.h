#ifndef UTIL_H
#define UTIL_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

#ifdef _XOPEN_SOURCE
#define _XOPEN_SOURCE_OLD _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif // #ifdef _XOPEN_SOURCE

#define _XOPEN_SOURCE 500

// ISO Includes
#include <stdio.h>

// POSIX Includes
#include <sys/types.h>
#include <sys/stat.h>
#include <ftw.h>

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

static inline int _remove_cb(const char* path_p, const struct stat*, int, struct FTW*) { return remove(path_p); }

extern int mkdirs(const char* path_p, mode_t mode);

static inline int rmdirs(const char* path_p) {
    if (path_p == nullptr || path_p[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    return nftw(path_p, _remove_cb, 64, FTW_DEPTH | FTW_PHYS);
}

extern const char* gethome(void);

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#ifdef _XOPEN_SOURCE_OLD
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE _XOPEN_SOURCE_OLD
#endif // #ifdef _XOPEN_SOURCE_OLD

#endif // #ifndef UTIL_H

