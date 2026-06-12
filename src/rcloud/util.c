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
#include <rcloud/util.h>

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

int mkdirs(const char* path_p, const mode_t mode) {
    static struct stat st;

    // Validate the pointer to the path and the path itself
    if (path_p == nullptr || *path_p == '\0') {
        errno = ENOENT;
        return -1;
    }

    // Create a copy of the path
    char* path_cpy_p = strdup(path_p);
    if (path_cpy_p == nullptr) {
        return -1;
    }

    // Drop trailing slashes to avoid creating empty components
    size_t path_len = strlen(path_p);
    while (path_len > 1 && path_cpy_p[path_len - 1] == '/') {
        path_cpy_p[path_len--] = '\0';
    }

    for (char* char_p = path_cpy_p + 1; ; char_p++) {
        if (*char_p != '/' && *char_p != '\0') {
            continue;
        }

        const char sep = *char_p;
        *char_p = '\0';

        if (mkdir(path_cpy_p, mode) == -1) {
            if (errno != EEXIST || stat(path_cpy_p, &st) == -1) {
                free(path_cpy_p);
                return -1;
            }

            if (!S_ISDIR(st.st_mode)) {
                errno = ENOTDIR;
                free(path_cpy_p);
                return -1;
            }
        }

        if (sep == '\0') {
            break;
        }
        *char_p = '/';
    }

    free(path_cpy_p);
    return EXIT_SUCCESS;
}

const char* gethome(void) {
    // Check the program is running under sudo
    const char* sudo_user_p = getenv("SUDO_USER");

    if (sudo_user_p != nullptr) {
        // Look up the system record for the original user
        const struct passwd* pw_p = getpwnam(sudo_user_p);
        if (pw_p != nullptr) {
            return pw_p->pw_dir;
        }
    }

    // If not running under sudo, use the standard HOME variable
    const char* home_p = getenv("HOME");
    if (home_p != nullptr) {
        return home_p;
    }

    // Look up the current active UID record
    const struct passwd* pw_p = getpwuid(getuid());
    return pw_p != nullptr ? pw_p->pw_dir : nullptr;
}

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

