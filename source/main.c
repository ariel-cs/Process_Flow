#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "task.h"
#include "executor.h"
#include "redirect.h"

char workdir_atual[512] = "";

void comando_workdir(char *linha){
    char *token = strtok(linha, " ");
    token = strtok(NULL, " ");

    if(token == NULL){
        printf("Erro: comando 'workdir' pede diretório.\n");
        return;
    }

    struct stat info;
    if (stat(token, &info) != 0 || !S_ISDIR(info.st_mode)) {
        printf("Erro: diretorio não existe.\n");
        return;
    }

    strncpy(workdir_atual, token, sizeof(workdir_atual) - 1);
    workdir_atual[sizeof(workdir_atual) - 1] = '\0';
    printf("Diretorio de trabalho alterado para '%s'.\n", workdir_atual);
}

int check_comando(const char *linha, const char *comando) {
    size_t len = strlen(comando);
    if (strncmp(linha, comando, len) != 0) {
        return 0;
    }
    return linha[len] == '\0' || linha[len] == ' ';
}

int main(int argc, char *argv[]) {
    char linha[1024];

    (void)argc;
    (void)argv;

    while (1) {
        printf("processflow> ");
        fflush(stdout);

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            printf("\n");
            break;
        }

        linha[strcspn(linha, "\n")] = '\0';

        if (strlen(linha) == 0) {
            continue;
        }

        if (strcmp(linha, "exit") == 0) {
            break;
        }

        if (check_comando(linha, "task")) {
            comando_task(linha);
        }
        else if (check_comando(linha, "run")) {
            comando_run(linha);
        }
        else if (check_comando(linha, "input")) {
            comando_input(linha);
        }
        else if (check_comando(linha, "output")) {
            comando_output(linha);
        }
        else if (check_comando(linha, "append")) {
            comando_append(linha);
        }
        else if (check_comando(linha, "workdir")) {
            comando_workdir(linha);
        }
        else {
            printf("Comando desconhecido: '%s'\n", linha);
        }
    }

    printf("ProcessFlow encerrado.\n");
    return 0;
}
