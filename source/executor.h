#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "task.h"

void child_applyexec(Task *t);
int processar_status(const char *nome, int status);
int executar_task(Task *t);
void comando_run(char *linha);

#endif
