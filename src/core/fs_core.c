#include "fs_core.h"

#include <fs_fat.h>
#include <fs_files.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Função para imprimir informações gerais do sistema de arquivos
void fs_print_info(fs_ctx_t *fs_ctx) {
    uint32_t total_size = fs_ctx->header.blocks_number* FS_BLOCK_SIZE;
    uint32_t used_blocks = 0;
    uint32_t free_blocks = 0;

    for (uint32_t i = 0; i < fs_ctx->header.blocks_number; i++) {
        if (fs_ctx->fat[i] == 0) {
            free_blocks++;
        } else {
            used_blocks++;
        }
    }

    uint32_t used_size = used_blocks * FS_BLOCK_SIZE;
    uint32_t free_size = free_blocks * FS_BLOCK_SIZE;

    printf("Informações do Sistema de Arquivos:\n");
    printf("Tamanho Total: %u bytes\n", total_size);
    printf("Tamanho Usado: %u bytes\n", used_size);
    printf("Tamanho Disponível: %u bytes\n", free_size);
}

fs_ctx_t *initialize_fs_ctx(const char *name, uint32_t size) {
    printf("Inicializando contexto do disco virtual\n");
    fs_ctx_t *fs_ctx = calloc(1, sizeof(fs_ctx_t));

    if (fs_ctx == NULL) {
        return NULL;
    }
    strncpy(fs_ctx->name, name, FS_FILENAME_MAX);
    fs_ctx->size = size;
    fs_ctx->fat = NULL;
    fs_ctx->last_block = 2;
    return fs_ctx;
};

