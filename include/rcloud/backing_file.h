#ifndef BACKING_FILE_H
#define BACKING_FILE_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO Includes
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

// POSIX Includes
#include <unistd.h>
#include <sys/types.h>
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

static inline int backing_file_open(struct backing_file* file_p, const int flags, const mode_t mode) {
    if (file_p == nullptr) {
        errno = ENOENT;
        return -1;
    }

    file_p->bk_fd = open(file_p->bk_path, flags, mode);
    return file_p->bk_fd;
}

static inline int backing_file_close(struct backing_file* file_p) {
    if (file_p == nullptr) {
        errno = ENOENT;
        return -1;
    }

    const int ret = close(file_p->bk_fd);
    file_p->bk_fd = -1;
    return ret;
}

static inline bool backing_file_exists(const char* path_p) {
    struct stat st;
    return path_p != nullptr && stat(path_p, &st) == EXIT_SUCCESS;
}

static inline bool backing_file_is_reg(const char* path_p) {
    struct stat st;
    if (path_p == nullptr || stat(path_p, &st) == -1) {
        errno = ENOENT;
        return false;
    }

    return S_ISREG(st.st_mode);
}

extern int backing_file_create(struct backing_file* file_p, const char* path_p, off_t size);

extern int backing_file_remove(struct backing_file* file_p);

extern int backing_file_init(struct backing_file* file_p, const char* path_p);

extern void backing_file_destroy(struct backing_file* file_p);

#ifdef __cplusplus
}
#endif // #ifndef __cplusplus

#endif // #ifndef BACKING_FILE_H

