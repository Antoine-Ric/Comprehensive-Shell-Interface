#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

typedef struct {
    char ** items;
    size_t size;
} tokenlist;


int pathing(char ** argv);
char *get_input(void);
tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
tokenlist *get_tokens(char *input, char delimeter);
void free_tokens(tokenlist *tokens);

int main(int argc, char * argv[]){
	pid_t pid;
	int status;
	int sleeptime = 0;
	int snds = atoi(argv[1]);
	//converting seconds from argument into an integer
	pathing(argv);
	printf("%s\n", argv[2]);
	if((pid = fork())== 0)
	{
    	execv(argv[2], &argv[2]);
		printf("ERROR");
	}
	else {
        while(!waitpid(pid, &status, WNOHANG) && sleeptime != snds)
	//continue loop while waitpid is not satisified and sleeptime has not met seconds allowed
    	{
    		sleep(1);
		sleeptime++;
    	} if(!waitpid(pid, &status, WNOHANG)){
		if(kill(pid, SIGTERM) == 0){
			//If the child takes longer than amount of time it should have process is killed
			printf(" Process terminated\n");
		}
	}
	}
}

int pathing(char ** argv){
	int exists = 0;
        tokenlist *path = get_tokens(getenv("PATH"), ':');
 	if (argv[2][0] != '/'){
        	for(int i = 0; i < path->size; i++){
			char *fullPath = malloc(strlen(path->items[i]) + 1 + strlen(argv[2]) + 1);
                      strcpy(fullPath, path->items[i]);
                      strcat(fullPath, "/");
                      strcat(fullPath, argv[2]);
                      //test weither file exists
                      if(access(fullPath, X_OK) == 0){
                              printf("Files is Executeable: %s\n", fullPath);
                              exists = 2;
                              argv[2] = malloc(strlen(fullPath) +1);
                              argv[2] = strcpy(argv[2], fullPath);
                              i = path->size;
                      } else if(access(fullPath, F_OK) == 0){
                              printf("File Exists: %s\n", fullPath);
                              exists = 1;
                              i = path->size;
                      }
                                //printf("%s\n", fullPath);
                }
        }else{
                if(access(argv[2], X_OK) == 0){
                      printf("Files is Executeable: %s\n", argv[2]);
                      exists = 2;
                } else if(access(argv[2], F_OK) == 0){ 
                      printf("File Exists: %s\n", argv[2]);
                      exists = 1;
                }
        }
        free_tokens(path);
	return exists;
}


char *get_input(void) {
        char *buffer = NULL;
        int bufsize = 0;
        char line[5];
        while (fgets(line, 5, stdin) != NULL)       
        {
                int addby = 0;
                char *newln = strchr(line, '\n');
                if (newln != NULL)          
                        addby = newln - line;
                else
                        addby = 5 - 1; 
                buffer = (char *)realloc(buffer, bufsize + addby);
                memcpy(&buffer[bufsize], line, addby);
                bufsize += addby;
                if (newln != NULL)
                        break;
        }
        buffer = (char *)realloc(buffer, bufsize + 1);
        buffer[bufsize] = 0;      
        return buffer;
}

tokenlist *new_tokenlist(void) {
        tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
        tokens->size = 0;
        tokens->items = (char **)malloc(sizeof(char *));
        tokens->items[0] = NULL; /* make NULL terminated */
        return tokens;
}

void add_token(tokenlist *tokens, char *item) {
        int i = tokens->size;

        tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
        tokens->items[i] = (char *)malloc(strlen(item) + 1);
        tokens->items[i + 1] = NULL;
        strcpy(tokens->items[i], item);

        tokens->size += 1;  
}

tokenlist *get_tokens(char *input, char delimeter) {
        char *buf = (char *)malloc(strlen(input) + 1);
        strcpy(buf, input);   
        tokenlist *tokens = new_tokenlist();     
        char *tok = strtok(buf, &delimeter);
        while (tok != NULL)                  
        {           
                add_token(tokens, tok);
                tok = strtok(NULL, &delimeter);
        }
        free(buf);               
        return tokens;            
}

void free_tokens(tokenlist *tokens) {                 
        for (int i = 0; i < tokens->size; i++)
                free(tokens->items[i]);
        free(tokens->items);
        free(tokens);
}
