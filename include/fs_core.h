#ifndef FS_CORE_H
#define FS_CORE_H

#include "fs_types.h"

int fs_format(const char * fs_name, int fs_size);

fs_ctx_t* initialize_fs_ctx(const char* name, uint32_t size);

int fs_mount(fs_ctx_t *fs_ctx);

void fs_umount(fs_ctx_t *fs_ctx);

void fs_print_info(fs_ctx_t *fs_ctx);


#endif //FS_CORE_H
