#ifndef PROCESSFLOW_H
#define PROCESSFLOW_H

#define MAX_TASK 64
#define MAX_ARGS 32
#define MAX_LIST_TASKS 16
#define MAX_JOBS 64

typedef struct {
    char nome[64];
    char programa[256];
    char *args[MAX_ARGS];
    int num_args;
    char input_file[256];
    char output_file[256];
    int append_mode;
} Task;

#endif
