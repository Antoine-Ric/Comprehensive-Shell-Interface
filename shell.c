#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

//stores up to 10 background process pid and command
BackProcess backPros[10];

int main(){
    //stores last three valid commands
    char ** lastValid;    
    lastValid = (char**)malloc(3);
    lastValid[0] = malloc(2);
    lastValid[0] = strcpy(lastValid[0], "x");
    lastValid[1] = malloc(2);
    lastValid[1] = strcpy(lastValid[1], "x");
    lastValid[2] = malloc(2);
    lastValid[2] = strcpy(lastValid[2], "x");

    //set all pids equal to 0
    for(int i = 0; i < 10; i++){
        backPros[i].pid = 0;
    }
    //holds if it is a background process
    bool background;
    while (1) {
        background = false;
	//prompt that is printed out before waiting for command
        char *prompt;
        set_prompt(&prompt);
        printf("%s>", prompt);

        /* input contains the whole command
         * tokens contains substrings from input split by spaces
         */
        
        char *input = get_input();
 	//holds position of > < and | 1 and | 2 and end of commands may be before & symbol
        int inOut = 0, position [5] = {0, 0, 0, 0, 0};
        //token list that is not changed
	tokenlist *tokens = get_tokens(input, ' ');
	//a list of tokens that will change
        tokenlist *pipeList = get_tokens(input, ' ');

	//To keep from seg falt if the user just presses enter
        if(tokens->size > 0){
	//itentifies important characters & | > < ~ $
        specialCharIdent(pipeList, position, &inOut, &background);
	//int that holds -1 if it is built in, 0 if it does not exist, 1 if exists and 2 if executable
        int exists = 0;
	//finds that path to a command e.g. for ls finds bin/ls
        exists = pathing(pipeList, tokens, 0, lastValid);

	//if there is piping
        if(inOut >= 4){
	    //if it is executable
            if(exists == 2){
              exists = pathing(pipeList, tokens, position[2] + 1, lastValid);
            }
	    //if there are two pipes
	    if(inOut >= 8){
	      if(exists == 2){
                 exists = pathing(pipeList, tokens, position[3] + 1, lastValid);
	      }
	   }       
        }
	//built in command
        if(exists == -1){

        }else if(exists == 0){ //does not exits
            printf("%s: Command not found\n", pipeList->items[0]);
        }else if(exists == 1){ //not executable
            printf("%s: Permission Denied\n", pipeList->items[0]); 
        }else{
	    //holds the command and arguments for each command
	    char ** args;
	    char ** args2;
	    char ** args3;
	    // set to a high number and finds which is lowest in first 3 of position but not 0
	    int min = 10000000;
	    //if there is < > | or &
	    if(inOut > 0 || background){
               //finds the one that showed first < > or |
	       for(int i =0; i < 3; i++){
                    if(position[i] < min && position[i] != 0){
                        min = position[i];
                    }
               }

		
            	args = (char**)malloc(min);
                args2 = (char**)malloc(position[3]-position[2]-1);
		args3 = (char**)malloc(position[4]-position[3]-1);
		//sets the values of the arguments
	    	setArgs(pipeList, args, args2, args3, position, min, inOut, background);
	    }

	    int status;
	    //store current command as a valid one
	    //defined valid as command that exists and is executable
            storeValid(lastValid, tokens);
	    //if background call function for that
            if(background){
    		BackgroundProc(args, args2, args3, tokens, position, &inOut);
            }else if(inOut >= 4){ //if has pipes call function for that
                piping(args, args2, args3, &inOut);
            }else { //otherwise handle regular or input output redirection here
                pid_t pid = fork();
                if(pid == 0){
                    if(inOut == 0){ //no redirection
                        execv(pipeList->items[0], pipeList->items);
                    }else if(inOut % 4 == 1){ //input redirection
                        Iredirect(pipeList->items[position[0]+1]);
                        execv(pipeList->items[0], args);
                    }else if(inOut % 4 == 2){ //output redirection
                        Oredirect(pipeList->items[position[1]+1]);
                        execv(pipeList->items[0], args);
                    }else if(inOut % 4 == 3){ //both
                        Iredirect(pipeList->items[position[0]+1]);
                        Oredirect(pipeList->items[position[1]+1]);
                        execv(pipeList->items[0], args);
                    }
                }else if(pid >= 0){ //parent waits for child
                    waitpid(pid, &status, 0);
                }else{
                    printf("%s: Error Running Program", pipeList->items[0]);
                }
            }
            }
        }

        free(input);
        free_tokens(tokens);
        free_tokens(pipeList);
    }

    return 0;
}

