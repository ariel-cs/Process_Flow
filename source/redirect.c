#include <stdio.h>
#include <string.h>
#include "redirect.h"
#include "task.h"

static void definir_redirect(char *linha, const char *nome_comando, int is_output, int append) {
    char *token = strtok(linha, " \t");
    token = strtok(NULL, " \t");

    if (token == NULL) {
        printf("Erro: comando '%s' precisa do nome da tarefa.\n", nome_comando);
        return;
    }

    Task *t = buscar_task(token);
    if (t == NULL) {
        printf("Erro: tarefa '%s' nao encontrada.\n", token);
        return;
    }

    char *arquivo = strtok(NULL, " \t");
    if (arquivo == NULL) {
        printf("Erro: comando '%s' precisa do nome de arquivo.\n", nome_comando);
        return;
    }

    if (is_output) {
        strncpy(t->output_file, arquivo, sizeof(t->output_file) - 1);
        t->output_file[sizeof(t->output_file) - 1] = '\0';
        t->append_mode = append;
    } else {
        strncpy(t->input_file, arquivo, sizeof(t->input_file) - 1);
        t->input_file[sizeof(t->input_file) - 1] = '\0';
    }

    printf("Redirecionamento feito para tarefa '%s'.\n", t->nome);
}

void comando_input(char *linha)  { definir_redirect(linha, "input", 0, 0); }
void comando_output(char *linha) { definir_redirect(linha, "output", 1, 0); }
void comando_append(char *linha) { definir_redirect(linha, "append", 1, 1); }
