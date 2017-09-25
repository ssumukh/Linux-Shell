#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <makefile.h>
char *readline()
{
  int k=size;
  char *command=malloc(sizeof(char)*size),ch;
  int pos=0;
  while(1)
  {
    ch=getchar();
    if(ch==EOF)
    {
      printf("\n");
      exit(0);
    }
    if(ch=='\n')
    {
      command[pos++]='\0';
      return command;
    }
    else
    {
      command[pos++]=ch;
    }
  }
  if(pos>=k)
  {
    k+=size;
    command=realloc(command,k);
  }
}
int check1(char *inp)
{
  int len=0,i;
  for(i=0;i<strlen(inp);i++)
  {
    if(inp[i]=='>'||inp[i]=='<')
      len++;
  }
  return len;
}
int check(char *inp)
{
  int i,flag=-1;
  for(i=0;i<strlen(inp);i++)
      {
        if(inp[i]=='>')
         {
          if(inp[i+1]=='>')
          { 
             flag=2;
             break;
           }
          else
          {  
            flag=0;
            break;
          }
         }
        else if(inp[i]=='<')
        {
          flag=1;
          break;
        }
      }
      return flag;
}

char **redirparser(char *com)
{
  int tok_size=t_size;
  char **tokens=malloc(sizeof(char *)*tok_size);
  char *token;
  token = strtok(com,DELIM);
  int position=0;
  while (token != NULL)
  {
    tokens[position++] = token;
    if(position>=tok_size)
    {
      tok_size+=t_size;
      tokens=realloc(tokens,tok_size*sizeof(char *));
    }
    token=strtok(NULL,DELIM);
  }
  return tokens;
}

char **pipeparser(char *com)
{
  int tok_size=t_size;
  char **tokens=malloc(sizeof(char *)*tok_size);
  char *token;
  token = strtok(com,PIPE);
  int position=0;
  while (token != NULL)
  {
    tokens[position++] = token;
    if(position>=tok_size)
    {
      tok_size+=t_size;
      tokens=realloc(tokens,tok_size*sizeof(char *));
    }
    token=strtok(NULL,PIPE);
  }
  return tokens;
}

char **lineparser(char *com)
{
  int tok_size=t_size;
  char **tokens=malloc(sizeof(char *)*tok_size);
  char *token;
  token = strtok(com,TOK_DELIM);
  int position=0;
  while (token != NULL)
  {
    tokens[position++] = token;
    if(position>=tok_size)
    {
      tok_size+=t_size;
      tokens=realloc(tokens,tok_size*sizeof(char *));
    }
    token=strtok(NULL,TOK_DELIM);
  }
  return tokens;
}