//sets the value of the prompt to be printed out
void set_prompt(char ** prompt){
    //Form USER@MACHINE:PWD
    *prompt = malloc(strlen(getenv("USER"))+1);
    strcpy(*prompt, getenv("USER"));
    *prompt = realloc(*prompt, strlen(getenv("USER"))+1+strlen(getenv("MACHINE"))+1
	+strlen(getenv("PWD"))+1);
    strcat(*prompt, "@");
    strcat(*prompt, getenv("MACHINE"));
    strcat(*prompt, ":");
    strcat(*prompt, getenv("PWD"));
    strcat(*prompt, "\0");
}

//replace command with full path e.g. ls is /bin/ls
int pathing(tokenlist *pipeList, tokenlist *tokens, int num, char ** lastValid){
    int exists = 0;
    //brakes path environment variable into its parts using : as diliminator
    tokenlist *path = get_tokens(getenv("PATH"), ':');
    //for built in exit command
    if(num == 0 && strcmp(pipeList->items[0], "exit") == 0){
        //wait for up to 10 children
	for(int i = 0; i < 10; i++){
            int status;
            bool first = true;
            if(backPros[i].pid != 0){
		//Checks if there is a background process being run
                while(!waitpid(backPros[i].pid, &status, WNOHANG)){
                    if(first){
                        printf("Waiting on process %i\n", i+1);
                        first = false;
                    }
                }
            }
        }
	//find number of valid commands
        int numValid = 0;
                for(int i = 0; i < 3; i++){
                        if(strcmp(lastValid[i], "x") != 0){
                                numValid++;
                        }
                }
	    //if no valid commands
                if(numValid == 0){
                        printf("No valid commands\n");
                }else{ //if at least one valid command
                        printf("Last[%i] valid commands:\n", numValid);
                        for(int i = 0; i < numValid; i++){
                                printf("[%i]: %s\n", i+1, lastValid[i]);
                        }
                }
                exit(0);
    }else if(num == 0 && strcmp(pipeList->items[0], "cd") == 0){
	//built in cd command
        exists = -1;
        if(pipeList->size == 1){ // if only cd
	    //set to home
            chdir(getenv("HOME"));
            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            setenv("PWD", cwd, 1); //change evironment variable so prompt changes
            storeValid(lastValid, tokens); //store as valid command
        }else if(pipeList->size == 2){ //if cd directory
            if(access(pipeList->items[1], F_OK) == 0){ // if it is a file
                if(access(pipeList->items[1], R_OK) != 0){ //if you have read permission
                    printf("%s: Permission Denied.\n", pipeList->items[1]);
                }else if(chdir(pipeList->items[1]) != 0){ //if it is a directory
                    printf("%s: Not a directory\n", pipeList->items[1]);
                }else{ 
                    char cwd[1024];
                    getcwd(cwd, sizeof(cwd));
                    setenv("PWD", cwd, 1);
                    storeValid(lastValid, tokens);
                }
            }else{ //did not exist
                printf("%s: No such file or directory\n", pipeList->items[1]);
            } 
        }else{ //more then 1 arguement
            printf("%s: Too many arguments.\n", pipeList->items[0]);
        }
    }else if(num == 0 && strcmp(pipeList->items[0], "jobs") == 0){ //built in jobs command
	    //store as valid command
            storeValid(lastValid, tokens);
        for(int i = 0; i < 10; i++){ // loopes through possible 10 background processes
            int status;
            if(backPros[i].pid != 0 && !waitpid(backPros[i].pid, &status, WNOHANG)){ //if running
                //print out information
		printf("[%i]  +  %i %s\n", i+1, backPros[i].pid, backPros[i].command);
            }
        }
        exists = -1;
    }else if (pipeList->items[num][0] != '/'){ // if it is an absolute path
        for(int i = 0; i < (int)path->size; i++){
            char *fullPath = malloc(strlen(path->items[i]) + 1 + strlen(pipeList->items[num]) + 1);
            strcpy(fullPath, path->items[i]);
            strcat(fullPath, "/");
            strcat(fullPath, pipeList->items[num]);
            //test weither file exists
            if(access(fullPath, X_OK) == 0){
                exists = 2;
                pipeList->items[num] = realloc(pipeList->items[num], strlen(fullPath) +1);
                pipeList->items[num] = strcpy(pipeList->items[num], fullPath);
                i = path->size;
            } else if(access(fullPath, F_OK) == 0){
                exists = 1;
                i = path->size;
            }
        }
    }else{
        if(access(pipeList->items[num], X_OK) == 0){
           exists = 2;
        } else if(access(pipeList->items[num], F_OK) == 0){  
            exists = 1;
        }
    }
    return exists;
}

