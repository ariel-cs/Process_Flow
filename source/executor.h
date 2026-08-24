#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "processflow.h"

void child_applyexec(Task *t);
int executar_task(Task *t);
void comando_run(char *linha);

#endif
