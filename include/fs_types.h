#ifndef FS_TYPES_H
#define FS_TYPES_H

#include <stdint.h>

#define FS_BLOCK_SIZE 0x1000 // 4KB
#define FS_FILENAME_MAX 0x20 // 32 bytes
#define FS_MIN_SIZE 0x100000 // 1MB
#define FS_MAX_SIZE 0x40000000 // 1GB
#define FS_MAX_ENTRIES_PER_BLOCK (FS_BLOCK_SIZE / sizeof(fs_dir_entry_t))

#define FS_ATTR_READ_ONLY 0x01
#define FS_ATTR_PROTECT 0x02
#define FS_ATTR_DIR 0x03

// File system header
typedef struct {
  uint8_t header_size;
  uint16_t block_size;
  uint32_t fs_size;
  uint32_t fat_size;
  uint32_t blocks_number;
  uint32_t fat_block;
  uint32_t root_block;
  uint32_t data_block;
} fs_header_t;

typedef struct {
  char name[FS_FILENAME_MAX];
  uint32_t size;
  uint32_t first_block;
  uint8_t attr;
  uint32_t parent_block;
} fs_dir_entry_t;

typedef struct {
  char name[FS_FILENAME_MAX];
  uint32_t size;
  fs_header_t header;
  uint32_t *fat;
  uint32_t last_block;
  fs_dir_entry_t root_dir[FS_MAX_ENTRIES_PER_BLOCK];
  fs_dir_entry_t *work_dir;
  fs_dir_entry_t work_dir_entry;
} fs_ctx_t;

#endif //FS_TYPES_H