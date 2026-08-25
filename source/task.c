#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "task.h"

#define DELIM " \t"

Task task[MAX_TASK];
int num_task = 0;

void comando_task(char *linha) {
    char *token = strtok(linha, DELIM);
    token = strtok(NULL, DELIM);
    if (token == NULL) {
        printf("Erro: comando 'task' precisa de um nome.\n");
        return;
    }
    if (num_task >= MAX_TASK) {
        printf("Erro: numero maximo de tarefas atingido.\n");
        return;
    }
    if (buscar_task(token) != NULL) {
        printf("Erro: ja existe uma tarefa com o nome '%s'.\n", token);
        return;
    }

    Task *t = &task[num_task];
    strncpy(t->nome, token, sizeof(t->nome) - 1);
    t->nome[sizeof(t->nome) - 1] = '\0';

    token = strtok(NULL, DELIM);
    if (token == NULL) {
        printf("Erro: comando 'task' requer um programa.\n");
        return;
    }
    strncpy(t->programa, token, sizeof(t->programa) - 1);
    t->programa[sizeof(t->programa) - 1] = '\0';

    t->num_args = 0;
    t->args[t->num_args++] = t->programa;

    while ((token = strtok(NULL, DELIM)) != NULL && t->num_args < MAX_ARGS - 1) {
        char *copia = malloc(strlen(token) + 1);
        if (copia == NULL) {
            printf("Erro: falha ao alocar memoria.\n");
            return;
        }
        strcpy(copia, token);
        t->args[t->num_args++] = copia;
    }
    t->args[t->num_args] = NULL;

    t->input_file[0] = '\0';
    t->output_file[0] = '\0';
    t->append_mode = 0;

    num_task++;
    printf("Tarefa '%s' cadastrada.\n",
           t->nome);
}

Task *buscar_task(const char *nome) {
    for (int i = 0; i < num_task; i++) {
        if (strcmp(task[i].nome, nome) == 0) {
            return &task[i];
        }
    }
    return NULL;
}

void liberar_tasks(void) {
    for (int i = 0; i < num_task; i++) {
        for (int j = 1; j < task[i].num_args; j++) {
            free(task[i].args[j]);
        }
    }
}
