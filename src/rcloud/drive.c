#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // #ifndef _GNU_SOURCE

// ISO Includes
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// POSIX Includes
#include <unistd.h>
#include <pwd.h>

// Local Includes
#include "rcloud/fs.h"
#include "rcloud/backing_file.h"
#include "rcloud/loop.h"
#include "rcloud/drive.h"

// The name of the backing file.
static constexpr char BACKING_FILE_NAME[] = "rcloud.img";

static char* gethome(char* result_buf, const size_t result_size) {
    if (result_buf == nullptr) {
        errno = EFAULT;
        return nullptr;
    }

    struct passwd pw;
    struct passwd* result;

    // Allocate temporary space for the database lookup
    char buffer[1024];

    if (getpwuid_r(getuid(), &pw, buffer, sizeof(buffer), &result) != 0 || result == nullptr) {
        return nullptr;
    }

    // Ensure the result buffer is large enough to hold the user's home directory path
    const size_t pw_dir_len = strlen(pw.pw_dir);
    if (pw_dir_len + 1 > result_size) {
        errno = ENAMETOOLONG;
        return nullptr;
    }

    // Copy the home directory into the result buffer
    memcpy(result_buf, pw.pw_dir, pw_dir_len);
    result_buf[pw_dir_len] = '\0';
    return result_buf;
}

