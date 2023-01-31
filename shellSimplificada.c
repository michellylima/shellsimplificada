// Shell Simplificada
// Universidade Federal de Goiás - Sistemas Operacionais
// Código por Michelly Lima (201802780), Beatriz Guimarães (202000508) e Virgínia Fernandes (201700281)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAXCHAR 1000
#define MAXCOMANDOS 100
#define clear() printf("\033[H\033[J")

void start()
{
  clear();
  printf("\n\n\n\t-----SHELL-----");

  char *usuario = getenv("USER");

  printf("\n\n\nUsuario: @%s", usuario);
  printf("\n");

  sleep(1);
  clear();
}

// Function to print Current Directory.
void diretorioAtual()
{
  char diretorio[1024];
  getcwd(diretorio, sizeof(diretorio));
  printf("\nDir: %s", diretorio);
}

int entrada(char *str)
{
  char *buffer;

  buffer = readline("\n>>> ");

  if (strlen(buffer) != 0)
  {
    add_history(buffer);
    strcpy(str, buffer);
    return 0;
  }

  else
  {
    return 1;
  }
}

int processString(char *str, char **parsed, char **parsedpipe)
{

  char *strpiped[2];
  int piped = 0;

  piped = parsePipe(str, strpiped);

  if (piped)
  {
    parseSpace(strpiped[0], parsed);
    parseSpace(strpiped[1], parsedpipe);
  }
  else
  {

    parseSpace(str, parsed);
  }

  if (comandosBuiltin(parsed))
    return 0;
  else
    return 1 + piped;
}

void help()
{
  printf(
      "\n---- HELP ----"
      "\nLista de comandos:"
      "\n>mkdir"
      "\n>cd"
      "\n>ls"
      "\n>exit"
      "\n>pipe"
      "\n>comandos gerais da Shell UNIX"
      "\n>improper space handling");

  return;
}

int comandosBuiltin(char **parsed)
{
  int qtdComandos = 4, i, opcao = 0;
  char *listaDeComandos[qtdComandos];
  char *usuario;

  listaDeComandos[0] = "hello";
  listaDeComandos[1] = "cd";
  listaDeComandos[2] = "help";
  listaDeComandos[3] = "exit";

  for (i = 0; i < qtdComandos; i++)
  {
    if (strcmp(parsed[0], listaDeComandos[i]) == 0)
    {
      opcao = i + 1;
      break;
    }
  }

  switch (opcao)
  {
  case 1:
    usuario = getenv("USER");
    printf("\nOla, %s!\nSeja bem-vindo!"
           "\nUse o comando help para mais informacoes.\n",
           usuario);
    return 1;

  case 2:
    chdir(parsed[1]);
    return 1;

  case 3:
    help();
    return 1;

  case 4:
    printf("\nGoodbye\n");
    exit(0);

  default:
    break;
  }

  return 0;
}

// Executar os comandos
void execArgs(char **parsed)
{
  pid_t pid = fork();

  if (pid == -1)
  {
    printf("\nErro no fork!");
    return;
  }

  else if (pid == 0)
  {
    if (execvp(parsed[0], parsed) < 0)
    {
      printf("\nComando nao pode ser executado!");
    }
    exit(0);
  }

  else
  {
    wait(NULL);
    return;
  }
}

void execArgsPiped(char **parsed, char **parsedpipe)
{
  int pipefd[2];
  // 0 = leitura do final do pipe
  // 1 = escrita do final do pipe

  pid_t p1, p2;

  if (pipe(pipefd) < 0)
  {
    printf("\nPipe não inicializado!");
    return;
  }

  p1 = fork();

  if (p1 < 0)
  {
    printf("\nErro no fork!");
    return;
  }

  if (p1 == 0)
  {
    // Filho 1
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);

    if (execvp(parsed[0], parsed) < 0)
    {
      printf("\nErro no comando 1!");
      exit(0);
    }
  }

  else
  {
    // Pai
    p2 = fork();

    if (p2 < 0)
    {
      printf("\nErro no fork!");
      return;
    }

    // Filho 2
    if (p2 == 0)
    {
      close(pipefd[1]);
      dup2(pipefd[0], STDIN_FILENO);
      close(pipefd[0]);
      if (execvp(parsedpipe[0], parsedpipe) < 0)
      {
        printf("\nErro no comando 2!");
        exit(0);
      }
    }

    else
    {
      wait(NULL);
      wait(NULL);
    }
  }
}

// function for finding pipe
int parsePipe(char *str, char **strpiped)
{
  int i;

  for (i = 0; i < 2; i++)
  {
    strpiped[i] = strsep(&str, "|");
    if (strpiped[i] == NULL)
    {
      break;
    }
  }

  if (strpiped[1] == NULL)
  {
    // nenhum pipe foi encontrado
    return 0;
  }

  else
  {
    return 1;
  }
}

// function for parsing command words
void parseSpace(char *str, char **parsed)
{
  int i;

  for (i = 0; i < MAXCOMANDOS; i++)
  {
    parsed[i] = strsep(&str, " ");

    if (parsed[i] == NULL)
    {
      break;
    }

    if (strlen(parsed[i]) == 0)
    {
      i--;
    }
  }
}

int main()
{
  int retorno = 0;
  // retorna 0 em comandos builtin ou se não tiver comando,
  // 1 para comandos simples e
  // 2 para comandos piped

  char comando[MAXCHAR];
  char *parsedArgs[MAXCOMANDOS];
  char *parsedArgsPiped[MAXCOMANDOS];

  start();

  while (1)
  {
    diretorioAtual();

    if (entrada(comando))
    {
      continue;
    }

    retorno = processString(comando, parsedArgs, parsedArgsPiped);

    if (retorno == 1) // comando simples
    {
      execArgs(parsedArgs);
    }

    if (retorno == 2) // comandos piped
    {
      execArgsPiped(parsedArgs, parsedArgsPiped);
    }
  }

  return 0;
}