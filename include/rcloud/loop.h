#ifndef LOOP_H
#define LOOP_H

// ISO Includes
#include <stddef.h>
#include <stdbool.h>

// POSIX Includes
#include <sys/ioctl.h>

// GNU Includes
#include <linux/fs.h>
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

// The absolute path to the Loop Control device.
static constexpr char LOOP_CTL_PATH[] = "/dev/loop-control";

static inline int loop_ctl_get_free(const int ctl_fd) { return ioctl(ctl_fd, LOOP_CTL_GET_FREE); }

static inline int loop_ctl_add(const int ctl_fd, const unsigned long int lo_number) {
    return ioctl(ctl_fd, LOOP_CTL_ADD, lo_number);
}

static inline int loop_ctl_rm(const int ctl_fd, const unsigned long int lo_number) {
    return ioctl(ctl_fd, LOOP_CTL_REMOVE, lo_number);
}

static inline int loop_config(const int loop_fd, const struct loop_config* lo_cfg) {
    return ioctl(loop_fd, LOOP_CONFIGURE, lo_cfg);
}

static inline int loop_change_fd(const int loop_fd, const int bk_fd) {
    return ioctl(loop_fd, LOOP_CHANGE_FD, bk_fd);
}

static inline int loop_clr_fd(const int loop_fd) {
    const int status = ioctl(loop_fd, LOOP_CLR_FD, 0);
    close(loop_fd);
    return status;
}

static inline int loop_set_status64(const int loop_fd, const struct loop_info64* lo_info64) {
    return ioctl(loop_fd, LOOP_SET_STATUS64, lo_info64);
}

static inline int loop_get_status64(const int loop_fd, struct loop_info64* lo_info64) {
    return ioctl(loop_fd, LOOP_GET_STATUS64, lo_info64);
}

static inline int loop_set_block_size(const int loop_fd, const size_t block_size) {
    return ioctl(loop_fd, LOOP_SET_BLOCK_SIZE, block_size);
}

static inline int loop_set_direct_io(const int loop_fd, const bool direct_io_flag) {
    return ioctl(loop_fd, LOOP_SET_DIRECT_IO, direct_io_flag);
}

static inline int loop_set_capacity(const int loop_fd) { return ioctl(loop_fd, LOOP_SET_CAPACITY, 0); }

static inline ssize_t loop_block_size(const int loop_fd) {
    int block_size;
    return ioctl(loop_fd, BLKSSZGET, &block_size) >= 0 ? block_size : -1;
}

extern int loop_device_init(int backing_fd, char* loop_dev_path);

extern int loop_device_open(const char* backing_path_p);

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif // #ifndef LOOP_H

