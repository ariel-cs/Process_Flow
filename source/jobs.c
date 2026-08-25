#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "jobs.h"
#include "task.h"
#include "executor.h"
#include "processflow.h"

typedef struct {
    int id;
    pid_t pid;
    char nome_tarefa[64];
    int finalizado;
    int codigo_saida;
} Job;

static Job jobs[MAX_JOBS];
static int num_jobs = 0;

void comando_start(char *linha) {
    char *token = strtok(linha, " \t");
    token = strtok(NULL, " \t");

    if (token == NULL) {
        printf("Erro: comando 'start' precisa de uma tarefa.\n");
        return;
    }

    Task *t = buscar_task(token);
    if (t == NULL) {
        printf("Erro: tarefa '%s' nao encontrada.\n", token);
        return;
    }

    if (num_jobs >= MAX_JOBS) {
        printf("Erro: numero maximo de jobs atingido.\n");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        child_applyexec(t);
    }

    Job *j = &jobs[num_jobs];
    j->id = num_jobs + 1;
    j->pid = pid;
    strncpy(j->nome_tarefa, t->nome, sizeof(j->nome_tarefa) - 1);
    j->nome_tarefa[sizeof(j->nome_tarefa) - 1] = '\0';
    j->finalizado = 0;
    j->codigo_saida = 0;
    num_jobs++;

    printf("[%d] %d\n", j->id, j->pid);
}

static void atualizar_status(Job *j) {
    if (j->finalizado) return;

    int status;
    pid_t resultado = waitpid(j->pid, &status, WNOHANG);

    if (resultado < 0) {
        /* processo ja nao existe mais / foi coletado em outro lugar */
        perror("waitpid");
        j->finalizado = 1;
        return;
    }

    if (resultado == j->pid) {
        j->finalizado = 1;
        j->codigo_saida = processar_status(j->nome_tarefa, status);
    }
}

void comando_jobs(char *linha) {
    (void)linha;

    if (num_jobs == 0) {
        printf("Nenhum job iniciado.\n");
        return;
    }

    for (int i = 0; i < num_jobs; i++) {
        atualizar_status(&jobs[i]);
        if (jobs[i].finalizado) {
            printf("[%d] %d %s Concluido (codigo %d)\n",
                   jobs[i].id, jobs[i].pid, jobs[i].nome_tarefa, jobs[i].codigo_saida);
        } else {
            printf("[%d] %d %s Executando\n",
                   jobs[i].id, jobs[i].pid, jobs[i].nome_tarefa);
        }
    }
}

void comando_wait_job(char *linha) {
    char *token = strtok(linha, " \t");
    token = strtok(NULL, " \t");

    if (token == NULL) {
        printf("Erro: comando 'wait' precisa de um id de job.\n");
        return;
    }

    char *fim;
    long id = strtol(token, &fim, 10);

    if (*fim != '\0' || id <= 0 || id > MAX_JOBS) {
        printf("Erro: id de job invalido: '%s'.\n", token);
        return;
    }

    Job *alvo = NULL;
    for (int i = 0; i < num_jobs; i++) {
        if (jobs[i].id == (int)id) {
            alvo = &jobs[i];
            break;
        }
    }

    if (alvo == NULL) {
        printf("Erro: job %ld nao encontrado.\n", id);
        return;
    }

    if (alvo->finalizado) {
        printf("Job [%d] ja concluido (codigo %d).\n", alvo->id, alvo->codigo_saida);
        return;
    }

    int status;
    if (waitpid(alvo->pid, &status, 0) < 0) {
        perror("waitpid");
        return;
    }
    alvo->finalizado = 1;
    alvo->codigo_saida = processar_status(alvo->nome_tarefa, status);
    printf("Job [%d] concluido (codigo %d).\n", alvo->id, alvo->codigo_saida);
}
