
#ifndef FS_DIR_H
#define FS_DIR_H

#include "fs_types.h"

uint32_t fs_get_dir_entry_count(const fs_ctx_t *fs_ctx);

int fs_dir_add_entry(fs_ctx_t *fs_ctx, fs_dir_entry_t entry);

void fs_dir_ls(const fs_dir_entry_t *dir);

int fs_create_dir(fs_ctx_t *fs_ctx, char *name);

fs_dir_entry_t fs_find_dir(fs_ctx_t *fs_ctx, char *path);

void fs_set_work_dir(fs_ctx_t *fs_ctx, char *path);

uint32_t fs_dir_save(fs_ctx_t *fs_ctx, fs_dir_entry_t *dir, fs_dir_entry_t dir_entry);

fs_dir_entry_t fs_find_dir(fs_ctx_t *fs_ctx, char *path) ;

uint32_t fs_delete_dir(fs_ctx_t *fs_ctx, const char *dirname) ;

#endif //FS_DIR_H