//input redirection
void Iredirect(char * INfile){
    int fp;
    if (INfile != NULL){
		//Opening input file and setting appropriate read only permissions
        fp = open(INfile, O_RDONLY);
        if (fp == -1){
			//Checking that file was opened successfully
            printf("Could not open file\n");
        }
	//close stdin
        close(0);
	//move file to 0 spot
        dup2(fp, 0);
	//close extra file in 3 spot
        close(fp);
    }
}

//output redirection
void Oredirect(char * Outfile){
    int fp;
    if (Outfile != NULL){
		//Opening file and setting appropriate permissions
        fp = open(Outfile, O_RDWR | O_CREAT | O_TRUNC, 00400 | 00200);
        if (fp == -1){
           printf("Could not open file\n");
        }
	//close stdout
        close(1);
        int fp2;
	//move file to 1 spot
        fp2 = dup2(fp, 1);
        if (fp2 == -1){
           printf("Unable to dup\n");
        }
	//close extra file in 3 spot
        close(fp);
    }
}

//piping
void piping(char **command1, char **command2, char ** command3, int * inOut){
    //holds the position that path function sets
    int fd[2];
    pipe(fd);
    int status, status2;
    pid_t pid = fork();
    
    if(pid == 0){//child process
	//close stdout
        close(1);
	//move pipe_out to 1 spot
        dup2(fd[1], 1);
	//close pipe_in and pipe_out
        close(fd[0]);
        close(fd[1]);
	//execute command
        execv(command1[0], command1);
        printf("Error in execution");
	exit(1);
    }else if(pid >= 0){
	//set up to do 2 pipes
	int fd2[2];
	//if there are 2 pipes
	if((*inOut) >=8){
	   pipe(fd2);
	}
        pid_t pid2 = fork();
        if(pid2 == 0){
            close(0);
	    //move pipe1_in to 0 spot
            dup2(fd[0], 0);
            close(fd[0]);
            close(fd[1]);
	    //if two pipes also move pipe2_out to 1 spot 
	    if((*inOut) >= 8){
		close(1);
		dup2(fd2[1], 1);
		close(fd[0]);
		close(fd[1]);
	    }
	    execv(command2[0], command2);
	    exit(1);
        }else if(pid2 >= 0){
	    //if 2 pipes
	    if((*inOut) >= 8){
		pid_t pid3 = fork();
	        if(pid3 == 0){ //third child
		   close(0);
		   //moves pipe2_in to 0 spot
                   dup2(fd2[0], 0);                  
                   close(fd[0]);
                   close(fd[1]);
                   close(fd2[0]);
                   close(fd2[1]);
		   execv(command3[0], command3);
		   exit(1);
		}else{
		   close(fd[0]);
                   close(fd[1]);
                   close(fd2[0]);
                   close(fd2[1]);
		   //wait for all 3 children
                   waitpid(pid, &status, 0);
                   waitpid(pid2, &status, 0);
                   waitpid(pid3, &status, 0);

		}
            }else{
            	close(fd[0]);
                close(fd[1]);
                //only have 2 children to wait on
		waitpid(pid, &status, 0);
                waitpid(pid2, &status2, 0);
	    }
	}
    }else{
        printf("Error in piping\n");
    }
}

