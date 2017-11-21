#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "process.c"
#include "variable.c"

#define bufsize 512
#define tokelen 128
/*
Kipling Gillespie
CS485
Proj4
11/19/2017

mysh is a very basic shell program written in C for an ubuntu linux virtual machine. 
I was not able to figure out sigprocmask before I had to turn it in, so I commented it out,
but when I tested mysh, it was stable.

input:
cmd followed by a program name will execute that program.



*/

void GetInput(char* output);
int Parser(char* input, char** tokens);
int Execute(char** tokens, int bg);


// catches SIGCHLD msg from when child exits
void catch(int num);

// Our set of built in commands to be added to a 
// function pointer array. 
int set(char **tokens);
int curdir(char **tokens);
int setprompt(char **tokens);
int cd(char **tokens);
int listp(char **tokens);
int show(char **tokens);
int showVar(char* name);
int bye(char **tokens);

// names of Built In commands 
char *BuiltIn[] = 
{
    "set",
    "curdir",
    "setprompt",
    "cd",
    "listp",
    "show",
    "bye",
    NULL
};

// A function pointer to my cmd functions
int (*BuiltInFunc[])(char **) =
{
    &set,
    &curdir,
    &setprompt,
    &cd,
    &listp,
    &show,
    &bye
};

// Arrays to hold active variables and Processes. 
struct variable** VarList = NULL;

// Adds variable name, and assigns it value.
// if name already exists, update it's value
void addVar(char* name, char* value);

// Array holds pointers to the active background processes.
struct process** ActiveProc = NULL;

// add and remove background process by pid
void addProcess(pid_t pid, char* name);
void remProcess(pid_t pid);

// Holds prompt string
char* prompt = NULL;

int main(int argc, char **argv)
{
    // Initialize Variables
    char* tokens[tokelen];
    memset(tokens, 0, tokelen);

	// VarList starts with the PATH variable
    VarList = malloc(sizeof(struct variable*)*2);
    VarList[0] = SetVar(VarList[0], "PATH", "/bin:/usr/bin");
	// update setenv whenever PATH is modified.
    setenv(VarList[0]->name, VarList[0]->value, 1);
    VarList[1] = NULL;

    ActiveProc = malloc(sizeof(struct process*));
    ActiveProc[0] = NULL;

    // initialize default prompt string
    prompt = malloc(sizeof(char) * strlen("mysh$ ")+1);
    strcpy(prompt, "mysh$ ");

    int status;
    int bg;
    char buffer[bufsize];
    memset(buffer, 0, bufsize);

	// set signal function for SIGCHLD to catch
    signal(SIGCHLD, catch);

    // Run Program
    do
    {
        // Display prompt prompt
        printf("%s", prompt);
		
		// Get Input
        GetInput(buffer);

        // Parse our input. If a background token is detected, set bg to 1.
        bg = Parser(buffer, tokens);

		// check for empty input. 
        if(tokens[0])
            // Execute given commands. 
            status = Execute(tokens, bg);
        
        // Clear the buffer and tokens contents
        memset(buffer, 0, bufsize);
        memset(tokens, 0, tokelen);

    }while(status);

    // Perform Clean Up
    free(prompt);

    int k = 0;
    while(VarList[k])
        DelVar(VarList[k]);

    free(VarList);

    k = 0;
    while(ActiveProc[k])
        DelProc(ActiveProc[k]);

    free(ActiveProc);

    // Finish
    return 0;
}

void GetInput(char* output)
{
    // Input: char* output, is an c-string of size bufsize
    // that consists of just NULL characters. We will store our input int   
    // this buffer. 
    // Output: the char* output as described above
    int c;
    int index = 0;
    int notDone = 1;
    // get input and feed it into output
    do
    {
        c = getchar();
        if(c == '\n')
        {
            // adds a new line to the end of out output
            output[index] = '\n';
            notDone = 0;
            
        }else if(c == EOF)
        {
            printf("\n");
            exit(0);
        }
        else{
            // input is valid and added to the output.
            output[index] = c;
        }

        index += 1;

    }while(notDone);
    
    return;
}

int Parser(char* input, char** tokens)
{
    // INPUT: A c_string input
    // OUTPUT:an array of c_strings
    int index = 0;
    char* delim = "\n   ";
    char* token;

    input[strlen(input)-1] = ' ';

    // delimate on newlines and white spaces
    token = strtok(input, delim);
    while(token)
    {
        // we either have a comment or the command should run in background.
        if(strcmp(token,"&") == 0)
        {
            return 1;
            tokens[index] = NULL;
        }

        tokens[index] = token;
        index += 1;
        if(index > tokelen)
            perror("Token String too Long.\n");

        token = strtok(NULL, delim);
    }
    
    // Terminate the string so I can find number of tokens.
    tokens[index] = NULL;
    return 0;
}

