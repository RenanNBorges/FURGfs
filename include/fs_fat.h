#ifndef FS_FAT_H
#define FS_FAT_H

#include "fs_types.h"

uint32_t fs_fat_get_next(const fs_ctx_t *fs_ctx);

void fs_fat_set_next(const fs_ctx_t *fs_ctx, uint32_t block, uint32_t next);

uint32_t fs_fat_free_count(const fs_ctx_t *fs_ctx);

void fs_fat_alloc_blocks(fs_ctx_t *fs_ctx, uint32_t blocks_needed);

#endif //FS_FAT_H
