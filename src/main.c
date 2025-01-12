#include <fs_dir.h>
#include <fs_files.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs_core.h"

void print_main_menu() {
    printf("\n=== Sistema de Arquivos FURG ===\n");
    printf("1. Criar novo sistema de arquivos\n");
    printf("2. Montar sistema de arquivos existente\n");
    printf("0. Sair\n");
    printf("Escolha uma opção: ");
}

void print_operations_menu() {
    printf("\n=== Operações do Sistema de Arquivos ===\n");
    printf("1. Criar diretório\n");
    printf("2. Copiar arquivo para dentro\n");
    printf("3. Copiar arquivo para fora\n");
    printf("4. Renomear arquivo\n");
    printf("5. Excluir arquivo\n");
    printf("6. Ver informações gerais\n");
    printf("7. Listar arquivos\n");
    printf("8. Mudar diretório atual\n");
    printf("9. Proteger arquivo\n");
    printf("10. Excluir diretório\n");
    printf("0. Desmontar e sair\n");
    printf("Escolha uma opção: ");
}
void print_protection_menu() {
    printf("\n=== Tipo de Proteção ===\n");
    printf("1. Proteger contra escrita (somente leitura)\n");
    printf("2. Proteger contra exclusão\n");
    printf("0. Voltar\n");
    printf("Escolha uma opção: ");
}

// Função para limpar o buffer de entrada
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

// Função para validar o tamanho do nome
int validate_name(const char *name) {
    if (strlen(name) >= FS_FILENAME_MAX) {
        printf("Erro: Nome muito longo. Máximo %d caracteres.\n", FS_FILENAME_MAX - 1);
        return 0;
    }
    return 1;
}