int Execute(char** tokens, int bg)
{ 
    pid_t pid, waitID;
    //sigset_t mask_all, mask_one, prev_one;
    int status;

    if(tokens[0][0] == '$')
    {
        if(!showVar(tokens[0]))
            fprintf(stdout, "%s is not a defined variable.\n", tokens[0]);
        
        return 1;
    }

    // Check for cmd
    if(strcmp(tokens[0], "cmd") == 0)
    { 
        if(!tokens[1])
        {
            fprintf(stdout, "cmd expects an argument.\n");
            return 1;
        }

        // create child process. 
        fflush(stdout);
        //sigfillset(&mask_all);
        //sigemptyset(&mask_one);
        //sigaddset(&mask_one, SIGCHLD);

        //sigprocmask(SIG_BLOCK, &mask_one, &prev_one); // Block Sigchld
        pid = fork();

        // Error Trap
        if(pid < 0)
        {
            fprintf(stderr, "%s\n", "fork");
            exit(EXIT_FAILURE);
        }

        // Child Trap.
        if(pid == 0)
        {
            //sigprocmask(SIG_SETMASK, &prev_one, NULL); // Unblock Sigchld
            if(execvp(tokens[1], &tokens[1]) == -1)
            {
                perror("mysh");
                
                fprintf(stderr, "%s didn't execute\n", tokens[1]);
            }

            exit(EXIT_FAILURE);
        }

        // Parent Trap
        else
        {
            if(!bg)
            {
                // wait for child process to exit before continuing.
                waitID = waitpid(pid, &status, 0);
                if(waitID < 0)
                {
                    fprintf(stdout, "%s\n", "waitpid error");
                    return 1;
                }
            }
            else// add background process to list.
            {
                // Parent proess
                //sigprocmask(SIG_BLOCK, &mask_all, NULL);
                addProcess(pid, tokens[1]); 
                // unblock Sigchld
                //sigprocmask(SIG_SETMASK, &prev_one, NULL);
            }
        }

        return 1;        
    }
    else
    {
		// check input for built in function.
        int index = 0;
        // is it a built in function?
        while(BuiltIn[index] != NULL)
        {
            if(strcmp(tokens[0], BuiltIn[index]) == 0)
                return BuiltInFunc[index](tokens);

            index += 1;
        }
    }

    fprintf(stdout, "%s is not a valid command.\n", tokens[0]);
    return 1;
}

// tokens[1] is a variable name
// tokens[2] is the value to set the variable to.
int set(char **tokens)
{
    if(tokens[1] == NULL)
    {
        fprintf(stderr, "Set expects a variable name and value.");
    }
    else if(tokens[2] == NULL)
    {
        fprintf(stdout, "%s: expects a value.", tokens[1]);
    }
    else
    {
        addVar(tokens[1], tokens[2]);
    }
    return 1;
}

