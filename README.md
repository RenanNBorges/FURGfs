# Sistema de Arquivos FURG

Este projeto implementa um sistema de arquivos simples em C, com funcionalidades básicas como criação, exclusão e manipulação de arquivos e diretórios baseado no sistema FAT.

## Estrutura do Projeto

- `src/`: Contém os arquivos fonte do projeto.
    - `main.c`: Arquivo principal que contém a interface de linha de comando para interagir com o sistema de arquivos.
    - `operations/`: Contém as operações específicas do sistema de arquivos.
        - `fs_dir.c`: Implementação das operações relacionadas a diretórios.
        - `fs_files.c`: Implementação das operações relacionadas a arquivos.
- `include/`: Contém os arquivos de cabeçalho do projeto.
    - `fs_dir.h`: Declarações das funções relacionadas a diretórios.
    - `fs_files.h`: Declarações das funções relacionadas a arquivos.
    - `fs_types.h`: Declarações dos tipos de dados utilizados no sistema de arquivos.
    - `fs_core.h`: Declarações das funções principais do sistema de arquivos.

## Funcionalidades

- Criar novo sistema de arquivos
- Montar sistema de arquivos existente
- Criar diretório
- Copiar arquivo para dentro do sistema de arquivos
- Copiar arquivo para fora do sistema de arquivos
- Renomear arquivo
- Excluir arquivo
- Ver informações gerais do sistema de arquivos
- Listar arquivos
- Mudar diretório atual
- Proteger arquivo contra escrita ou exclusão
- Excluir diretório

## Compilação

Para compilar o projeto, siga os passos abaixo:

1. Crie um diretório `build` na raiz do projeto:
    ```sh
    mkdir build
    ```

2. Navegue até o diretório `build`:
    ```sh
    cd build
    ```

3. Execute o comando `make` para compilar o projeto:
    ```sh
    make
    ```

## Uso

Após a compilação, o executável gerado pode ser utilizado para interagir com o sistema de arquivos. Execute o programa e siga as instruções do menu para realizar as operações desejadas.

```sh
./main
```
