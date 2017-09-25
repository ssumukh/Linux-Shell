# Implementation of linux shell

### Features:

> * Executes all the commands and programs that can be executed on a normal linux shell
> * Implemented cwd, cd, ls explicitly
> * Can run programs on foreground as well as background
> * Defined commands like "pinfo <pid>" prints all the information about the process with that pid.
> * (BONUS feature - 1 implemented) Shows when a background process exits on the foreground.
> * (BONUS feature - 2 implemented) 'nightswatch -n <time in seconds> <command>'. The command can be 'interrupt', when it will display the interrupt information stored in /proc/interrupts, or 'dirty', which displays the dirty memory size stored in /proc/meminfo, every 'n' seconds
> * It can handle background and foreground processes.
> * It can also handle input/output redirections and piping. 
> * Also supports signal such as CTRL+C, CTRL+Z.
> * 'killall' kills all background processes
> * 'jobs' displays list of all processes
> * 'kjob <pid>' allows the user to kill a particular process

### Execution:

Type command 'make' to create the executable file 'shell'. To run it, use './shell' command
