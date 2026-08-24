#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include "executor.h"
#include "task.h"
#include "processflow.h"

extern char workdir_atual[512];

static void aplicar_workdir(void) {
    if (workdir_atual[0] != '\0') {
        if (chdir(workdir_atual) != 0) {
            perror("chdir");
            exit(1);
        }
    }
}

static void aplicar_input_redirect(const char *arquivo) {
    if (arquivo[0] != '\0') {
        int fd = open(arquivo, O_RDONLY);
        if (fd < 0) {
            perror("open input");
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
}

static void aplicar_output_redirect(const char *arquivo, int append) {
    if (arquivo[0] != '\0') {
        int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
        int fd = open(arquivo, flags, 0644);
        if (fd < 0) {
            perror("open output");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

void child_applyexec(Task *t) {
    aplicar_workdir();
    aplicar_input_redirect(t->input_file);
    aplicar_output_redirect(t->output_file, t->append_mode);

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

static void run_pipe(char *nomes[], int n) {
    if (n < 2) {
        printf("Erro: 'run pipe' requer ao menos 2 tarefas.\n");
        return;
    }

    Task *tarefas[MAX_LIST_TASKS];
    for (int i = 0; i < n; i++) {
        tarefas[i] = buscar_task(nomes[i]);
        if (tarefas[i] == NULL) {
            printf("Erro: tarefa '%s' não encontrada.\n", nomes[i]);
            return;
        }
    }

    int pipes[MAX_LIST_TASKS - 1][2];
    for (int i = 0; i < n - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return;
        }
    }

    pid_t pids[MAX_LIST_TASKS];

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return;
        }

        if (pid == 0) {
            aplicar_workdir();

            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            } else {
                aplicar_input_redirect(tarefas[i]->input_file);
            }

            if (i < n - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            } else {
                aplicar_output_redirect(tarefas[i]->output_file, tarefas[i]->append_mode);
            }

            for (int j = 0; j < n - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(tarefas[i]->programa, tarefas[i]->args);
            perror("execvp");
            exit(1);
        }

        pids[i] = pid;
    }

    for (int i = 0; i < n - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < n; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status)) {
            int codigo = WEXITSTATUS(status);
            if (codigo != 0) {
                printf("Aviso: tarefa '%s' terminou com código de saída %d.\n",
                       tarefas[i]->nome, codigo);
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

    if (strcmp(token, "sequential") == 0 || strcmp(token, "parallel") == 0 || strcmp(token, "pipe") == 0) {
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
        else if (strcmp(modo, "parallel") == 0) run_parallel(nomes, n);
        else run_pipe(nomes, n);
        return;
    }

    Task *encontrada = buscar_task(token);
    if (encontrada == NULL) {
        printf("Erro: tarefa '%s' não encontrada.\n", token);
        return;
    }
    executar_task(encontrada);
}
