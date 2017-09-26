#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include <dirent.h>
#include <grp.h>
#include <sysexits.h>
#include <strings.h>
#include <ctype.h>
#define size 1024
#define t_size 100
#define TOK_DELIM " \t\a\n"
void pwd();
void printStylo(char * cwd);
void removeSubstring(char *s,const char *toremove);
void echo(char **args);
int cd(char **tokens);
char *readline();
int check(char *inp);
char **lineparser(char *com);
char **redirparser(char *com);
#define DELIM "><"
int pinfo(char **args);
#define del "\n"
#define PIPE "|"
#define delim "  \n\t"
void redir_out(char *filename,char *command);
void redir_in(char *filename,char *command);
void redir_app(char *filename,char *command);
int check1(char *inp);
char **tokenize(char *line, char * dlmtr);
void quit();
char **pipeparser(char *com);
void jobs();
typedef struct bg
{
  char name[1024];
  pid_t pid,pgid;
  struct bg *next;
}bg;
char **ARR;
int PID;

bg * background;
int shell;
pid_t shell_pgid;
void killallbg();
void kjob(char **tokens);
void delete(pid_t pid);
void fg(char **tokens);
int nightswatch(char **args);
void ls(char **args);
int exec(char **tokens);