static int fs_create_header(fs_ctx_t *fs_ctx) {
    printf("Criando cabeçalho do disco virtual\n");
    fs_header_t header = {0};
    header.header_size = FS_BLOCK_SIZE / FS_BLOCK_SIZE;
    header.block_size = FS_BLOCK_SIZE;
    header.fs_size = fs_ctx->size;
    header.blocks_number = fs_ctx->size / FS_BLOCK_SIZE;
    header.fat_size = (header.blocks_number * sizeof(uint32_t) + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE;
    header.fat_block = 1;
    header.data_block = 1 + header.fat_size;
    header.root_block = header.data_block;
    fs_ctx->header = header;
    return 0;
}

static uint32_t *fs_create_fat(const fs_ctx_t *fs_ctx) {
    printf("Criando FAT do disco virtual\n");
    uint *fat = (uint32_t *) calloc(fs_ctx->header.blocks_number, sizeof(uint32_t));
    if (fat == NULL) {
        return NULL;
    }

    fat[0] = EOF;
    fat[1] = EOF;

    return fat;
}

// Cria o diretório raiz do disco virtual
static int fs_create_root(fs_ctx_t *fs_ctx) {
    printf("Criando Root do disco virtual\n");
    fs_dir_entry_t root_dir = {0};
    strcpy(root_dir.name, "/");
    root_dir.size = 0;
    root_dir.first_block = fs_ctx->header.root_block;
    root_dir.attr = FS_ATTR_DIR;
    fs_fat_set_next(fs_ctx, fs_ctx->header.root_block, EOF);
    fs_ctx->root_dir[0] = root_dir;
    return 0;
}

// Desmonta o disco virtual
void fs_umount(fs_ctx_t *fs_ctx) {
    FILE *fs_file = fopen(fs_ctx->name, "rb+");
    if (fs_ctx->fat != NULL) {
        // Escrever a FAT no arquivo do sistema de arquivos
        fseek(fs_file, FS_BLOCK_SIZE, SEEK_SET);
        if (fwrite(fs_ctx->fat, sizeof(uint32_t), fs_ctx->header.blocks_number, fs_file) == 0) {
            perror("Erro ao escrever a FAT");
        };

        free(fs_ctx->fat);
        fs_ctx->fat = NULL;
    }

    if (fs_ctx->work_dir != NULL) {
        fs_ctx->work_dir = NULL;
    }
    fclose(fs_file);
    free(fs_ctx);
}

// Imprime o cabeçalho do disco virtual
static void fs_header_print(const fs_ctx_t *fs_ctx) {
    printf("========== H E A D E R ==========\n");
    printf("Block size: %d\n", fs_ctx->header.block_size);
    printf("FS size: %d\n", fs_ctx->header.fs_size);
    printf("Block count: %d\n", fs_ctx->header.blocks_number);
    printf("FAT size: %d\n", fs_ctx->header.fat_size);
    printf("FAT block: %d\n", fs_ctx->header.fat_block);
    printf("Root block: %d\n", fs_ctx->header.root_block);
    printf("Data block: %d\n", fs_ctx->header.data_block);
};

// Formata o disco virtual
int fs_format(const char *fs_name, int fs_size) {
    printf("Formatando disco virtual\n");
    fs_ctx_t *fs_ctx = initialize_fs_ctx(fs_name, fs_size);
    if (fs_ctx == NULL) {
        return -1;
    }

    FILE *fs_file = fopen(fs_name, "wb");
    if (fs_file == NULL) {
        fs_umount(fs_ctx);
        return -1;
    }

    fs_create_header(fs_ctx);
    fs_ctx->fat = fs_create_fat(fs_ctx);
    if (fs_ctx->fat == NULL) {
        free(fs_ctx->fat);
        fs_umount(fs_ctx);
        return -1;
    };

    fs_ctx->last_block = fs_ctx->header.data_block;
    fs_create_root(fs_ctx);

    fseek(fs_file, fs_size, SEEK_SET);
    fwrite("\0", 1, 1, fs_file);

    // Escrever o header no arquivo do sistema de arquivos
    fseek(fs_file, 0, SEEK_SET);
    if (fwrite(&fs_ctx->header, sizeof(fs_header_t), 1, fs_file) == 0) {
        fs_umount(fs_ctx);
        return -1;
    };

    fseek(fs_file, fs_ctx->header.root_block * FS_BLOCK_SIZE, SEEK_SET);
    if (fwrite(fs_ctx->root_dir, sizeof(fs_dir_entry_t), FS_MAX_ENTRIES_PER_BLOCK, fs_file) == 0) {
        perror("Erro ao escrever o Root");
    }

    fclose(fs_file);
    fs_umount(fs_ctx);

    return 0;
};

int fs_mount(fs_ctx_t *fs_ctx) {
    FILE *fs_file = fopen(fs_ctx->name, "rb");
    if (fs_file == NULL) {
        return -1;
    }

    size_t bytes_read = 0;
    // Carregar o header
    fseek(fs_file, 0, SEEK_SET);
    bytes_read = fread(&fs_ctx->header, sizeof(fs_header_t), 1, fs_file);
    if (bytes_read == 0) {
        fclose(fs_file);

        perror("Erro ao carregar o cabeçalho");
        return -1;
    }
    fs_header_print(fs_ctx);

    // Carregar a FAT
    fseek(fs_file, FS_BLOCK_SIZE, SEEK_SET);
    fs_ctx->fat = calloc(fs_ctx->header.blocks_number, sizeof(uint32_t));
    bytes_read = fread(fs_ctx->fat, sizeof(uint32_t), fs_ctx->header.blocks_number, fs_file);
    if (bytes_read == 0) {
        fclose(fs_file);
        perror("Erro ao carregar a FAT");
        return -1;
    }
    // Carregar o Root
    fseek(fs_file, fs_ctx->header.root_block * FS_BLOCK_SIZE, SEEK_SET);
    bytes_read = fread(fs_ctx->root_dir, sizeof(fs_dir_entry_t), FS_MAX_ENTRIES_PER_BLOCK, fs_file);
    if (bytes_read == 0) {
        fclose(fs_file);
        perror("Erro ao carregar o Root");
        return -1;
    }
    fclose(fs_file);

    fs_ctx->work_dir = malloc(sizeof(fs_dir_entry_t) * FS_MAX_ENTRIES_PER_BLOCK);
    memcpy(fs_ctx->work_dir, fs_ctx->root_dir, sizeof(fs_dir_entry_t) * FS_MAX_ENTRIES_PER_BLOCK);
    fs_ctx->work_dir_entry = fs_ctx->root_dir[0];
    return 0;
}