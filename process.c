#ifndef PROCESS_H_
#define PROCESS_H_

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/*
Kipling Gillespie
CS485
11/21/2017

Data structure utilized by mysh shell program to hold process data.
*/

struct process
{
    pid_t pid;
    char* name;
};


void DelProc(struct process *target);
void InitProc(struct process *target);
struct process* SetProc(struct process *target, pid_t pid, char* name);

// frees memory for target's data members and its self. 
void DelProc(struct process *target)
{
    if(target->name)
    {
        free(target->name);
        target->name = NULL;
		
		free(target);
		target = NULL;
    }
}

// Initialize target's values to 0 and NULL;
// don't call on already initialized processes or you could leak name's memory.
void InitProc(struct process *target)
{
    target->name = NULL;
    target->pid = 0;
}

// Allocate memory for target and assign values to it's data members. 
struct process* SetProc(struct process *target, pid_t pid, char* name)
{
	// If target already has memory clean it up.
    if(target)
    {
        if(target->name)
            free(target->name);
        
        pid = 0;
        
        free(target);
    }
	else
		target = malloc(sizeof(struct process));
    
    target->name = malloc(sizeof(char*) * (strlen(name)+1));
    strcpy(target->name, name);

    target->pid = pid;
    return target; 
}

#endif