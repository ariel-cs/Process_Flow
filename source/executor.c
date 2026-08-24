#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "executor.h"
#include "task.h"
#include "processflow.h"

void child_applyexec(Task *t) {
    execvp(t->programa, t->args);
    perror("execvp");
    exit(1);
}

int executar_task(Task *t) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        child_applyexec(t);
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

static void run_sequential(char *nomes[], int n) {
    for (int i = 0; i < n; i++) {
        Task *t = buscar_task(nomes[i]);
        if (t == NULL) {
            printf("Erro: tarefa '%s' não encontrada.\n", nomes[i]);
            continue;
        }
        executar_task(t);
    }
}

static void run_parallel(char *nomes[], int n) {
    Task *tasks[MAX_LIST_TASKS];
    pid_t pids[MAX_LIST_TASKS];
    int total = 0;

    for (int i = 0; i < n; i++) {
        Task *t = buscar_task(nomes[i]);
        if (t == NULL) {
            printf("Erro: tarefa '%s' não encontrada.\n", nomes[i]);
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }
        if (pid == 0) {
            child_applyexec(t);
        }

        tasks[total] = t;
        pids[total] = pid;
        total++;
    }

    for (int i = 0; i < total; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status)) {
            int codigo = WEXITSTATUS(status);
            if (codigo != 0) {
                printf("Aviso: tarefa '%s' terminou com código de saída %d.\n",
                       tasks[i]->nome, codigo);
            }
        }
    }
}

void comando_run(char *linha) {
    char *token = strtok(linha, " ");
    token = strtok(NULL, " ");

    if (token == NULL) {
        printf("Erro: comando 'run' requer um nome de tarefa ou modo.\n");
        return;
    }

    if (strcmp(token, "sequential") == 0 || strcmp(token, "parallel") == 0) {
        char modo[16];
        strncpy(modo, token, sizeof(modo) - 1);
        modo[sizeof(modo) - 1] = '\0';

        char *nomes[MAX_LIST_TASKS];
        int n = 0;
        while ((token = strtok(NULL, " ")) != NULL && n < MAX_LIST_TASKS) {
            nomes[n++] = token;
        }

        if (n == 0) {
            printf("Erro: 'run %s' requer ao menos uma tarefa.\n", modo);
            return;
        }

        if (strcmp(modo, "sequential") == 0) run_sequential(nomes, n);
        else run_parallel(nomes, n);
        return;
    }

    Task *encontrada = buscar_task(token);

    if (encontrada == NULL) {
        printf("Erro: tarefa '%s' não encontrada.\n", token);
        return;
    }

    executar_task(encontrada);
}