//background processing when & sybol apears
void BackgroundProc(char **command1, char ** command2, char ** command3, tokenlist * tokens, 
	int * position, int * inOut){
    //mumber of background process
    int numPros = 0;
    for(int i = 0; i < 10; i++){ // look for next open spot
	int status;
        if(backPros[i].pid  == 0 || waitpid(backPros[i].pid, &status, WNOHANG)){
           numPros = i; //asigns that value
	   //stores the whole command in there
           backPros[i].command = malloc(strlen(tokens->items[0])+1+1);
           backPros[i].command = strcpy(backPros[i].command, tokens->items[0]);
           backPros[i].command = strcat(backPros[i].command, " ");
           for(int j = 1; j < (int)tokens->size; j++){
               backPros[i].command = realloc(backPros[i].command, strlen(tokens->items[j])+1+1);
               backPros[i].command = strcat(backPros[i].command, tokens->items[j]);
               backPros[i].command = strcat(backPros[i].command, " ");
           }
	   //skips to end of loop if found a spot
           i = 10;
        }
    }

    //forks to have a parent process that can keep executing 
    //while child waits on grandchild to complete
    pid_t pid = fork();
    if(pid == 0) {
	//if there is piping
        if((*inOut) >= 4){
            piping(command1, command2, command3, inOut);
	    //once it is done frees up spot for other backPros
            backPros[numPros].pid = 0;
	    //prints out message saying it is done
            printf("\n[%i] + done %s\n", numPros + 1, backPros[numPros].command);
	    //prints out prompt
	    char * prompt;
            set_prompt(&prompt);
            printf("%s> ", prompt);
            exit(1);
        }else{
	    //make grandchild process
            pid_t pid2 = fork();
            if(pid2 == 0){ 
		//executes based upon input output redirection needs
                if((*inOut) == 0){
                      execv(command1[0], command1);
                }else if((*inOut) % 4 == 1){
		       //Checks if only Input redirection is required
                      Iredirect(tokens->items[position[0]+1]);
                      execv(command1[0], command1);
                }else if((*inOut) % 4 == 2){
			//Checks if only output redirection is required
                      Oredirect(tokens->items[position[1]+1]);
                      execv(command1[0], command1);
                }else if((*inOut) % 4 == 3){
			//Checks if both input and output redirection is required
                      Iredirect(tokens->items[position[0]+1]);
                      Oredirect(tokens->items[position[1]+1]);
                      execv(command1[0], command1);
                }
                printf("error in execution");
                exit(1);
            }else{
		//child waits for grandchild to finish
                int status;
                while(!waitpid(pid2, &status, WNOHANG)){}
                backPros[numPros].pid = 0;		
		//prints out message and new prompt
                printf("\n[%i] + done %s\n", numPros + 1, backPros[numPros].command);
                char * prompt;
                set_prompt(&prompt);
                printf("%s> ", prompt); 
                exit(1);
            }
        }
    }else {
	//parent prints initial message about background process
        backPros[numPros].pid = pid;
        printf("[%i] %i\n", numPros+1, pid);    
    }
    
}

//store valid commands
void storeValid(char ** lastValid, tokenlist *tokens){
    char *valid = malloc(strlen(tokens->items[0])+1);
    valid = strcpy(valid, tokens->items[0]);
    for(int i = 1; i < (int)tokens->size; i++){
        valid = realloc(valid, strlen(valid) + 1 +strlen(tokens->items[i]) + 1);
        valid = strcat(valid, " ");
        valid = strcat(valid, tokens->items[i]);
    }
    if(strcmp(lastValid[0], "x") == 0){
        lastValid[0] = realloc(lastValid[0], strlen(valid)+1);
        lastValid[0] = strcpy(lastValid[0], valid);
    }else if(strcmp(lastValid[1], "x") == 0){
        lastValid[1] = realloc(lastValid[1], strlen(valid)+1);
        lastValid[1] = strcpy(lastValid[1], valid);
    }else if(strcmp(lastValid[2], "x") == 0){
        lastValid[2] = realloc(lastValid[2], strlen(valid)+1);
        lastValid[2] = strcpy(lastValid[2], valid);
    }else{//swaps in if there is already three valid commands
        lastValid[0] = realloc(lastValid[0], strlen(lastValid[1])+1);
        lastValid[0] = strcpy(lastValid[0], lastValid[1]);
        lastValid[1] = realloc(lastValid[1], strlen(lastValid[2])+1);
        lastValid[1] = strcpy(lastValid[1], lastValid[2]);
        lastValid[2] = realloc(lastValid[2], strlen(valid)+1);
        lastValid[2] = strcpy(lastValid[2], valid);
    }
} 


