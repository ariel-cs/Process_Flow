#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "executor.h"
#include "task.h"

int executar_task(Task *t) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        execvp(t->programa, t->args);
        perror("execvp");
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        int codigo = WEXITSTATUS(status);
        if (codigo != 0) {
            printf("Aviso: tarefa '%s' terminou com código de saída %d.\n", t->nome, codigo);
        }
        return codigo;
    }

    return -1;
}

void comando_run(char *linha) {
    char *token = strtok(linha, " ");
    token = strtok(NULL, " ");

    if (token == NULL) {
        printf("Erro: comando 'run' requer um nome de tarefa.\n");
        return;
    }

    Task *encontrada = buscar_task(token);

    if (encontrada == NULL) {
        printf("Erro: tarefa '%s' não encontrada.\n", token);
        return;
    }

    executar_task(encontrada);
}
