#ifndef EXPORT_H
#define EXPORT_H

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#ifndef RCLOUD_STATIC_DEFINE
#  include <rcloud/export_shared.h>
#else
#  include <rcloud/export_static.h>
#endif // #ifndef RCLOUD_STATIC_DEFINE

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif // #ifndef EXPORT_H

