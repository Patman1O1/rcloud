#ifndef BACKING_FILE_H
#define BACKING_FILE_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO Includes
#include <stdint.h>
#include <stdbool.h>

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

enum backing_file_state : uint8_t {
    DOES_NOT_EXIST,
    NOT_REGULAR,
    INVALID_SIZE,
    VALID,
    UNKNOWN
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

extern int backing_file_create(struct backing_file* file_p, const char* path_p, off_t size);

extern enum backing_file_state backing_file_get_state(const char* path_p);

extern int backing_file_destroy(struct backing_file* file_p);

#ifdef __cplusplus
}
#endif // #ifndef __cplusplus

#endif // #ifndef BACKING_FILE_H

