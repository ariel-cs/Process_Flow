#include <stdio.h>
#include <string.h>
#include "processflow.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

Task task[MAX_TASK];
int num_task = 0;

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

    Task *encontrada = NULL;
    for (int i = 0; i < num_task; i++) {
        if (strcmp(task[i].nome, token) == 0) {
            encontrada = &task[i];
            break;
        }
    }

    if (encontrada == NULL) {
        printf("Erro: tarefa '%s' não encontrada.\n", token);
        return;
    }

    executar_task(encontrada);
}

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
        else if (check_comando(linha, "run")) {
            comando_run(linha);
        }
        else {
            printf("Comando desconhecido: '%s'\n",linha);
        }

    }


    return 0;
}
