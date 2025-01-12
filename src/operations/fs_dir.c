#include "fs_dir.h"

#include <fs_fat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs_files.h"

void fs_dir_ls(const fs_dir_entry_t *dir) {
    for (uint32_t i = 0; i < FS_MAX_ENTRIES_PER_BLOCK; i++) {
        if (dir[i].size != 0) {
            printf("%s\n", dir[i].name);
        }
    }
}

static fs_dir_entry_t fs_get_dir_entry(fs_ctx_t *fs_ctx, char dir_name[FS_FILENAME_MAX]) {
    fs_dir_entry_t dir = {0};
    for (uint32_t i = 0; i < FS_MAX_ENTRIES_PER_BLOCK; i++) {
        if (strcmp(fs_ctx->work_dir[i].name, "") == 0) {
            break;
        } else if (fs_ctx->work_dir[i].attr == FS_ATTR_DIR && strcmp(fs_ctx->work_dir[i].name, dir_name) == 0) {
            dir = fs_ctx->work_dir[i];
            break;
        }
    }
    return dir;
}

uint32_t fs_get_dir_entry_count(const fs_ctx_t *fs_ctx) {
    uint32_t count = 0;
    for (uint32_t i = 0; i < FS_MAX_ENTRIES_PER_BLOCK; i++) {
        if (fs_ctx->root_dir[i].first_block != 0) {
            count++;
        }
    }
    return count;
}
uint32_t fs_dir_save(fs_ctx_t *fs_ctx, fs_dir_entry_t *dir, fs_dir_entry_t dir_entry) {
    FILE *fs_file = fopen(fs_ctx->name, "rb+");
    if (fs_file == NULL) {
        return -1;
    }
    fseek(fs_file, dir_entry.first_block * FS_BLOCK_SIZE, SEEK_SET);
    if (fwrite(dir, sizeof(fs_dir_entry_t), FS_MAX_ENTRIES_PER_BLOCK, fs_file) == 0) {
        fclose(fs_file);
        return -1;
    }
    fclose(fs_file);
    return 0;
}

int fs_dir_add_entry(fs_ctx_t *fs_ctx, fs_dir_entry_t entry) {
    if (fs_ctx->work_dir[FS_MAX_ENTRIES_PER_BLOCK - 1].first_block != 0) {
        printf("Diretório cheio\n");
        return EXIT_FAILURE;
    }
    for (uint32_t i = 0; i < FS_MAX_ENTRIES_PER_BLOCK; i++) {
        if (fs_ctx->work_dir[i].first_block == 0) {
            fs_ctx->work_dir[i] = entry;
            if (fs_ctx->work_dir_entry.first_block == fs_ctx->header.root_block) {
                fs_ctx->root_dir[i] = entry;
            }
            break;
        }
    }
    fs_dir_save(fs_ctx, fs_ctx->work_dir, fs_ctx->work_dir_entry);
    return EXIT_SUCCESS;
}

int fs_create_dir(fs_ctx_t *fs_ctx, char *name) {
    uint32_t dir_block = fs_fat_get_next(fs_ctx);
    if (dir_block == EOF) {
        printf("Não há espaço livre no disco\n");
        return EXIT_FAILURE;
    }

    // Verificar se esse diretório já existe
    if (fs_get_dir_entry(fs_ctx, name).first_block != 0) {
        printf("Diretório %s já existe\n", name);
        return EXIT_FAILURE;
    }

    fs_dir_entry_t new_dir_entry = fs_new_file(fs_ctx, name, dir_block, 0, FS_ATTR_DIR);
    if (new_dir_entry.first_block == 0) {
        printf("Erro ao criar diretório\n");
        return EXIT_FAILURE;
    }

    fs_dir_entry_t *new_dir = calloc(FS_MAX_ENTRIES_PER_BLOCK, sizeof(fs_dir_entry_t));
    fs_dir_save(fs_ctx, new_dir, new_dir_entry);
    free(new_dir);
    return EXIT_SUCCESS;
}


static int fs_get_dir(fs_ctx_t *fs_ctx, uint32_t dir_block, fs_dir_entry_t *dir) {
    if (dir_block == fs_ctx->header.root_block) {
        memcpy(dir, fs_ctx->root_dir, FS_MAX_ENTRIES_PER_BLOCK * sizeof(fs_dir_entry_t));
        return EXIT_SUCCESS;
    }
    FILE *fs_file = fopen(fs_ctx->name, "rb");
    if (fs_file == NULL) {
        perror("Erro ao abrir o arquivo");
        return EXIT_FAILURE;
    }
    fseek(fs_file, dir_block * FS_BLOCK_SIZE, SEEK_SET);
    if (fread(dir, sizeof(fs_dir_entry_t), FS_MAX_ENTRIES_PER_BLOCK, fs_file) == 0) {
        perror("Erro ao ler o diretório");
        fclose(fs_file);
        return EXIT_FAILURE;
    }
    fclose(fs_file);
    return EXIT_SUCCESS;
}

