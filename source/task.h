#ifndef TASK_H
#define TASK_H

#include "processflow.h"

extern Task task[MAX_TASK];
extern int num_task;

void comando_task(char *linha);
Task *buscar_task(const char *nome);

#endif
