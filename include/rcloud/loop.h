#ifndef LOOP_H
#define LOOP_H

// ISO Includes
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

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

extern int loop_ctl_open(void);

extern int loop_ctl_get_free(int ctl_fd);

extern int loop_ctl_add(int ctl_fd, unsigned long int lo_number);

extern int loop_ctl_remove(int ctl_fd, unsigned long int lo_number);

extern int loop_config(int loop_fd, const struct loop_config* lo_cfg_p);

extern int loop_change_fd(int loop_fd, int bk_fd);

extern int loop_set_status64(int loop_fd, const struct loop_info64* lo_info64_p);

extern int loop_get_status64(int loop_fd, struct loop_info64* lo_info64_p);

extern int loop_set_block_size(int loop_fd, size_t block_size);

extern int loop_set_direct_io(int loop_fd, bool direct_io_flag);

extern int loop_set_capacity(int loop_fd);

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

