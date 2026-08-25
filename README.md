# Process Flow

## Descrição

O **Process Flow** é um orquestrador de processos desenvolvido em C que utiliza `fork()`, `execvp()`, `pipe()`, `dup2()` e `waitpid()` para executar e gerenciar tarefas.

## Funcionalidades

* Cadastro e execução de tarefas.
* Execução sequencial, paralela e com pipe.
* Execução em background com `start`, `jobs` e `wait`.
* Redirecionamento de entrada, saída e append.
* Alteração do diretório de trabalho com `workdir`.
* Suporte aos modos interativo e workflow.

## Estrutura

* `main.c`: entrada e controle do programa.
* `task.c`: cadastro e gerenciamento de tarefas.
* `executor.c`: execução dos processos.
* `redirect.c`: redirecionamentos de I/O.
* `jobs.c`: gerenciamento dos processos em background.

## Compilação

```bash
make
./processflow
```

Para limpar os arquivos gerados:

```bash
make clean
```