int main() {
    fs_ctx_t *fs_ctx = NULL;
    char filename[256];
    int option;

    while (1) {
        print_main_menu();
        if (scanf("%d", &option) != 1) {
            clear_input_buffer();
            printf("Entrada inválida!\n");
            continue;
        }
        clear_input_buffer();

        if (option == 0) {
            if (fs_ctx != NULL) {
                fs_umount(fs_ctx);
                printf("Sistema de arquivos desmontado.\n");
            }
            printf("Saindo do programa...\n");
            break;
        }

        switch (option) {
            case 1: { // Criar novo sistema de arquivos
                printf("Digite o nome do sistema de arquivos (sem .fs): ");
                if (fgets(filename, sizeof(filename), stdin) != NULL) {
                    filename[strcspn(filename, "\n")] = 0; // Remove quebra de linha

                    if (!validate_name(filename)) {
                        break;
                    }

                    strcat(filename, ".fs");

                    uint32_t size;
                    do {
                        printf("Digite o tamanho em bytes (min: %d, max: %d): ", FS_MIN_SIZE, FS_MAX_SIZE);
                        if (scanf("%u", &size) != 1) {
                            clear_input_buffer();
                            printf("Entrada inválida!\n");
                            size = 0;
                            continue;
                        }
                        clear_input_buffer();

                        if (size < FS_MIN_SIZE || size > FS_MAX_SIZE) {
                            printf("Tamanho inválido!\n");
                        }
                    } while (size < FS_MIN_SIZE || size > FS_MAX_SIZE);

                    if (fs_format(filename, size) == 0) {
                        printf("Sistema de arquivos criado com sucesso!\n");
                    } else {
                        printf("Erro ao criar sistema de arquivos!\n");
                    }
                }
                break;
            }

            case 2: { // Montar sistema existente
                printf("Digite o nome do sistema de arquivos (com .fs): ");
                if (fgets(filename, sizeof(filename), stdin) != NULL) {
                    filename[strcspn(filename, "\n")] = 0; // Remove quebra de linha

                    // Se já existe um sistema montado, desmonta primeiro
                    if (fs_ctx != NULL) {
                        fs_umount(fs_ctx);
                        fs_ctx = NULL;
                    }

                    fs_ctx = initialize_fs_ctx(filename, FS_MIN_SIZE);
                    if (fs_mount(fs_ctx) != 0) {
                        printf("Erro ao montar sistema de arquivos!\n");
                        free(fs_ctx);
                        fs_ctx = NULL;
                        continue;
                    }

                    printf("Sistema de arquivos montado com sucesso!\n");

                    // Menu de operações
                    while (1) {
                        print_operations_menu();
                        if (scanf("%d", &option) != 1) {
                            clear_input_buffer();
                            printf("Entrada inválida!\n");
                            continue;
                        }
                        clear_input_buffer();

                        if (option == 0) {
                            fs_umount(fs_ctx);
                            fs_ctx = NULL;
                            printf("Sistema de arquivos desmontado.\n");
                            break;
                        }

                        char name[FS_FILENAME_MAX];
                        char path[256];

                        switch (option) {
                            case 1: // Criar diretório
                                printf("Digite o nome do diretório: ");
                                if (fgets(name, sizeof(name), stdin) != NULL) {
                                    name[strcspn(name, "\n")] = 0;
                                    if (!validate_name(name)) {
                                        break;
                                    }
                                    fs_create_dir(fs_ctx, name);
                                }
                                break;

                            case 2: // Copiar arquivo para dentro
                                printf("Digite o caminho completo do arquivo de origem: ");
                                if (fgets(path, sizeof(path), stdin) != NULL) {
                                    path[strcspn(path, "\n")] = 0;
                                    fs_file_copy_from(fs_ctx, path);
                                }
                                break;

                            case 3: // Copiar arquivo para fora
                                printf("Digite o nome do arquivo a ser copiado: ");
                                if (fgets(name, sizeof(name), stdin) != NULL) {
                                    name[strcspn(name, "\n")] = 0;
                                    if (!validate_name(name)) {
                                        break;
                                    }
                                    printf("Digite o caminho de destino: ");
                                    if (fgets(path, sizeof(path), stdin) != NULL) {
                                        path[strcspn(path, "\n")] = 0;
                                        fs_file_copy_to(fs_ctx, name, path);
                                    }
                                }
                                break;

                            case 4: // Renomear arquivo
                                printf("Digite o nome atual do arquivo: ");
                                if (fgets(name, sizeof(name), stdin) != NULL) {
                                    name[strcspn(name, "\n")] = 0;
                                    if (!validate_name(name)) {
                                        break;
                                    }
                                    char new_name[FS_FILENAME_MAX];
                                    printf("Digite o novo nome: ");
                                    if (fgets(new_name, sizeof(new_name), stdin) != NULL) {
                                        new_name[strcspn(new_name, "\n")] = 0;
                                        if (!validate_name(new_name)) {
                                            break;
                                        }
                                        fs_rename_file(fs_ctx, name, new_name);
                                    }
                                }
                                break;

                            case 5: // Excluir arquivo
                                printf("Digite o nome do arquivo a ser excluído: ");
                                if (fgets(name, sizeof(name), stdin) != NULL) {
                                    name[strcspn(name, "\n")] = 0;
                                    if (!validate_name(name)) {
                                        break;
                                    }
                                    fs_delete_file(fs_ctx, name);
                                }
                                break;

                            case 6: // Ver informações gerais
                                fs_print_info(fs_ctx);
                                break;

                            case 7: // Listar arquivos
                                fs_print_all(fs_ctx, "/", 0);
                                break;

                            case 8: // Mudar diretório
                                printf("Digite o caminho do diretório (/ para raiz): ");
                                if (fgets(path, sizeof(path), stdin) != NULL) {
                                    path[strcspn(path, "\n")] = 0;
                                    fs_set_work_dir(fs_ctx, path);
                                }
                                break;

                            case 9: // Proteger arquivo
                                printf("Digite o nome do arquivo a ser protegido: ");
                                if (fgets(name, sizeof(name), stdin) != NULL) {
                                    name[strcspn(name, "\n")] = 0;
                                    if (!validate_name(name)) {
                                        break;
                                    }

                                    int protection_option;
                                    do {
                                        print_protection_menu();
                                        if (scanf("%d", &protection_option) != 1) {
                                            clear_input_buffer();
                                            printf("Entrada inválida!\n");
                                            continue;
                                        }
                                        clear_input_buffer();

                                        switch (protection_option) {
                                            case 0:
                                                break;
                                            case 1:
                                                fs_file_read_only(fs_ctx->work_dir, name);
                                                printf("Arquivo protegido contra escrita.\n");
                                                break;
                                            case 2:
                                                fs_file_protect(fs_ctx->work_dir, name);
                                                printf("Arquivo protegido contra exclusão.\n");
                                                break;
                                            default:
                                                printf("Opção inválida!\n");
                                        }
                                    } while (protection_option != 0 && protection_option != 1 &&
                                             protection_option != 2);
                                }
                                break;
                            case 10: // Excluir diretório
                                printf("Digite o nome do diretório a ser excluído: ");
                                if (fgets(name, sizeof(name), stdin) != NULL) {
                                    name[strcspn(name, "\n")] = 0;
                                    if (!validate_name(name)) {
                                        break;
                                    }
                                    fs_delete_dir(fs_ctx, name);
                                }
                                break;

                            default:
                                printf("Opção inválida!\n");
                        }
                    }
                }
                break;
            }

            default:
                printf("Opção inválida!\n");
        }
    }

    return 0;
}
