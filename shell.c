#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <util.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <dirent.h>
#include <grp.h>
#include <sysexits.h>
#include <strings.h>
#include <ctype.h>

#define buff_size 1024

char homeDir[buff_size],hostname[buff_size];

char *username;

void printStylo(char * cwd)
{
  char s=' ';
  if(strstr(cwd,homeDir)!=NULL)
  {
    removeSubstring(cwd,homeDir);
    s='~';
  }
  fprintf(stdout, "\033[33;1m%s@%s:\033[0m\e[32m%c%s>\033[0m ", username, hostname,s, cwd);
}
//------------------------------------------------------------------------------------------------------------------------
//signal handling functions
void handle_signal(int signo)
{
  puts("");
  char cwd[buff_size];
  getcwd(cwd, buff_size);
  printStylo(cwd);
  fflush(stdout);
}
int gpid=-100;
void sig_handler(int signo)
{
  if(signo==SIGTSTP)
    if(gpid!=-100)
    {
      kill(gpid,SIGTSTP);
      gpid=-900;
    }

  }
//signal handling works
//------------------------------------------------------------------------------------------------------------------------


  void insert(char *process,pid_t pid)
  {
    bg *new=(bg*)malloc(sizeof(bg));
    strcpy(new->name,process);
    new->pid=new->pgid=pid;
    new->next=NULL;
    if(background==NULL)
      background=new;
    else
    {
      bg *temp=background;
      while(temp->next!=NULL)
        temp=temp->next;
      temp->next=new; 
    }
  }

  void delete(pid_t pid)
  {
    if(background!=NULL)
    {
      bg *temp=background;
      if(background->pid==pid)
      {
        background=background->next;
        free(temp);
      } 
      else
      {
        bg *temp2;
        while(temp!=NULL && temp->pid!=pid)
        {
          temp2=temp;
          temp=temp->next;
        }
        if(temp!=NULL)
        {
          temp2->next=temp->next;
          free(temp);
        }

      }
    }
  }

  void bgResponse(int signal)
  {
    pid_t pid;
    int status;
    pid_t processid;
    while((processid=waitpid(-1,&status,WNOHANG))>0)
    {
      if(processid!=-1 && processid!=0)
      {
        bg *temp=background;
        while(temp!=NULL && temp->pid!=processid)
          temp=temp->next;   
        if(WIFEXITED(status))
        {
          if(temp!=NULL)
          {
            fprintf(stderr,"%s with pid %d exited normally\n",temp->name,processid);
            delete(processid); 
          }
        }
      }
    }
  }

  void procSTOP(int signal) 
  {
    int status=0;
    printf("\nProcess %d has been stopped and sent to bg\n", PID);
    pid_t pid=PID;
    insert(ARR[0],PID); 
    if(kill(pid, SIGSTOP) < 0)
      perror("Error in putting it the process to bg : ");
  }
  void exe(char **args)
  {
    int te=-1;
    int status = 0,i;
    PID = fork();
    bg *temp;
    int backg = 0;
    i=0;
    ARR=args;
    signal(SIGTSTP,procSTOP);
    while(args[i]!=NULL)
     i++;
   if(i!=1)
   {
     if(strcmp(args[i-1], "&") == 0)
     {
      backg = 1;
      args[i-1] = NULL;
    }
  }
  if (PID == 0)
  {
   execvp(*args,args);
   signal(SIGTSTP,procSTOP);
   perror(*args);
   exit(1);
 }
 else
 {
  if (backg)
  {
    printf("starting background job %d\n", PID);
    insert(args[0],PID);
    signal(SIGCHLD,bgResponse);
  }
  else
    while(wait(&status)!=PID);
  if (status != 0)
    fprintf  (stderr, "error: %s exited with status code %d\n", args[0], status);
}
}

int exec(char **tokens)
{
  if(strcmp(tokens[0],"cd")==0)
  {
    cd(tokens);
    return 1;
  }
  else if(strcmp(tokens[0],"pwd")==0)
  {
    pwd();
    printf("\n");
    return 1;
  }
  else if(strcmp(tokens[0],"ls")==0)
  {
    ls(tokens);
    return 1;
  }
  else if(strcmp(tokens[0],"nightswatch")==0)
  {
    nightswatch(tokens);
    return 1;
  }
  else if(strcmp(tokens[0],"echo")==0)
  {
    echo(tokens);
    return 1;
  }
  else if(strcmp(tokens[0],"pinfo")==0)
  {
    pinfo(tokens);
    return 1;
  }
  else if(strcmp(tokens[0],"quit")==0)
  {
    quit();
    return 1;
  }
  else if(strcmp(tokens[0],"jobs")==0)
  {
    jobs();
    return 1;
  }
  else if(strcmp(tokens[0],"killallbg")==0)
  {
    killallbg();
    return 1;
  }
  else if(strcmp(tokens[0],"kjob")==0)
  {
    kjob(tokens);
    return 1;
  }
  else if(strcmp(tokens[0],"fg")==0)
  {
    fg(tokens);
    return 1;
  }
  else
  {
    exe(tokens);
    return 1;
  }
  return 0;
}
void redir_out(char *token,char *command)
{
  int status=0,pid,r;
  char **k,*filename;
  filename = strtok(token,TOK_DELIM);
  FILE *test=fopen(filename, "w");
  int fd=fileno(test); 
  k=lineparser(command);
  pid=fork(); 
  if (pid == 0) 
  { 
    dup2(fd,1);
    r=exec(k); 
    exit(0);
  }
  else
    while(wait(&status)!=pid);

}
void redir_in(char *token,char *command)
{
  int status=0,pid,r;
  char **k,*filename;
  filename = strtok(token,TOK_DELIM);
  FILE *test=fopen(filename, "r");
  int fd=fileno(test); 
  k=lineparser(command);
  pid=fork(); 
  if (pid == 0) 
  { 
    dup2(fd,0);
    r=exec(k); 
    exit(0);
  }
  else
    while(wait(&status)!=pid);

}
void redir_app(char *token,char *command)
{
  int status=0,pid,r;
  char **k,*filename;
  filename = strtok(token,TOK_DELIM);
  FILE *test=fopen(filename, "a");
  int fd=fileno(test); 
  k=lineparser(command);
  pid=fork(); 
  if (pid == 0) 
  { 
    dup2(fd,1);
    r=exec(k); 
    exit(0);
  }
  else
    while(wait(&status)!=pid);

}

