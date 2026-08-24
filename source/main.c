#include <stdio.h>
#include <string.h>
#include "task.h"
#include "executor.h"

int check_comando(const char *linha, const char *comando) {
    size_t len = strlen(comando);
    if (strncmp(linha, comando, len) != 0) {
        return 0;
    }
    return linha[len] == '\0' || linha[len] == ' ';
}

int main(int argc, char *argv[]) {
    char linha[1024];

    (void)argc;
    (void)argv;

    while (1) {
        printf("processflow> ");
        fflush(stdout);

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            printf("\n");
            break;
        }

        linha[strcspn(linha, "\n")] = '\0';

        if (strlen(linha) == 0) {
            continue;
        }

        if (strcmp(linha, "exit") == 0) {
            break;
        }

        if (check_comando(linha, "task")) {
            comando_task(linha);
        } else if (check_comando(linha, "run")) {
            comando_run(linha);
        } else {
            printf("Comando desconhecido: '%s'\n", linha);
        }
    }

    printf("ProcessFlow encerrado.\n");
    return 0;
}
