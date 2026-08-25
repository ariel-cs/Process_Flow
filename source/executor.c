#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include "executor.h"

extern char workdir_atual[512];

static void aplicar_workdir(void) {
    if (workdir_atual[0] == '\0') {
        return;
    }
    if (chdir(workdir_atual) != 0) {
        perror("chdir");
        _exit(EXIT_FAILURE);
    }
}

static void redirecionar_entrada(const char *arquivo) {
    int fd = open(arquivo, O_RDONLY);
    if (fd < 0) {
        perror("open input");
        _exit(EXIT_FAILURE);
    }
    if (dup2(fd, STDIN_FILENO) < 0) {
        perror("dup2 input");
        close(fd);
        _exit(EXIT_FAILURE);
    }
    close(fd);
}

static void redirecionar_saida(const char *arquivo, int append) {
    int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
    int fd = open(arquivo, flags, 0644);
    if (fd < 0) {
        perror("open output");
        _exit(EXIT_FAILURE);
    }
    if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2 output");
        close(fd);
        _exit(EXIT_FAILURE);
    }
    close(fd);
}

void child_applyexec(Task *t) {
    aplicar_workdir();

    if (t->input_file[0] != '\0') {
        redirecionar_entrada(t->input_file);
    }
    if (t->output_file[0] != '\0') {
        redirecionar_saida(t->output_file, t->append_mode);
    }

    execvp(t->programa, t->args);
    perror("execvp");
    _exit(EXIT_FAILURE);
}


int processar_status(const char *nome, int status) {
    if (WIFEXITED(status)) {
        int codigo = WEXITSTATUS(status);
        if (codigo != 0) {
            printf("Aviso: tarefa '%s' terminou com codigo de saida %d.\n", nome, codigo);
        }
        return codigo;
    }

    if (WIFSIGNALED(status)) {
        int sinal = WTERMSIG(status);
        printf("Aviso: tarefa '%s' terminou pelo sinal %d.\n", nome, sinal);
        return 128 + sinal;
    }

    return -1;
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
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return -1;
    }

    return processar_status(t->nome, status);
}

static void run_sequential(char *nomes[], int n) {
    for (int i = 0; i < n; i++) {
        Task *t = buscar_task(nomes[i]);
        if (t == NULL) {
            printf("Erro: tarefa '%s' nao encontrada.\n", nomes[i]);
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
            printf("Erro: tarefa '%s' nao encontrada.\n", nomes[i]);
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
        if (waitpid(pids[i], &status, 0) < 0) {
            perror("waitpid");
            continue;
        }
        processar_status(tasks[i]->nome, status);
    }
}

static void fechar_todos_pipes(int pipes[][2], int quantidade) {
    for (int j = 0; j < quantidade; j++) {
        close(pipes[j][0]);
        close(pipes[j][1]);
    }
}

static void run_pipe(char *nomes[], int n) {
    if (n < 2) {
        printf("Erro: 'run pipe' requer ao menos 2 tarefas.\n");
        return;
    }
    if (n > MAX_LIST_TASKS) {
        printf("Erro: numero maximo de tarefas em pipe excedido.\n");
        return;
    }

    Task *tarefas[MAX_LIST_TASKS];
    for (int i = 0; i < n; i++) {
        tarefas[i] = buscar_task(nomes[i]);
        if (tarefas[i] == NULL) {
            printf("Erro: tarefa '%s' nao encontrada.\n", nomes[i]);
            return;
        }
    }

    int pipes[MAX_LIST_TASKS - 1][2];
    for (int i = 0; i < n - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            fechar_todos_pipes(pipes, i);
            return;
        }
    }

    pid_t pids[MAX_LIST_TASKS];
    int total_criados = 0;

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");

            fechar_todos_pipes(pipes, n - 1);

            for (int j = 0; j < total_criados; j++) {
                int status;
                waitpid(pids[j], &status, 0);
            }
            return;
        }

        if (pid == 0) {
            aplicar_workdir();

            if (i > 0) {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) < 0) {
                    perror("dup2 pipe stdin");
                    _exit(EXIT_FAILURE);
                }
            } else if (tarefas[i]->input_file[0] != '\0') {
                redirecionar_entrada(tarefas[i]->input_file);
            }

            if (i < n - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) < 0) {
                    perror("dup2 pipe stdout");
                    _exit(EXIT_FAILURE);
                }
            } else if (tarefas[i]->output_file[0] != '\0') {
                redirecionar_saida(tarefas[i]->output_file, tarefas[i]->append_mode);
            }

            fechar_todos_pipes(pipes, n - 1);

            execvp(tarefas[i]->programa, tarefas[i]->args);
            perror("execvp");
            _exit(EXIT_FAILURE);
        }

        pids[i] = pid;
        total_criados++;
    }

    fechar_todos_pipes(pipes, n - 1);

    for (int i = 0; i < total_criados; i++) {
        int status;
        if (waitpid(pids[i], &status, 0) < 0) {
            perror("waitpid");
            continue;
        }
        processar_status(tarefas[i]->nome, status);
    }
}

void comando_run(char *linha) {
    char *token = strtok(linha, " \t");
    token = strtok(NULL, " \t");

    if (token == NULL) {
        printf("Erro: comando 'run' precisa de um nome de tarefa ou modo.\n");
        return;
    }

    if (strcmp(token, "sequential") == 0 || strcmp(token, "parallel") == 0 || strcmp(token, "pipe") == 0) {
        char modo[16];
        strncpy(modo, token, sizeof(modo) - 1);
        modo[sizeof(modo) - 1] = '\0';

        char *nomes[MAX_LIST_TASKS];
        int n = 0;
        while ((token = strtok(NULL, " \t")) != NULL && n < MAX_LIST_TASKS) {
            nomes[n++] = token;
        }

        if (n == 0) {
            printf("Erro: 'run %s' precisa de pelo menos uma tarefa.\n", modo);
            return;
        }

        if (strcmp(modo, "sequential") == 0) run_sequential(nomes, n);
        else if (strcmp(modo, "parallel") == 0) run_parallel(nomes, n);
        else run_pipe(nomes, n);
        return;
    }

    Task *encontrada = buscar_task(token);
    if (encontrada == NULL) {
        printf("Erro: tarefa '%s' nao encontrada.\n", token);
        return;
    }
    executar_task(encontrada);
}
