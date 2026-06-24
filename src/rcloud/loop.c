#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO Includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// POSIX Includes
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

// GNU Includes
#include <sys/file.h>

// Local Includes
#include "rcloud/loop.h"

int loop_device_init(const int backing_fd, char* loop_dev_path) {
    // Open the loop control device
    const int ctl_fd = open(LOOP_CTL_PATH, O_RDWR | O_CLOEXEC);
    if (ctl_fd == -1) {
        // Close the backing file
        close(backing_fd);
        return -1;
    }

    // Gain ownership of the loop control device's lock
    if (flock(ctl_fd, LOCK_EX) == -1) {
        // Close the loop control device
        close(ctl_fd);

        // Close the backing file
        close(backing_fd);
        return -1;
    }

    // Get the index to a free loop device
    const int loop_num = loop_ctl_get_free(ctl_fd);
    if (loop_num == -1) {
        // Give up ownership of the loop control device's lock
        flock(ctl_fd, LOCK_UN);

        // Close the loop control device
        close(ctl_fd);

        // Close the backing file
        close(backing_fd);
        return -1;
    }

    // Construct the path to the loop device
    snprintf(loop_dev_path, LOOP_DEV_PATH_MAX, "/dev/loop%u", (unsigned int)loop_num);

    // Open the loop device
    const int loop_fd = open(loop_dev_path, O_RDWR | O_CLOEXEC);
    if (loop_fd == -1) {
        // Give up ownership of the loop control device lock
        flock(ctl_fd, LOCK_UN);

        // Close the loop control device
        close(ctl_fd);

        // Close the backing file
        close(backing_fd);
        return -1;
    }

    // Give up ownership of the loop control device lock
    if (flock(ctl_fd, LOCK_UN) == -1) {
        // Close the loop control device
        close(ctl_fd);

        // Close the backing file
        close(backing_fd);

        // Close the loop device
        close(loop_fd);
        return -1;
    }

    // Configure the loop device
    struct loop_config loop_cfg = {0};
    loop_cfg.fd = backing_fd;
    loop_cfg.block_size = 4096;
    if (loop_config(loop_fd, &loop_cfg) == -1) {
        close(loop_fd);
        close(ctl_fd);
        close(backing_fd);
        return -1;
    }

    // Close the loop control device
    close(ctl_fd);

    // Close the backing file
    close(backing_fd);

    // Return the file descriptor of the loop device
    return loop_fd;
}

int loop_device_open(const char* backing_path_p) {
    if (backing_path_p == nullptr) {
        errno = EFAULT;
        return -1;
    }

    struct stat64 backing_st64;
    if (stat64(backing_path_p, &backing_st64) != EXIT_SUCCESS) {
        return -1;
    }

    DIR* dstream_p = opendir("/sys/block/");
    if (dstream_p == nullptr) {
        return -1;
    }

    struct dirent* dir_p;
    while ((dir_p = readdir(dstream_p)) != nullptr) {
        // Look only for loop devices
        if (strncmp(dir_p->d_name, "loop", 4) == 0) {
            char path[256];
            snprintf(path, sizeof(path), "/sys/block/%s/loop/backing_file", dir_p->d_name);

            // Read the backing file path from the sysfs entry
            FILE* file_p = fopen(path, "r");
            if (file_p == nullptr) {
                continue;
            }

            char backing_path[1024];
            if (fgets(backing_path, sizeof(backing_path), file_p)) {
                // Remove newline
                backing_path[strcspn(backing_path, "\n")] = 0;

                struct stat64 loop_st64;
                if (stat64(backing_path, &loop_st64) == 0) {
                    // Match by Device ID and Inode
                    if (loop_st64.st_dev == backing_st64.st_dev && loop_st64.st_ino == backing_st64.st_ino) {
                        fclose(file_p);
                        closedir(dstream_p);

                        char dev_path[32];
                        snprintf(dev_path, sizeof(dev_path), "/dev/%s", dir_p->d_name);
                        return open(dev_path, O_RDWR | O_CLOEXEC);
                    }
                }
            }
            fclose(file_p);
        }
    }
    closedir(dstream_p);
    return -1;
}
