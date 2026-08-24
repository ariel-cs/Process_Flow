#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "task.h"

Task task[MAX_TASK];
int num_task = 0;

void comando_task(char *linha) {
    char *token = strtok(linha, " ");
    token = strtok(NULL, " ");
    if (token == NULL) {
        printf("Erro: task ...\n");
        return;
    }
    if (num_task >= MAX_TASK) {
        printf("Erro: numero max de task.\n");
        return;
    }
    Task *t = &task[num_task];
    strncpy(t->nome, token, sizeof(t->nome) - 1);
    t->nome[sizeof(t->nome) - 1] = '\0';

    token = strtok(NULL, " ");
    if (token == NULL) {
        printf("Erro: comando task pede um programa.\n");
        return;
    }
    strncpy(t->programa, token, sizeof(t->programa) - 1);
    t->programa[sizeof(t->programa) - 1] = '\0';

    t->num_args = 0;
    t->args[t->num_args++] = t->programa;

    while ((token = strtok(NULL, " ")) != NULL && t->num_args < MAX_ARGS - 1) {
        char *copia = malloc(strlen(token) + 1);
        if(copia == NULL){
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
    printf("Tarefa '%s' cadastrada (programa: %s, %d argumentos).\n",
           t->nome, t->programa, t->num_args - 1);
}

Task *buscar_task(const char *nome) {
    for (int i = 0; i < num_task; i++) {
        if (strcmp(task[i].nome, nome) == 0) {
            return &task[i];
        }
    }
    return NULL;
}
