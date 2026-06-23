#ifndef RCLOUD_FS_H
#define RCLOUD_FS_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

#ifdef _XOPEN_SOURCE
#define _XOPEN_SOURCE_OLD _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif // #ifdef _XOPEN_SOURCE

#define _XOPEN_SOURCE 500

#ifdef __cplusplus
extern "C" {
#endif

// ISO Includes
#include <stdio.h>
#include <errno.h>

// POSIX Includes
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ftw.h>

static inline int _nftw_cb(const char* path_p, const struct stat*, int, struct FTW*) { return remove(path_p); }

extern int mkdirs(const char* path, mode_t mode);

static inline int rmdirs(const char* path_p) {
    if (path_p == nullptr || path_p[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    return nftw(path_p, _nftw_cb, 64, FTW_DEPTH | FTW_PHYS);
}

extern bool is_mounted(const char* dev_path);

extern int mkfs_ext4(const char* dev_path);

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#ifdef _XOPEN_SOURCE_OLD
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE _XOPEN_SOURCE_OLD
#endif // #ifdef _XOPEN_SOURCE_OLD

#endif // #ifndef RCLOUD_FS_H
