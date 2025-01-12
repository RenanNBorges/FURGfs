//
// Created by devborges on 07/01/25.
//

#include "fs_fat.h"
#include <stdio.h>

void fs_fat_set_next(const fs_ctx_t *fs_ctx, uint32_t block, uint32_t next) {
  fs_ctx->fat[block] = next;

};

uint32_t fs_fat_get_next(const fs_ctx_t *fs_ctx) {
  for (uint32_t i = fs_ctx->last_block; i < fs_ctx->header.blocks_number; i++) {
    if (fs_ctx->fat[i] == 0) {
      return i;
    }
  }
  if (fs_fat_free_count(fs_ctx) > 0) {
    for (uint32_t i = 2; i < fs_ctx->last_block; i++) {
      if (fs_ctx->fat[i] == 0) {
        return i;
      }
    }
  }
  return EOF;
};

uint32_t fs_fat_free_count(const fs_ctx_t *fs_ctx) {
  uint32_t count = 0;
  for (uint32_t i = 0; i < fs_ctx->header.blocks_number; i++) {
    if (fs_ctx->fat[i] == 0) {
      count++;
    }
  }
  return count;
};


void fs_fat_alloc_blocks(fs_ctx_t *fs_ctx, uint32_t blocks_needed) {
  uint32_t block = fs_fat_get_next(fs_ctx);
  fs_ctx->last_block = block + 1;
  for (uint32_t i = 1; i < blocks_needed; i++) {
    fs_ctx->last_block = block + 1;
    uint32_t next_block = fs_fat_get_next(fs_ctx);
    fs_fat_set_next(fs_ctx, block, next_block);
    block = next_block;
  }
  fs_fat_set_next(fs_ctx, block, EOF);

};