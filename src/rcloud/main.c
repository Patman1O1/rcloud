#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO Includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// POSIX Includes
#include <unistd.h>

// GNU Includes

// Local Includes
#include "rcloud/drive.h"

static void cleanup_path(char** path_ref);

#define _path_t char* __attribute__((cleanup(cleanup_path)))

static int handle_drive_cmd(const char* img_path, const char* mnt_path, const char* action);

int main(const int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <create|destroy>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Construct the absolute path to the image file
    char* img_path __attribute__((cleanup(cleanup_path))) = rcloud_drive_img_path();
    if (img_path == nullptr) {
        perror("rcloud_drive_img_path");
        return EXIT_FAILURE;
    }
    // Construct the absolute path to the mount point
    char* mnt_path __attribute__((cleanup(cleanup_path))) = rcloud_drive_mnt_path();
    if (mnt_path == nullptr) {
        perror("rcloud_drive_mnt_path");
        return EXIT_FAILURE;
    }

    if (handle_drive_cmd(img_path, mnt_path, argv[1]) != 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static void cleanup_path(char** path_ref) {
    if (*path_ref == nullptr) {
	return;
    }

    free(*path_ref);
    *path_ref = nullptr;
}

static int handle_drive_cmd(const char* img_path, const char* mnt_path, const char* action) {
    if (strcmp(action, "create") == 0) {
        return rcloud_drive_create(img_path, mnt_path);
    }

    if (strcmp(action, "destroy") == 0) {
	return rcloud_drive_destroy(img_path, mnt_path);	
    }

    fprintf(stderr, "Invalid action: %s\n", action);
    return -1;
}

