Kipling Gillespie

CS485

Proj 4

11/19/2017


mysh
mysh is a linux shell program that allows for the execution

of multiple processes and view their pid and name. It also will


let you assign values to variables to be stored.



commands
set <variable> <value>
associates value with variable and stores information



setprompt <newPrompt>
makes newPrompt the shell prompt



show
 lists all the stored variables and their values



$<variable> 
shows the stored value of variable



curdir
Lists the PATH of the current directory



cd <directoryName>
changes the shell's directory to the directoryName. If 
directoryName starts with / 
then it will be an absolute filepath
otherwise it will be relative the current directory.



listp
 lists the pids and names of the currently running child 
processes.



bye 
exits shell.



cmd <param> 
executes file with param filename.The shell is suspended until
the child process finishes.



cmd <para> &
executes file with param filename. The shell will continue
 running while the child process continues 
running in the 
background.

