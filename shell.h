#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

typedef struct {
    char ** items;
    size_t size;
} tokenlist;

typedef struct{
        char * command;
        pid_t pid;
} BackProcess;

void set_prompt(char ** prompt);
void BackgroundProc (char ** command1, char ** command2, char **command3, tokenlist * tokens, 
    int * position, int * inOut);
int pathing(tokenlist *pipeList, tokenlist *tokens, int item, char ** lastValid);
void Iredirect(char * file);
void Oredirect(char * file);
void piping(char **command1, char **command2, char ** command3, int * inOut);
void storeValid(char ** lastValid, tokenlist *tokens);
void specialCharIdent(tokenlist *pipeList, int * position, int * inOut, bool * background);
void setArgs(tokenlist * pipeList, char ** args, char ** args2, char ** args3 , int * position, 
    int  min, int inOut, bool background);
char * get_input(void);
tokenlist * get_tokens(char *input, char delimeter);
tokenlist * new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);


