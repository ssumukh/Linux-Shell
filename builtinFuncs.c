#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <util.h>
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
#include <dirent.h>
#include <getopt.h>
#include <time.h>
//---------------------------------------------------------------------------------------------------------------
//a utility function to remove substring
void removeSubstring(char *s,const char *toremove)
{
  while( s=strstr(s,toremove) )
    memmove(s,s+strlen(toremove),1+strlen(s+strlen(toremove)));
}
//---------------------------------------------------------------------------------------------------------------
// other support functions
int numbers_only(const char *s)
{
    while (*s) {
        if (isdigit(*s++) == 0) return 0;
    }

    return 1;
}

int get_argc(char ** argv) {
   int c;
   for(c=0;argv[c]!=NULL;c++);

   return c;
}
char **tokenize(char *line, char * dlmtr)
{
  int bufsize = 64, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;
  if (!tokens) {
    fprintf(stderr, "Error: allocation error\n");
    exit(EXIT_FAILURE);
  }
  token = strtok(line, "\n\t\r\a ");
  while (token != NULL) {
    tokens[position] = token;
    position++;
    if (position >= bufsize) {
      bufsize += 64;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "Error: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }token = strtok(NULL, " \n\t\r\a");
  }
  tokens[position] = NULL;
  return tokens;
}

//---------------------------------------------------------------------------------------------------------------
//pinfo funcs
// print the properpath of the exec file, by replacing home directory with ~ symbol>>>>>utility for pinfo
void properPath(char * exe)
{
    char s=' ';
    //get present directory
    char *home = getenv("HOME");
    //remove home directory if the process is in home already
    if(strstr(exe,home)!=NULL)
    {
      removeSubstring(exe,home);
      s='~';
    }
    printf("Executable path: %c%s\n",s,exe );
}

// get executable file's location
int get_exctbl(int pid){

    char exe[1024];
    int ret;

    struct passwd *passwdEnt = getpwuid(getuid());
    char *home = passwdEnt->pw_dir;

    char c[1024];
    sprintf(c,"/proc/%d/exe",pid);

    ret = readlink(c,exe,sizeof(exe)-1);
    if(ret ==-1) {
        fprintf(stderr,"ERROR\n");
    }
    else{
        exe[ret] = 0;
        properPath(exe);
    }
}

// parsing lines input
int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

//print size of virtual memory of the process in KB
int getValue(int pid){
    char c[20];
    sprintf(c,"/proc/%d/status",pid);
    FILE* file = fopen(c, "r");
    int result = -1;
    char line[128];
    //finding the place at the which virtual memory sixe is stored in the file
    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    printf("Virtual memory: %d KB\n",result);
    return result;
}

//get the state in which the process of given pid is in
void get_state(int pid){
    char c[20];
    sprintf(c,"/proc/%d/stat",pid);
    FILE* fp;
    char  line[1024];
    char* val;
    fp = fopen(c , "r");
    const char s[2] = " ";
    if (fgets(line, sizeof(line), fp) != NULL)
    {
        val = strtok(line, s);
        val = strtok(NULL,s);
        val = strtok(NULL,s);//go to the third token of the file /proc/<pid>/stat, where the status of the process is stored
        printf("Process state: %s\n",val);
    }
    fclose(fp);
}

int pinfo(char **args){
    int pid;
    //check if there are any arguments. if not, apid = pid of current process
    if(args[1] == NULL){
        pid = getpid();
    }
    else{
        pid = atoi(args[1]);
    }
    //check if proces exists
    if (getpgid(pid)<0) {
        printf("Process does not exit\n");
    }
    else{
        printf("PID: %d\n",pid);
        get_state(pid);
        getValue(pid);
        get_exctbl(pid);
    }

}
//---------------------------------------------------------------------------------------------------------------
//ls functions
struct Options
{
    bool optn_a;
    bool optn_l;
};

static void init_opts(struct Options* opts)
{
    opts->optn_a = false;
    opts->optn_l = false;
}

void print_opts(struct Options* opts)
{

  printf("a:%d\n",opts->optn_a);
  printf("l:%d\n",opts->optn_l);
}



struct Options get_opts(int count, char** args)
{
    struct Options opts;
    init_opts(&opts);
    int opt;
    if(count > 1)
    {
      int l  = strlen(args[1]);
      int i;
      char c=' ';
      for (i=1;i<l;i++){
        c = *(args[1]+i*sizeof(char));
        switch (c)
          {
              case 'a': opts.optn_a = true; break;
              case 'l': opts.optn_l = true; break;
              default: printf("INVALID OPTION '%c' detected\n",c);
          }
      }
    }
    return opts;
}


void print_name_or_link(const char* filename,  mode_t mode)
{
    if (mode & S_IFLNK)
    {
        char link_buf[512];
        int count = readlink(filename, link_buf, sizeof(link_buf));

        if (count >= 0)
        {
            link_buf[count] = '\0';
      printf(" %s -> %s \n", filename, link_buf);
            return;
        }
    }
    // check if quotes are required when displaying
    
    printf(" %s", filename);
    
    putchar('\n');
}

static void print_permissions(mode_t mode)
{
    putchar((mode & S_IRUSR) ? 'r' : '-');
    putchar((mode & S_IWUSR) ? 'w' : '-');
    putchar((mode & S_IXUSR) ? 'x' : '-');
    putchar((mode & S_IRGRP) ? 'r' : '-');
    putchar((mode & S_IWGRP) ? 'w' : '-');
    putchar((mode & S_IXGRP) ? 'x' : '-');
    putchar((mode & S_IROTH) ? 'r' : '-');
    putchar((mode & S_IWOTH) ? 'w' : '-');
    putchar((mode & S_IXOTH) ? 'x' : '-');
}

static void print_filetype(mode_t mode)
{
    switch (mode & S_IFMT)
    {
        case S_IFREG: putchar('-'); break;
        case S_IFDIR: putchar('d'); break;
        case S_IFLNK: putchar('l'); break;
        case S_IFCHR: putchar('c'); break;
        case S_IFBLK: putchar('b'); break;
        case S_IFSOCK: putchar('s'); break;
        case S_IFIFO: putchar('f'); break;
    }
}

void print_time(time_t mod_time)
{
    // get current time with year
    time_t curr_time;
    time(&curr_time);
    struct tm* t = localtime(&curr_time);
    const int curr_mon = t->tm_mon;
    const int curr_yr = 1970 + t->tm_year;

    // get mod time and year
    t = localtime(&mod_time);
    const int mod_mon = t->tm_mon;
    const int mod_yr = 1970 + t->tm_year;

    // determine format based on years
    const char* format = ((mod_yr == curr_yr)
                       && (mod_mon >= (curr_mon - 6)))
                           ? "%b %e %H:%M"
                           : "%b %e  %Y";

    char time_buf[128];
    strftime(time_buf, sizeof(time_buf), format, t);
    printf("%s", time_buf);
}

struct stat get_stats(char * dir,const char* filename)
{
    char path[1024];
    sprintf(path, "%s/%s", dir, filename);
    struct stat sb;

    if (lstat(path, &sb) < 0)
    {   
        perror(path);
        exit(EX_IOERR);
    }

    return sb;
}

void display_stats(char* dir, char* filename)
{

   // global_dir = dir;
    struct stat sb = get_stats(dir,filename);
    print_filetype(sb.st_mode);
    print_permissions(sb.st_mode);
    printf(" %d ", (int)sb.st_nlink);
    printf("%10s ", getpwuid(sb.st_uid)->pw_name);
    printf("%10s", getgrgid(sb.st_gid)->gr_name);
    printf("%10ld ", (long)sb.st_size);
    print_time(sb.st_mtime);
    print_name_or_link(filename, sb.st_mode);
}


void recurse_dirs(char* dir, struct Options opts)
{
    DIR* dfd = opendir(dir);
    struct dirent* dp = readdir(dfd);

   // printf("\n%s:\n", dir);

    while ((dp = readdir(dfd)))
    {
        const bool omit_hidden = !opts.optn_a && dp->d_name[0] == '.';

        if (!omit_hidden)
        {
            if (opts.optn_l)
            {
                display_stats(dir, dp->d_name);
            }
            else
            {
                printf("%s\n", dp->d_name);
            }
        }
    }

    closedir(dfd);
}

void ls(char **argv)
{
  //printf("count:%d\n",get_argc(argv));
  int argc=get_argc(argv);
  //printf("count:%d\n",argc);
  struct Options opts;
  opts=get_opts(argc,argv);
  //print_opts(&opts);
  recurse_dirs(".", opts);
  //scan_dirs(argc, argv, get_opts(argc, argv));

}
//---------------------------------------------------------------------------------------------------------------
int nightswatch(char **args){

  int run=1;
  char *line;
  char **commands;
  int argc=get_argc(args);
  if(argc > 3)
  {
    if(strcmp(args[1],"-n") == 0 && numbers_only(args[2])){
      if(strcmp(args[3],"interrupt") == 0){
        char cmnd[1000]="watch -n ";
        //p(cmnd);
        strcat(cmnd,args[2]);
        //p(cmnd);
        strcat(cmnd," cat /proc/interrupts");
        //p(cmnd);
        commands=tokenize(cmnd," \n\t\r\a");
          run=exec(commands);
      }
      else if(strcmp(args[3],"dirty") == 0){
        char cmnd[1000]="watch -n ";
        strcat(cmnd,args[2]);
        strcat(cmnd," grep ^Dirty /proc/meminfo");
        commands=tokenize(cmnd,"\n\t\r\a ");
          run=exec(commands);
      }
      else{
        printf("INVALID COMMAND\nThe men of wall refuse to do the work for people of the south\nJon+Dany=LOVE\n");
      }
    }
    else
    {
      printf("INVALID COMMAND\nThe men of wall refuse to do the work for people of the south\nJon+Dany=LOVE\n");  
    }
  }
  else
  {
    printf("INVALID COMMAND\nThe men of wall refuse to do the work for people of the south\nJon+Dany=LOVE\n");  
  }
  return run;

}
//----------------------------------------------------------------------------------------------------------------------

void quit()
{
  exit(0);
}
struct jobs
{
  char name[80];
  int id;
};
//-----------------------------------------------------------------------------------------------------------------

void pwd()
{
  char result[1024],*token;
  getcwd(result,sizeof(result));
	int pos=0,count=0;
  register struct passwd *pw;
  register uid_t uid;
  uid = geteuid();
  pw = getpwuid(uid);
  token=strtok(result+1,delim);
  while(token!=NULL)
  {
    if(strcmp(token,"home")==0)
    {
      token=strtok(NULL,delim);
      if(strcmp(token,pw->pw_name)==0)
         printf("~");
    }
    else
      printf("/%s",token);
    token=strtok(NULL,delim);
  }
}
//-----------------------------------------------------------------------------------------------------------------

void echo(char **args)
{
  int i=1,j;
  while(args[i]!=NULL)
  {
    for(j=0;j<strlen(args[i]);j++)
    {
      if((args[i][j]!='\"'&&args[i][j]!='\''))
        printf("%c",args[i][j]);
    }
    i++;
  }
  printf("\n");
}

//-----------------------------------------------------------------------------------------------------------------

int cd(char **args)
{
  if ((args[1] == NULL) || (strcmp(args[1],"~") == 0)) {
    chdir(getenv("HOME")); 
    return 1;
  }
  else {
    if (chdir(args[1]) != 0) {
      perror("Could not change directory");
    }
  }
  return 1;
}
//-----------------------------------------------------------------------------------------------------------------


void jobs()
{
  bg *temp=background;
  int i=1;
  while(temp!=NULL)
  {
    fprintf(stdout, "[%d]%s[%d]\n",i++,temp->name,temp->pid );
    temp=temp->next;
  }
}
//-----------------------------------------------------------------------------------------------------------------

void killallbg()
{
  bg *temp=background;
  int t,flag=0;
  while(temp!=NULL)
  {
    t=kill(temp->pid,9);
    if(t<0)
    {
      flag=1;
      perror("Process could  not be killed\n");
    }
    temp=temp->next;
  }
  if(flag==0)
    printf("All backgrounfd processes have been killed!\n");
  else
    printf("Unable to killallbg\n");
  background=temp;
}
//-----------------------------------------------------------------------------------------------------------------

void kjob(char **tokens)
{
  bg *temp=background;
  int jn=atoi(tokens[1]);
  pid_t pid;
  int signal=atoi(tokens[2]);
  int t=jn,i;
  while(temp!=NULL && --jn)
    temp=temp->next;
  if(temp!=NULL)
    pid=temp->pid;
  else
    pid=-1;
  for(i=0;tokens[i]!=NULL;i++);

  if(i!=3)
      fprintf(stderr,"Invalid number of args\n");
  else if(pid!=-1)
      {
        if(kill(pid,signal)<0)
          perror("Signal not sent");
        else
        {
          printf("KILLED %s with pid [%d] and job number %d\n",temp->name, pid,t);
          delete(pid);
        }
      }
  else
        fprintf(stderr,"No such job number\n");
}
//-----------------------------------------------------------------------------------------------------------------

void fg(char **tokens)
{
  int i;
  pid_t pid,pgid,child_pid;
  //int shell;
  //pid_t shell_pgid;
  int status;
  bg *temp =background;
  int t=atoi(tokens[1]);
  for(i=0;tokens[i]!=NULL;i++);
  if(i==2)
  {
    while(temp!=NULL && --t)
      temp=temp->next;
    if(temp!=NULL)
      pid=temp->pid;
    else
      pid=-1;
    if(pid>=0)
    {
      fprintf(stderr, "%s\n", temp->name);
      pgid=getpgid(pid);
      tcsetpgrp(shell,pgid);
      //child_pid=pgid;
      if(killpg(pgid,SIGCONT)<0)
        perror("Cannot Continue");
      waitpid(pid,&status,WUNTRACED);
            //printf("\n");

      if(WIFSTOPPED(status))
      {  
        fprintf(stderr, "\n[%d]+ stopped %s\n",child_pid,temp->name );
        delete(pid);
      }
      else
      {
        delete(pid);
        //child_pid=0;
      }
      tcsetpgrp(shell,shell_pgid);
    }
    else
      fprintf(stderr, "No such job\n" );
  }
  else
      printf("Invalid number of arguments!\n" );
}
//-----------------------------------------------------------------------------------------------------------------
   
