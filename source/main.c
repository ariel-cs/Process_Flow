#include <stdio.h>
#include <string.h>
#include "processflow.h"

Task task[MAX_TASK];
int num_task = 0;

int check_comando(const char *linha, const char *comando) {
    size_t len = strlen(comando);
    if (strncmp(linha, comando, len) != 0) {
        return 0;
    }
    return linha[len] == '\0' || linha[len] == ' ';
}

void comando_task(char *linha){
    char *token = strtok(linha," ");
    token = strtok(NULL, " ");
    if (token == NULL) {
        printf("Erro: task ...\n");
        return;
    }

    if (num_task>= MAX_TASK) {
        printf("Erro: numero max de task.\n");
        return;
    }

    Task *t = &task[num_task];
    strncpy(t->nome, token, sizeof(t->nome) - 1);
    t->nome[sizeof(t->nome) - 1] = '\0';

    token = strtok(NULL, " ");
    if(token == NULL){
        printf("Erro: comando task pede um programa.\n");
        return;
    }

    strncpy(t->programa, token, sizeof(t->programa) - 1);
    t->programa[sizeof(t->programa) - 1] = '\0';

    t->num_args = 0;
    t->args[t->num_args++] = t->programa;

    while ((token = strtok(NULL, " ")) != NULL && t->num_args < MAX_ARGS - 1) {
        t->args[t->num_args++] = token;
    }
    t->args[t->num_args] = NULL;

    num_task++;
    printf("Tarefa '%s' cadastrada (programa: %s, %d argumentos).\n",
        t->nome, t->programa, t->num_args - 1);
}

int main(int argc, char *argv[]){
    char linha[1024];


    while (1) {
        printf("processflow> ");
        fflush(stdout);

        if (fgets(linha, sizeof(linha), stdin) == NULL){
            printf("\n");
            break;
        }

        linha[strcspn(linha, "\n")] = '\0';

        if (strlen(linha) == 0){
            continue;
        }

        if (strcmp(linha, "exit") == 0){
            break;
        }

        if (check_comando(linha, "task")) {
            comando_task(linha);
        }
        else {
            printf("Comando desconhecido: '%s'\n",linha);
        }

    }


    return 0;
}
