#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "task.h"
#include "executor.h"
#include "redirect.h"
#include "jobs.h"

char workdir_atual[512] = "";

void comando_workdir(char *linha) {
    char *token = strtok(linha, " ");
    token = strtok(NULL, " ");
    if (token == NULL) {
        printf("Erro: comando 'workdir' pede diretório.\n");
        return;
    }
    struct stat info;
    if (stat(token, &info) != 0 || !S_ISDIR(info.st_mode)) {
        printf("Erro: diretorio '%s' não existe.\n", token);
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

int processar_comando(char *linha) {
    if (strlen(linha) == 0) {
        return 1;
    }
    if (strcmp(linha, "exit") == 0) {
        return 0;
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
    else if (check_comando(linha, "start")) {
        comando_start(linha);
    }
    else if (check_comando(linha, "jobs")) {
        comando_jobs(linha);
    }
    else if (check_comando(linha, "wait")) {
        comando_wait_job(linha);
    }
    else {
        printf("Comando desconhecido: '%s'\n", linha);
    }

    return 1;
}

static void modo_interativo(void) {
    char linha[1024];

    while (1) {
        printf("processflow> ");
        fflush(stdout);

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            printf("\n");
            break;
        }

        linha[strcspn(linha, "\n")] = '\0';

        if (!processar_comando(linha)) {
            break;
        }
    }
}

static int modo_workflow(const char *caminho_arquivo) {
    FILE *arquivo = fopen(caminho_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo de workflow");
        return 1;
    }

    char linha[1024];
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        linha[strcspn(linha, "\n")] = '\0';
        printf("%s\n", linha);

        if (!processar_comando(linha)) {
            break;
        }
    }

    fclose(arquivo);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        fprintf(stderr, "Uso: %s [workflowFile]\n", argv[0]);
        return 1;
    }

    int erro = 0;
    if (argc == 2) {
        erro = modo_workflow(argv[1]);
    } else {
        modo_interativo();
    }

    printf("ProcessFlow encerrado.\n");
    return erro;
}
