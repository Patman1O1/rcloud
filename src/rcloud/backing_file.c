#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO Includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// POSIX Includes
#include <unistd.h>
#include <fcntl.h>

// Local Includes
#include <rcloud/fs.h>
#include <rcloud/backing_file.h>

int backing_file_create(const char* path) {
    // Isolate parent directory path and create if necessary
    const char* last_slash = strrchr(path, '/');
    if (last_slash != nullptr) {
        const size_t dir_len = last_slash - path;
        char dir_path[dir_len + 1];
        memcpy(dir_path, path, dir_len);
        dir_path[dir_len] = '\0';

        if (mkdirs(dir_path, 0755) == -1) {
            return -1;
        }
    }

    // Create the file only if it does not exist.
    return open(path, O_CREAT | O_RDWR | O_EXCL | O_CLOEXEC, 0644);
}

int backing_file_init(const char* path, const off64_t size) {
    // Get a file descriptor to the backing file
    int backing_fd;
    if (access(path, F_OK) != EXIT_SUCCESS) {
        // Create the backing file
        backing_fd = backing_file_create(path);
        if (backing_fd == -1) {
            return -1;
        }
    } else {
        // Open the backing file
        backing_fd = open(path, O_RDWR | O_CLOEXEC);
        if (backing_fd == -1) {
            return -1;
        }
    }

    // Get the backing file's current size
    const off64_t curr_size = backing_file_size(path);
    if (curr_size < 0) {
        return -1;
    }

    // Return the backing file descriptor if the backing file size is at least BACKING_FILE_INIT_SIZE
    if (curr_size >= size) {
        return backing_fd;
    }

    // Resize the backing file so it is at least BACKING_FILE_INIT_SIZE and return its file descriptor
    return ftruncate64(backing_fd, size) == EXIT_SUCCESS ? backing_fd : -1 ;
}
