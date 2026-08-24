#ifndef PROCESSFLOW_H
#define PROCESSFLOW_H

#define MAX_TASK 64
#define MAX_ARGS 32

typedef struct {
    char nome[64];
    char programa[256];
    char *args[MAX_ARGS];
    int num_args;
} Task;

#endif