//itentify < > | & characters
void specialCharIdent(tokenlist * pipeList, int * position, int * inOut, bool * background){
    position[2] = 0;
    position[3] = 0;
    position[4] = 0;
    for (int i = 0; i < (int)pipeList->size; i++) {
        //Environmental Variables Handling
		//Checks if $ is the first character
		// If so envName is set to the position after $
        if(pipeList->items[i][0] == '$'){
            char *envName = malloc(strlen(pipeList->items[i]+1));
            strcpy(envName, pipeList->items[i] + 1);
            if(getenv(envName)){
                char *envVar = malloc(strlen(getenv(envName))+1);
                strcpy(envVar, getenv(envName));
            
                if(envVar != NULL){
	//Checks that envVar was set appropriately before copying back into piping list
                    strcpy(pipeList->items[i], envVar);
                }
            }
        }
            
        //Tilde expansion
            
        if (pipeList->items[i][0] == '~' && pipeList->items[i][1] == '/'){
			//Checks for first test case ~/ of tilde expansion
            char *user  = malloc(strlen(getenv("HOME"))+1);
            strcpy(user, getenv("HOME"));
            user = realloc(user, strlen(getenv("HOME"))+strlen(pipeList->items[i]+1));
            strcat(user, pipeList->items[i] + 1);
            strcpy(pipeList->items[i], user);
        }else if (pipeList->items[i][0] == '~' && pipeList->items[i][1] != '/'){
			//Checks for second test case just ~ of tilde expansion
            char *user = malloc(strlen(getenv("HOME"))+1);
            strcpy(user, getenv("HOME"));
            strcpy(pipeList->items[i], user);
        }

		//Setting variable inOout to 1 if only input redirection is required
        if(pipeList->items[i][0] == '<'){
            (*inOut) += 1;
            position[0] = i;
        }

	//Setting variable inOut to 2 if only output redirection is required
	//Setting variable inOut to 3 if input redirection is also required
        if(pipeList->items[i][0] == '>'){
            (*inOut) += 2;
            position[1] = i;
        }

	//adding 4 to the value if pipeing is required
        if(pipeList->items[i][0] == '|'){
            (*inOut) += 4;
            if(position[2] == 0){
                position[2] = i;
            }else if(position[3] == 0){
                position[3] = i;
            }
        }
    }
        
    if(pipeList->items[pipeList->size-1][0] == '&'){
        (*background) = true;
        if((*inOut) < 4){
            position[2]  = pipeList->size-1;
        }
    }


    if((*inOut) < 8 && position[3] == 0){
        if(!(*background)){
            position[3] = pipeList->size;
        }else{
            position[3] = pipeList->size-1;
        }
    }
    if((*background)){
	position[4] = pipeList->size-1;
    }else{
	position[4] = pipeList->size;
    }
}

//set argument values that will be passed into execv
void setArgs(tokenlist * pipeList, char ** args, char ** args2, char ** args3, int * position, 
	int  min, int inOut, bool background){
	if(inOut > 0 || background){
            int count = 0;
	    int count2 = 0;
            args = (char**)realloc(args, min);
            for(int i = 0; i < min; i++){
               args[i] = malloc(strlen(pipeList->items[i])+1);
               args[i] = strcpy(args[i], pipeList->items[i]);
	    }
            if(inOut >= 4){
               args2 = (char**)realloc(args2, position[3]-position[2]-1);
               for(int i = position[2]+1; i < position[3]; i++){
                   args2[count] = malloc(strlen(pipeList->items[i])+1);
                   args2[count] = strcpy(args2[count], pipeList->items[i]);
                   count++;
               }
            }
	    if(inOut >= 8){
	 	args3 = (char**)realloc(args3, position[4]-position[3]-1);
		for(int i = position[3]+1; i < position[4]; i++){
                   args3[count2] = malloc(strlen(pipeList->items[i])+1);
                   args3[count2] = strcpy(args3[count2], pipeList->items[i]);
		   count2++;
		}
	    }
     }	
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
        for (int i = 0; i < (int)tokens->size; i++)
                free(tokens->items[i]);
        free(tokens->items);
        free(tokens);     
}