void catch(int num)
{
    // Handler function for child signals
    pid_t pid;
    //sigset_t mask_all, prev_all;
    int status;
    //sigfillset(&mask_all);
    if((pid = waitpid(-1, &status, 0)) > 0);//&status, WNOHANG | WUNTRACED)) >= 0)
    {
        //sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
		// remove process with pid from our list ActiveProc
        remProcess(pid);
        //sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }

    //signal(SIGCHLD, catch);
}

// displays the current directory path
int curdir(char **tokens)
{
	//INPUT:char** tokens is not used, but it has to match my function pointer's format.
	//		passing this function NULL is fine.
    char buf[bufsize];

    if(getcwd(buf, bufsize) == NULL )
    {
        perror("mysh");
        fprintf(stderr, "Couldn't get current path\n");
    }
    else
    {
        fprintf(stdout, "DIR:%s\n", buf);
    }

    return 1;
}

// Change the prompt text to the value of the second token.
int setprompt(char **tokens)
{
    // Free prompt's memory and set to NULL
    if(prompt)
    {
        free(prompt);
        prompt = NULL;
    }

    // allocate memory plus room for %, ' ', and NULL
    size_t size = strlen(tokens[1]) + 3;
    prompt = malloc(size*sizeof(char));
    strcpy(prompt, tokens[1]);
    strcpy(&prompt[size - 3], "> ");

    return 1;
}

// Change directory to the string in the second token
int cd(char **tokens)
{
    if(tokens[1] == NULL)
        fprintf(stderr, "cd: exptected argument. ");
    else
            if(chdir(tokens[1]) != 0)
                perror("mysh");
            else// curdir doesn't use input, so pass it NULL.
                curdir(NULL);

    return 1;
}

// List all active processes
int listp(char **tokens)
{
    if(ActiveProc[0] == NULL)
    {   
        // No active process are running in the background.
        fprintf(stderr, "No Active Processes.\n");
    }
    else
    {
        // Iterate through ActiveProc list and print members until
        // we reach the null terminator.
        int index = 0;
        while(ActiveProc[index] != NULL)
        {
            printf("%d\t\t%s\n", ActiveProc[index]->pid, ActiveProc[index]->name);
            index += 1;
        }
    }

    return 1;
}

int show(char **tokens)
{
	// Display all variables stored in VarList
    if(VarList == NULL)
    {
        fprintf(stderr, "No Saved Variables.\n");
    }
    else
    {
        // Iterate through VarList and print members until
        // we reach the null terminator.
        int index = 0;
        while(VarList[index] != NULL)
        {
            printf("%s=%s\n", VarList[index]->name, VarList[index]->value);
            index += 1;
        }
    }

    return 1;
}

// search my variable list for a variable that shares tokens[0][1];
int showVar(char *name)
{
    int found = 0;
    if(VarList == NULL)
    {
        fprintf(stderr, "No Saved Variables.\n");
    }
    else
    {
        // Iterate through VarList and print members until
        // we reach the null terminator.
        int index = 0;
        while(VarList[index] != NULL)
        {
            //printf("%s, %s\n", &name[1], VarList[index]->name);
            // Find variable to print
            if(strcmp(&name[1], VarList[index]->name) == 0)
            {
                printf("%s=%s\n", VarList[index]->name, VarList[index]->value);
                found = 1;
            }

            index += 1;
        }
    }

    return found;
}

// Returns 0 to exit program
int bye(char **tokens)
{
    return 0;
}

void addVar(char* name, char* value)
{
	// Add a variable to VarList by copying contents to new array of 1 size larger
	// with a new variable with name and value added in alphabetic order. 
    int numVars = 0;
    int found = 0;

    // count number of currently stored variables
    while(VarList[numVars])
    {
        numVars++;
    }

    // empty list
    if(numVars == 0)
    {
        VarList = realloc(VarList, sizeof(struct variable *)*2);
        VarList[0] = SetVar(VarList[numVars], name, value);
        //VarList[1] = NULL;
        return;
    }

    // search for duplicates
    for(int i = 0; i < numVars; i++)
    {
        if(strcmp(VarList[i]->name, name)==0)
        {
            // should I update the PATH?
            if(strcmp("PATH", VarList[i]->name) == 0)
                setenv(name, value, 1);

            // update VarList[i];
            VarList[i] = SetVar(VarList[i], name, value);
            found = 1;
            break;
        }
    }

    // No duplicate was found
    if(!found)
    {
        struct variable *toAdd = NULL;
        toAdd = SetVar(toAdd, name, value);
        int cur = 0;
        struct variable **temp = malloc(sizeof(struct variable) * (numVars + 2));
        for(int i = 0; i < numVars; i++)
        {
            if(strcmp(toAdd->name, VarList[cur]->name) < 0 && !found)
            {
                temp[i] = toAdd;
                found = 1;
            }
            else
            {
                temp[i] = VarList[cur];
                cur++;
            }
        }

        if(!found)
        {
            temp[numVars] = toAdd;
            found = 1;
        }
        else
        {
            temp[numVars] = VarList[cur];
        }

		// free old list and point VarList to the new list.
        temp[numVars+1] = NULL;
        free(VarList);
        VarList = temp;
    }
}

void addProcess(pid_t pid, char* name)
{
	// Add a process to ActiveProc
    // create new process. 
    struct process *toAdd = NULL;
    toAdd = SetProc(toAdd, pid, name);


    // empty case
    if(ActiveProc[0] == NULL)
    {
        ActiveProc = realloc(ActiveProc, sizeof(struct process) * 2);
        ActiveProc[0] = toAdd;
        ActiveProc[1] = NULL;
        return;
    }

    size_t numProc = 0;
    while(ActiveProc[numProc])
    {
        numProc++;
    }
    int cur = 0;
    int added = 0;
    struct process ** temp = malloc(sizeof(struct process*) * (numProc+2));    
    for(size_t i = 0; i < numProc; i++)
    {
        if((toAdd->pid < ActiveProc[cur]->pid)  && !added)
        {
            temp[i] = toAdd;
            added = 1;
        }
        else
        {
            temp[i] = ActiveProc[cur];
            cur++;
        }
    }

    // the last element still needs to be added in
    if(!added)
    {
        temp[numProc] = toAdd;
        added = 1;
    }
    else
    {
        temp[numProc] = ActiveProc[cur];
    }

	// free old list and point ActiveProc to new list.
    temp[numProc +1] = NULL;
    free(ActiveProc);
    ActiveProc = temp;

}

void remProcess(pid_t pid)
{
	// Remove a process from ActiveProc
    int numProc = 0;
    int found = 0;
    while(ActiveProc[numProc])
    {
        if(ActiveProc[numProc]->pid == pid)
            found = 1;
        numProc++;
    }

    // pid is not in our list
    if(!found)
    {
        return;
        printf("PID not found");
    }

    struct process **temp = malloc(sizeof(struct process*) * (numProc));
    int cur = 0;
    for(int i = 0; i < numProc - 1; i++)
    {
        if(ActiveProc[cur]->pid != pid)
        {
            temp[i] = ActiveProc[cur];
            cur++;
        }
        else
        {
            cur++;
            temp[i] = ActiveProc[cur];
            cur++;
        }
    }
	
	// free old memory and point ActiveProc to new array. 
    temp[numProc-1] = NULL;
    free(ActiveProc);
    ActiveProc = temp;
}