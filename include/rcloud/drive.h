#ifndef RCLOUD_DRIVE_H
#define RCLOUD_DRIVE_H

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

extern char* rcloud_drive_img_path(void);

extern char* rcloud_drive_mnt_path(void);

extern int rcloud_drive_create(const char* img_path, const char* mnt_path);

extern int rcloud_drive_destroy(const char* img_path, const char* mnt_path);

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif // #ifndef RCLOUD_DRIVE_H