void fs_set_work_dir(fs_ctx_t *fs_ctx, char *path) {
    if (fs_ctx == NULL || path == NULL)
        return;
    if (strcmp(path, "/") == 0) {
        memcpy(fs_ctx->work_dir, fs_ctx->root_dir, sizeof(fs_dir_entry_t) * FS_MAX_ENTRIES_PER_BLOCK);
        fs_ctx->work_dir_entry = fs_ctx->root_dir[0];
        return;
    }
    fs_dir_entry_t dir_entry = fs_find_dir(fs_ctx, path);
    if (strcmp(dir_entry.name, "") == 0) {
        printf("Diretório não encontrado\n");
        return;
    }
    fs_ctx->work_dir_entry = dir_entry;
    fs_dir_entry_t *cd = malloc(sizeof(fs_dir_entry_t) * FS_MAX_ENTRIES_PER_BLOCK);
    fs_get_dir(fs_ctx, dir_entry.first_block, cd);
    memcpy(fs_ctx->work_dir, cd, FS_MAX_ENTRIES_PER_BLOCK * sizeof(fs_dir_entry_t));
    free(cd);
}

fs_dir_entry_t fs_find_dir(fs_ctx_t *fs_ctx, char *path) {
    fs_dir_entry_t dir_entry = {0};
    if (fs_ctx == NULL || path == NULL)
        return dir_entry;
    fs_dir_entry_t *currrent_dir = malloc(FS_MAX_ENTRIES_PER_BLOCK * sizeof(fs_dir_entry_t));

    if (strncmp(path, "/", 1) == 0) {
        memcpy(currrent_dir, fs_ctx->root_dir, FS_MAX_ENTRIES_PER_BLOCK * sizeof(fs_dir_entry_t));
    }
    else {
        memcpy(currrent_dir, fs_ctx->work_dir, FS_MAX_ENTRIES_PER_BLOCK * sizeof(fs_dir_entry_t));
    }

    char *path_copy = strdup(path);
    char *state;
    char *token = strtok_r(path_copy, "/", &state);

    while (token != NULL) {
        uint32_t i = 0;

        for (i = 0; i < FS_MAX_ENTRIES_PER_BLOCK; i++) {
            if (currrent_dir[i].first_block == 0) {
                break;
            } else if (strcmp(currrent_dir[i].name, token) == 0) {
                dir_entry = currrent_dir[i];
                fs_get_dir(fs_ctx, currrent_dir[i].first_block, currrent_dir);

                break;
            }
        }
        token = strtok_r(NULL, "/", &state);
    }

    free(currrent_dir);
    free(path_copy);
    return dir_entry;
}

uint32_t fs_delete_dir(fs_ctx_t *fs_ctx, const char *dirname) {
    if (strcmp(dirname, "/") == 0) {
        printf("Não é possível deletar o diretório raiz\n");
        return EXIT_FAILURE;
    }

    uint32_t dir_index = fs_file_find(fs_ctx->work_dir, dirname);
    if (dir_index == EOF) {
        printf("Diretório não encontrado\n");
        return EXIT_FAILURE;
    }

    fs_dir_entry_t *entry = &fs_ctx->work_dir[dir_index];
    if (entry->attr != FS_ATTR_DIR) {
        printf("Não é um diretório\n");
        return EXIT_FAILURE;
    }

    // Verifica se o diretório está vazio
    fs_dir_entry_t *dir_entries = malloc(sizeof(fs_dir_entry_t) * FS_MAX_ENTRIES_PER_BLOCK);
    FILE *fs_file = fopen(fs_ctx->name, "rb");
    fseek(fs_file, entry->first_block * FS_BLOCK_SIZE, SEEK_SET);
    fread(dir_entries, sizeof(fs_dir_entry_t), FS_MAX_ENTRIES_PER_BLOCK, fs_file);
    fclose(fs_file);

    for (int i = 0; i < FS_MAX_ENTRIES_PER_BLOCK; i++) {
        if (dir_entries[i].first_block != 0) {
            printf("Diretório não está vazio\n");
            free(dir_entries);
            return EXIT_FAILURE;
        }
    }
    free(dir_entries);

    // Remove a entrada do diretório
    memset(entry, 0, sizeof(fs_dir_entry_t));
    fs_dir_save(fs_ctx, fs_ctx->work_dir, fs_ctx->work_dir_entry);
    printf("Diretório excluído com sucesso\n");
    return EXIT_SUCCESS;
}

