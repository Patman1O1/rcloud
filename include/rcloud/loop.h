#ifndef LOOP_H
#define LOOP_H

// ISO Includes
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// POSIX Includes
#include <sys/ioctl.h>

// GNU Includes
#include <linux/loop.h>

// Local Includes
#include <rcloud/backing_file.h>

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

// The maximum size the path to the loop device can be. Let num_digits_in_uint32_max
// be the number of digits in the stdint.h macro UINT32_MAX and let padding equal 4.
// Then LOOP_DEV_PATH_MAX is calculated via the formula below.
// LOOP_DEV_PATH_MAX = sizeof("/dev/loop") + num_digits_in_uint32_max + padding
static constexpr size_t LOOP_DEV_PATH_MAX = 24;

// The maximum size the name of the loop device can be. Let num_digits_in_uint32_max
// be the number of digits in the stdint.h macro UINT32_MAX and let padding equal 1.
// Then LOOP_DEV_NAME_MAX is calculated via the formula below.
// LOOP_DEV_NAME_MAX = sizeof("loop") + num_digits_in_uint32_max + padding
static constexpr size_t LOOP_DEV_NAME_MAX = 16;

static constexpr size_t LOOP_MUTEX_PATH_MAX = 32;

// The absolute path to the Loop Control file.
static constexpr char LOOP_CTL_PATH[] = "/dev/loop-control";

struct loop_device {
    struct loop_config lo_cfg;
    char lo_path[LOOP_DEV_PATH_MAX];
    int lo_fd;
    uint8_t _padding[4];
};

struct loop_device_mutex {
    char m_path[LOOP_MUTEX_PATH_MAX];
    int m_fd;
};

static inline int loop_ctl_get_free(const int ctl_fd) { return ioctl(ctl_fd, LOOP_CTL_GET_FREE); }

static inline int loop_ctl_add(const int ctl_fd, const unsigned long int lo_number) {
    return ioctl(ctl_fd, LOOP_CTL_ADD, lo_number);
}

static inline int loop_ctl_remove(const int ctl_fd, const unsigned long int lo_number) {
    return ioctl(ctl_fd, LOOP_CTL_REMOVE, lo_number);
}

static inline int loop_config(const int loop_fd, const struct loop_config* lo_cfg_p) {
    return ioctl(loop_fd, LOOP_CONFIGURE, lo_cfg_p);
}

static inline int loop_change_fd(const int loop_fd, const int bk_fd) {
    return ioctl(loop_fd, LOOP_CHANGE_FD, bk_fd);
}

static inline int loop_set_status64(const int loop_fd, const struct loop_info64* lo_info64_p) {
    return ioctl(loop_fd, LOOP_SET_STATUS64, lo_info64_p);
}

static inline int loop_get_status64(const int loop_fd, struct loop_info64* lo_info64_p) {
    return ioctl(loop_fd, LOOP_GET_STATUS64, lo_info64_p);
}

static inline int loop_set_block_size(const int loop_fd, const size_t block_size) {
    return ioctl(loop_fd, LOOP_SET_BLOCK_SIZE, block_size);
}

static inline int loop_set_direct_io(const int loop_fd, const bool direct_io_flag) {
    return ioctl(loop_fd, LOOP_SET_DIRECT_IO, direct_io_flag);
}

static inline int loop_set_capacity(const int loop_fd) { return ioctl(loop_fd, LOOP_SET_CAPACITY, 0); }

extern int loop_device_init(struct loop_device* lo_dev_p, const struct backing_file* bk_file_p);

extern int loop_device_destroy(struct loop_device* lo_dev_p);

extern int loop_device_create(struct loop_device* lo_dev_p);

extern int loop_device_remove(struct loop_device* lo_dev_p);

extern int loop_device_mutex_init(struct loop_device_mutex* mutex_p, const struct loop_device* dev_p);

extern int loop_device_mutex_destroy(struct loop_device_mutex* mutex_p);

extern int loop_device_mutex_create(struct loop_device_mutex* mutex_p, const struct loop_device* dev_p);

extern int loop_device_mutex_remove(struct loop_device_mutex* mutex_p);

extern int loop_device_mutex_lock(struct loop_device_mutex* mutex_p);

extern int loop_device_mutex_unlock(struct loop_device_mutex* mutex_p);

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif // #ifndef LOOP_H

