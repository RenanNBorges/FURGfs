#ifndef FS_FILES_H
#define FS_FILES_H

#include "fs_types.h"


fs_dir_entry_t fs_new_file(fs_ctx_t *fs_ctx,const char* name, const uint32_t first_block, uint32_t size, uint8_t attr);

uint32_t fs_file_copy_from(fs_ctx_t *fs_ctx, const char* src_path);

uint32_t fs_file_copy_to(const fs_ctx_t *fs_ctx, const char *filename, const char *path) ;

uint32_t fs_file_read_only(fs_dir_entry_t *dir, const char name[FS_FILENAME_MAX]);

uint32_t fs_file_protect(fs_dir_entry_t *dir, const char name[FS_FILENAME_MAX]);

uint32_t fs_delete_file(fs_ctx_t *fs_ctx, const char *filename);

void fs_print_all(fs_ctx_t *fs_ctx, const char *path, int level);

uint32_t fs_rename_file(fs_ctx_t *fs_ctx, const char *old_name, const char *new_name);

uint32_t fs_file_find(const fs_dir_entry_t *dir, const char *name);

#endif //FS_FILES_H
