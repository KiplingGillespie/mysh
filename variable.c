#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/*
Kipling Gillespie
CS485
11/21/2017

Data structure utilized by mysh shell program to hold variable data.
*/

struct variable
{
    char* value;
    char* name;
};


void DelVar(struct variable *target);
void InitVar(struct variable *target);
struct variable * SetVar(struct variable *target, char* name, char* value);

// frees memory for target's data members and its self. 
void DelVar(struct variable *target)
{
    if(target)
    {
        // Delete dynamkc memory and set to NULL
        if(target->name)
        {
            free(target->name);
            target->name = NULL;
        }

        // Delete dynamic memory and set to NULL
        if(target->value)
        {
            free(target->value);
            target->value = NULL;
        }

		// free memory and set to NULL
        free(target);
        target = NULL;
    }
}

// Set's target's data members to NULL pointers so we know if they haven't been set. 
void InitVar(struct variable *target)
{
    target->name = NULL;
    target->value = NULL;
}

// Creates dynamic memory for target variable.
struct variable * SetVar(struct variable *target, char* name, char* value)
{
    // free memory
    if(target)
    {   
        if(target->value)
            free(target->value);

        if(target->name)
            free(target->name);
            
        free(target);
    }

    target = malloc(sizeof(struct variable));
    
    target->value = malloc(sizeof(char) * (strlen(value)+1));
    strcpy(target->value, value);

    target->name = malloc(sizeof(char) * (strlen(name)+1));
    strcpy(target->name, name);

    return target;
}

#endif