void redir_ior(char *token,char *command,char *token2)
{
  int status=0,pid,r;
  char **k,*filename,*file2;
  filename = strtok(token,TOK_DELIM);
  file2 = strtok(token2,TOK_DELIM);
  FILE *test=fopen(filename, "r");
  FILE *test1=fopen(file2, "w");
  int fd=fileno(test); 
  int fd1=fileno(test1);
  k=lineparser(command);
  pid=fork(); 
  if (pid == 0) 
  { 
    dup2(fd,0);
    dup2(fd1,1);
    r=exec(k);
    exit(0);
  }
  else
    while(wait(&status)!=pid);
}
void redir(int flag,char **l)
{
  if(flag==0)
  {
    redir_out(l[1],l[0]);

  }
  else if(flag==1)
  {
    redir_in(l[1],l[0]);
  }
  else if(flag==2)
  {
    redir_app(l[1],l[0]);
  }
  else if(flag==3)
  {
    redir_ior(l[1],l[0],l[2]);
  }
}

int pipeCommand(char **args) 
{
  int status=0,numOfPipes=-1,flag=-1,len;
  pid_t pid, wpid;
  char **k,**l;
  int i, j, total,r;
  for(i=0;args[i]!=NULL;i++)
  {
    numOfPipes++;
  }
  int pipes[2*numOfPipes];
  for(i=0;i<(2*numOfPipes);i+=2) 
  {
    pipe(pipes + i);
  }
  for(i=-2;i<(2*numOfPipes);i+=2) 
  {
    if ((pid = fork()) == 0) 
    {
      if((i+3)<(2*numOfPipes))
        dup2(pipes[i+3], 1);
      if(i!=-2)
        dup2(pipes[i], 0);
      for(j=0;j<(2*numOfPipes);j++)
        close(pipes[j]);
      r=i/2+1;
      len=check1(args[(i+2)/2]);
      if(len==2)
      {
       flag=check(args[(i+2)/2]);
       if(flag!=2)
        flag=3;
    }
    else
      flag=check(args[(i+2)/2]);

    l=redirparser(args[(i+2)/2]);
    if(l[1]!='\0')
    { 
     redir(flag,l);
     r=1;
   }
   else
   {
    k=lineparser(args[(i+2)/2]);
    r=exec(k);
  }
  exit(EXIT_FAILURE);
}
}
for(i=0;i<(2*numOfPipes);i++)
  close(pipes[i]);
while ((wpid = wait(&status)) > 0);
return 0;
}
void shell_loop()
{
  char *line;
  char **commands;
  int status=0,flag,run=1;

  char home[]="HOME:";
  if(gethostname(hostname, sizeof(hostname)) == -1)
    perror("gethostname() error\n");

  char cwd[buff_size];

  getcwd(cwd, buff_size);

  putenv(home);
  strcpy(homeDir,cwd);

  struct passwd *pass;
  pass = getpwuid(getuid());
  username = pass->pw_name;
  char *a;
  int res,r,c,i,len,length;
  char *inp_com,**k,**l,*inp,**p;
  do
  {
    getcwd(cwd, buff_size);
    printStylo(cwd);
    inp_com=readline();
    len=check1(inp_com);
    if(len==2)
    {
      flag=check(inp_com);
      if(flag!=2)
        flag=3;
    }
    else
      flag=check(inp_com);
    if(inp_com[0]=='\0')
      continue; 
    p=pipeparser(inp_com);
    if(p[1]=='\0')
    { 
      l=redirparser(inp_com);
      if(l[1]=='\0')
      {
        k=lineparser(inp_com);
        r=exec(k);
      }
      else
      {
        redir(flag,l);
        r=1;
      }
    }
    else
    {
      r=pipeCommand(p);
      r=1;
    }
  }while(r);
}

int main(int argc,char *argv[])
{
  signal(SIGINT, SIG_IGN);
  signal(SIGINT, handle_signal);
  signal(SIGTSTP,sig_handler);
    // sets home environment
  char homey[1024];
  getcwd(homey,1024);
  setenv("HOME",homey,1);
  shell_loop();
  return 0;
}
