#include "fs_files.h"
#include <stdio.h>
#include "fs_dir.h"
#include "fs_fat.h"

#include <stdlib.h>
#include <string.h>

// Extrai o nome do arquivo de um caminho
static const char *fs_extract_filename(const char *path) {
    const char *filename = strrchr(path, '/');
    if (filename == NULL) {
        return path;
    }
    return filename + 1;
}

static uint32_t fs_file_overwrite(fs_ctx_t fs_ctx, fs_dir_entry_t file_entry) { return EXIT_SUCCESS; }

// Cria um novo arquivo no sistema de arquivos
fs_dir_entry_t fs_new_file(fs_ctx_t *fs_ctx, const char *name, const uint32_t first_block, uint32_t size,
                           uint8_t attr) {
    if (attr == FS_ATTR_DIR) {
        printf("Criando Diretório %s\n", name);
    } else {
        printf("Criando arquivo %s\n", name);
    }

    fs_dir_entry_t entry = {0};

    strncpy(entry.name, name, FS_FILENAME_MAX);
    entry.size = size;
    entry.first_block = first_block;
    entry.attr = attr;
    entry.parent_block = fs_ctx->work_dir_entry.first_block;

    if (fs_dir_add_entry(fs_ctx, entry) == EXIT_FAILURE) {
        return entry;
    };
    fs_fat_alloc_blocks(fs_ctx, (size + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE);
    return entry;
};

uint32_t fs_file_find(const fs_dir_entry_t *dir, const char *name) {
    int i = 0;
    while (dir[i].first_block != 0) {
        if (strcmp(dir[i].name, name) == 0) {
            return i;
        }
        i++;
    }
    return EOF;
}

// Escreve o arquivo no sistema de arquivos
int fs_write_file(const fs_ctx_t *fs_ctx, FILE *src_file, uint32_t first_block) {
    FILE *fs_file = fopen(fs_ctx->name, "rb+");
    if (fs_file == NULL) {
        return -1;
    }
    fseek(src_file, 0, SEEK_SET);
    uint32_t current_block = first_block;
    while (current_block != EOF) {
        char BUFFER[FS_BLOCK_SIZE];
        if (fread(BUFFER, 1, FS_BLOCK_SIZE, src_file) == 0) {
            fclose(fs_file);
            return -1;
        }
        fseek(fs_file, current_block * FS_BLOCK_SIZE, SEEK_SET);
        fwrite(BUFFER, FS_BLOCK_SIZE, 1, fs_file);
        current_block = fs_ctx->fat[current_block];
    };

    fclose(fs_file);
    return EXIT_SUCCESS;
}

// Copia um arquivo do sistema de arquivos real para o sistema de arquivos virtual
uint32_t fs_file_copy_from(fs_ctx_t *fs_ctx, const char *src_path) {

    FILE *src_file = fopen(src_path, "rb");
    if (src_file == NULL) {
        return -1;
    }

    const char *filename = fs_extract_filename(src_path);

    uint32_t file_index = fs_file_find(fs_ctx->work_dir, filename);
    if (file_index != EOF) {
        printf("Arquivo já existe\n");
        if (fs_ctx->work_dir[file_index].attr == FS_ATTR_READ_ONLY) {
            printf("Arquivo é somente leitura\n");
            return EXIT_FAILURE;
        }

        printf("Deseja sobrescrever o arquivo? (s/n): ");
        char response;
        scanf("%c", &response);
        if (response != 's') {
            fs_ctx->work_dir[file_index].attr = 0;
            fs_delete_file(fs_ctx, filename);
        } else {
            return EXIT_FAILURE;
        }

        return EXIT_FAILURE;
    }

    fseek(src_file, 0, SEEK_END);
    uint32_t src_size = ftell(src_file);
    uint32_t src_blocks = (src_size + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE;
    fs_dir_entry_t src_entry = {0};
    src_entry = fs_new_file(fs_ctx, filename, fs_fat_get_next(fs_ctx), src_blocks * FS_BLOCK_SIZE, 0);

    if (fs_write_file(fs_ctx, src_file, src_entry.first_block) != EXIT_SUCCESS) {
        fclose(src_file);
        return -1;
    }
    fclose(src_file);
    return EXIT_SUCCESS;
}

// Copia um arquivo do sistema de arquivos virtual para o sistema de arquivos real
uint32_t fs_file_copy_to(const fs_ctx_t *fs_ctx, const char *filename, const char *path) {
    FILE *fs_file = fopen(fs_ctx->name, "rb");
    if (fs_file == NULL) {
        return -1;
    }
    FILE *dst_file = fopen(path, "wb");
    if (dst_file == NULL) {
        fclose(fs_file);
        return -1;
    }

    fs_dir_entry_t entry = {0};

    uint32_t file_index = fs_file_find(fs_ctx->work_dir, filename);
    if (file_index == EOF) {
        printf("Arquivo não encontrado\n");
        return EXIT_FAILURE;
    };
    entry = fs_ctx->work_dir[file_index];

    uint32_t current_block = entry.first_block;

    while (current_block != EOF) {
        char BUFFER[FS_BLOCK_SIZE];
        fseek(fs_file, current_block * FS_BLOCK_SIZE, SEEK_SET);
        fread(BUFFER, FS_BLOCK_SIZE, 1, fs_file);
        fwrite(BUFFER, FS_BLOCK_SIZE, 1, dst_file);
        current_block = fs_ctx->fat[current_block];
    }
    fclose(fs_file);
    fclose(dst_file);
    return EXIT_SUCCESS;
}

uint32_t fs_file_read_only(fs_dir_entry_t *dir, const char name[FS_FILENAME_MAX]) {
    uint32_t file_index = fs_file_find(dir, name);
    if (file_index == EOF) {
        printf("Arquivo não encontrado\n");
        return EXIT_FAILURE;
    }

    dir[file_index].attr = FS_ATTR_READ_ONLY;
    return EXIT_SUCCESS;
};

uint32_t fs_file_protect(fs_dir_entry_t *dir, const char name[FS_FILENAME_MAX]) {
    uint32_t file_index = fs_file_find(dir, name);
    if (file_index == EOF) {
        printf("Arquivo não encontrado\n");
        return EXIT_FAILURE;
    }

    dir[file_index].attr = FS_ATTR_READ_ONLY;
    return EXIT_SUCCESS;
};

uint32_t fs_delete_file(fs_ctx_t *fs_ctx, const char *filename) {
    uint32_t file_index = fs_file_find(fs_ctx->work_dir, filename);
    if (file_index == EOF) {
        printf("Arquivo não encontrado\n");
        return EXIT_FAILURE;
    }

    fs_dir_entry_t *entry = &fs_ctx->work_dir[file_index];

    if (entry->attr == FS_ATTR_PROTECT) {
        printf("Arquivo protegido contra exclusão\n");
        return EXIT_FAILURE;
    }

    // Liberar os blocos alocados pelo arquivo
    uint32_t current_block = entry->first_block;
    while (current_block != EOF) {
        uint32_t next_block = fs_ctx->fat[current_block];
        fs_ctx->fat[current_block] = 0; // Marcar bloco como livre
        current_block = next_block;
    }

    // Remover a entrada do diretório
    memset(entry, 0, sizeof(fs_dir_entry_t));
    fs_dir_save(fs_ctx, fs_ctx->work_dir, fs_ctx->work_dir_entry);
    printf("Arquivo excluído!\n");
    return EXIT_SUCCESS;
}

void fs_print_all(fs_ctx_t *fs_ctx, const char *path, int level) {
    fs_dir_entry_t *curr_dir = malloc(sizeof(fs_dir_entry_t) * FS_MAX_ENTRIES_PER_BLOCK);

    // Se o path for raiz, usa o root_dir, senão busca o diretório
    if (strcmp(path, "/") == 0) {
        memcpy(curr_dir, fs_ctx->root_dir, sizeof(fs_dir_entry_t) * FS_MAX_ENTRIES_PER_BLOCK);
    } else {
        fs_dir_entry_t dir = fs_find_dir(fs_ctx, path);
        FILE *fs_file = fopen(fs_ctx->name, "rb");
        fseek(fs_file, dir.first_block * FS_BLOCK_SIZE, SEEK_SET);
        fread(curr_dir, sizeof(fs_dir_entry_t), FS_MAX_ENTRIES_PER_BLOCK, fs_file);
        fclose(fs_file);
    }

    // Itera sobre as entradas do diretório
    for (int i = 0; i < FS_MAX_ENTRIES_PER_BLOCK; i++) {
        if (curr_dir[i].first_block == 0)
            continue;

        // Imprime indentação
        for (int j = 0; j < level; j++) {
            printf("  ");
        }

        // Imprime nome e tipo
        if (curr_dir[i].attr == FS_ATTR_DIR) {
            if (strcmp(curr_dir[i].name, "/") == 0) {
                printf("📁 root/\n");
            } else {
                printf("📁 %s/\n", curr_dir[i].name);
            }

            // Se for diretório, chama recursivamente
            char new_path[256];
            if (strcmp(path, "/") == 0) {
                snprintf(new_path, sizeof(new_path), "/%s", curr_dir[i].name);
            } else {
                snprintf(new_path, sizeof(new_path), "%s/%s", path, curr_dir[i].name);
            }
            fs_print_all(fs_ctx, new_path, level + 1);
        } else {
            printf("📄 %s (%d bytes)\n", curr_dir[i].name, curr_dir[i].size);
        }
    }

    free(curr_dir);
}


uint32_t fs_rename_file(fs_ctx_t *fs_ctx, const char *old_name, const char *new_name) {
    uint32_t file_index = fs_file_find(fs_ctx->work_dir, old_name);
    if (file_index == EOF) {
        printf("Arquivo não encontrado\n");
        return EXIT_FAILURE;
    }

    uint32_t new_file_index = fs_file_find(fs_ctx->work_dir, new_name);
    if (new_file_index != EOF) {
        printf("Já existe um arquivo com o novo nome\n");
        return EXIT_FAILURE;
    }

    strncpy(fs_ctx->work_dir[file_index].name, new_name, FS_FILENAME_MAX);
    fs_ctx->work_dir[file_index].name[FS_FILENAME_MAX] = '\0'; // Garante que o nome seja nulo-terminado

    fs_dir_save(fs_ctx, fs_ctx->work_dir, fs_ctx->work_dir_entry);

    printf("Arquivo renomeado com sucesso\n");
    return EXIT_SUCCESS;
}