#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include "arraylist.h"
#include <limits.h>
#include <glob.h>

#ifndef DEBUG
#define DEBUG 0
#endif

//global variable tracking exit status when each command is executed
int exitStatus = 0;

//initialize arraylist
void al_init(arraylist_t *L, unsigned size){
    L->data = malloc(size * sizeof(char*));
    L->length = 0;
    L->capacity = size;
}

//free arraylist
void al_destroy(arraylist_t *L){
        free(L->data);
    
}

//geet length of arraylist
unsigned al_length(arraylist_t *L){
    return L->length;
}

//add an element to the arraylist
void al_push(arraylist_t *L, char* item){
    if (L->length == L->capacity) {
        L->capacity *= 2;
        char **temp = realloc(L->data, L->capacity * sizeof(char*));
        if (!temp) {
        // for our own purposes, we can decide how to handle this error
        // for more general code, it would be better to indicate failure to our
        // caller
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
        }
        L->data = temp;
        if (DEBUG) printf("Resized array to %u\n", L->capacity);
    }
    L->data[L->length] = item;
    L->length++;
}

// returns 1 on success and writes popped item to dest
// returns 0 on failure (list empty)
//pop all elements from arraylist
int al_pop(arraylist_t *L, char **dest){
    if (L->length == 0) return 0;
    L->length--;
    *dest = L->data[L->length];
    return 1;
}

//get the element in the arraylist at a specific index
char *al_get(arraylist_t *L, int i) {
    return L->data[i];
}

//change directory using chdir()
void cd(char* argument){
    if(chdir(argument)!=0){
        exitStatus = 1;
        perror("cd");
    }else {
        exitStatus = 0; 
    }
}

//print absolute path of working directory using getcwd()
void pwd() {
    char workingDirectory[2048]={0};
    if((getcwd(workingDirectory, sizeof(workingDirectory))) != NULL) {
        exitStatus = 0;
        strcat(workingDirectory,"\n");
        write(STDOUT_FILENO,workingDirectory,strlen(workingDirectory));
    }else{
        exitStatus = 1;
        perror("pwd");
    }
}

//gives search path for bare names
void which(char *argument){
    char dest[2048]={0};
    strcat(dest, "/usr/local/bin/");
    strcat(dest, argument);

    char dest2[2048]={0};
    strcat(dest2, "/usr/bin/");
    strcat(dest2, argument);

    char dest3[2048]={0};
    strcat(dest3, "/bin/");
    strcat(dest3, argument);
    
    if(access(dest, F_OK)==0){ 
        printf("%s\n",dest);
        exitStatus = 0;
    }else if(access(dest2,F_OK)==0){
        printf("%s\n",dest2);
        exitStatus = 0;
    }else if(access(dest3,F_OK)==0){
        printf("%s\n",dest3);
        exitStatus = 0;
    }else{
        exitStatus = 1;
        perror("which");
    }
}

//prints out words after exit command
void exitFunction(arraylist_t arguments) {
    if(al_length(&arguments)>1){
        for (int i = 1; i < al_length(&arguments); i++) {
            printf("%s", al_get(&arguments,i));
            printf(" ");   
        }
        printf("\n");
    }
}

//uses the first word in the command to determine how to execute command
void checkFirstArg(arraylist_t command,int pipeExists, int redirectionExists){
    char *firstWord=al_get(&command,0); 
    if(firstWord[0] == '/'){
        char* args[al_length(&command)+1];
        for(int i=0;i<al_length(&command);i++){
            args[i]=al_get(&command,i);
        }
        args[al_length(&command)]=NULL;
        if(pipeExists==1 || redirectionExists ==1){
                execv(firstWord,args);
                exitStatus = 1;
                perror("Couldn't execute command\n");
        }else{
            pid_t child = fork();   
            pid_t childWait;
            if(child == 0){
                //in child
                // al_destroy(&command);  
                execv(firstWord,args);
                exitStatus = 1;
                perror("Couldn't execute command\n");
            }
            //in parent
            wait(&childWait);
        }
    }else if((strcmp(firstWord,"cd")!=0 && strcmp(firstWord,"pwd")!=0 && strcmp(firstWord,"which")!=0 && strcmp(firstWord,"exit")!=0)){
        char* args[al_length(&command)+1];
        for(int i=0;i<al_length(&command);i++){
            args[i]=al_get(&command,i);
        }
        args[al_length(&command)]=NULL;
        char dest[2048]={0};
        strcat(dest,"/usr/local/bin/");
        strcat(dest,firstWord);

        char dest2[2048]={0};
        strcat(dest2,"/usr/bin/");
        strcat(dest2,firstWord);

        char dest3[2048]={0};
        strcat(dest3,"/bin/");
        strcat(dest3,firstWord);
        if(access(dest, X_OK)==0){
            if(pipeExists==1 || redirectionExists ==1){
                exitStatus = 0;
                execv(dest,args);
                exitStatus = 1;
                perror("Couldn't execute command\n");
            }else{
                pid_t child = fork();
                pid_t childWait; 
                if(child == 0){
                    //in child
                    exitStatus = 0;
                    execv(dest,args);
                    exitStatus = 1;
                    perror("Couldn't execute command\n");
                }
                //in parent
                wait(&childWait);
            }
        }else if(access(dest2,X_OK)==0){
            if(pipeExists==1 || redirectionExists ==1){
                exitStatus = 0;
                execv(dest2,args);
                exitStatus = 1;
                perror("Couldn't execute command\n");
            }else{
                pid_t child = fork();
                pid_t childWait; 
                if(child == 0){
                    //in child
                    exitStatus = 0;
                    execv(dest2,args);
                    exitStatus = 1;
                    perror("Couldn't execute command\n");
                }
                //in parent
                wait(&childWait);
            }
        }else if(access(dest3,X_OK)==0){
            if(pipeExists==1 || redirectionExists ==1){
                exitStatus = 0;
                execv(dest3,args);
                exitStatus = 1;
                perror("Couldn't execute command\n");
            }else{
                pid_t child = fork();
                pid_t childWait; 
                if(child == 0){
                    //in child
                    exitStatus = 0;
                    execv(dest3,args);
                    exitStatus = 1;
                    perror("Couldn't execute command\n");

                }
                //in parent
                wait(&childWait);
            }
        }else{
            exitStatus = 1;
            printf("word %s\n", firstWord);
            perror("Not a valid command");
        }
    }else if((strcmp(firstWord, "cd")==0 || strcmp(firstWord,"pwd")==0 || strcmp(firstWord,"which")==0 ||strcmp(firstWord,"exit")==0)){
        if(strcmp(firstWord,"cd")==0){
            if(al_length(&command)==2 && al_get(&command,1) != NULL){
                cd(al_get(&command,1));
            }else{
                exitStatus = 1;
                perror("Incorrect amount of arguments for this command");
            }
        }else if(strcmp(firstWord,"pwd")==0){
            if(al_length(&command)==1){
                pwd();
            }else{
                exitStatus = 1;
                perror("Incorrect amount of arguments for this command");
            }
        }else if(strcmp(firstWord,"which")==0){
            if(al_length(&command)==2 && al_get(&command,1) != NULL){
                which(al_get(&command,1));
            }
            else{
                exitStatus = 1;
                perror("Incorrect amount of arguments for this command");
            }
        }else if(strcmp(firstWord,"exit")==0){
            exitStatus = 2;
            exitFunction(command);
            
        }
    }
    if(exitStatus==2){
        return;
    }
}

//check if we need to redirect input or output
void checkForRedirection(arraylist_t command,int pipeExists) {
    int redirectionExists = 0;
    char* inputFile = NULL;
    char* outputFile= NULL;
    arraylist_t argumentList; 
    al_init(&argumentList,1);
    for(int i=0;i<al_length(&command);){ //traverse through arraylist of tokens that make up command
        if(strcmp(al_get(&command,i),"<")==0){ //recongize < token
            redirectionExists = 1;
            inputFile = al_get(&command,i+1); //extract file following redirection token
            if (i+2 < al_length(&command)) {
                i=i+2;
            }
            else {
                break;
            }
        }else if(strcmp(al_get(&command,i),">")==0){ //recognize > token
            redirectionExists = 1;
            outputFile = al_get(&command,i+1); //extract file following redireection token
            if (i+2 < al_length(&command)) {
                i=i+2;
            }
            else {
               break;
            }
        }else{
            al_push(&argumentList,al_get(&command,i));
            i++;
        }
    }

    if(redirectionExists == 1 && pipeExists==0){ //only redirection exists
        pid_t child =fork();
        pid_t childWait;
        if (child == 0) {
            if(inputFile !=NULL){
                int fd1 = open(inputFile, O_RDONLY);
                if(fd1 == -1){
                    perror("Can't open file");
                    exitStatus = 1; 
                }
                if (dup2(fd1, STDIN_FILENO) == -1) {
                        perror("dup2 output failed");
                        exitStatus = 1; 
                }
                close(fd1);
            }

            if(outputFile!=NULL){
                int fd2 = open(outputFile, O_WRONLY | O_TRUNC | O_CREAT, 0640);
                if(fd2 == -1){
                    perror("Can't open file");
                    exitStatus = 1;
                }
                if (dup2(fd2, STDOUT_FILENO) == -1) {
                        perror("dup2 output failed");
                        exitStatus = 1; 
                }
                close(fd2);
            }
            checkFirstArg(argumentList, pipeExists, redirectionExists);   
        }
        wait(&childWait);                    
    }else if(redirectionExists==1 && pipeExists == 1){ //both redirection and piping exists
        if(inputFile !=NULL){
                int fd1 = open(inputFile, O_RDONLY);
                if(fd1 == -1){
                    perror("Can't open file");
                    exitStatus = 1; 
                }
                if (dup2(fd1, STDIN_FILENO) == -1) {
                        perror("dup2 output failed");
                        exitStatus = 1; 
                }
                close(fd1);
            }

            if(outputFile!=NULL){
                int fd2 = open(outputFile, O_WRONLY | O_TRUNC | O_CREAT, 0640);
                if(fd2 == -1){
                    perror("Can't open file");
                    exitStatus = 1;
                }
                if (dup2(fd2, STDOUT_FILENO) == -1) {
                        perror("dup2 output failed");
                        exitStatus = 1; 
                }
                close(fd2);
            }
            checkFirstArg(argumentList, pipeExists, redirectionExists);   

    }else{ //no redirection exists
        checkFirstArg(argumentList, pipeExists, redirectionExists);   
    }
    //pop all elements from argumentList arraylist
    char* b;
    while (al_pop(&argumentList, &b)) {
        continue;
    }
    //free argumentList arraylist
    al_destroy(&argumentList);
    
    //check exitStatus to see if exit command was read
    if(exitStatus == 2){
        return;
    }              
}

//check to see if command has a "|"
void checkForPipeline(arraylist_t command){
    arraylist_t firstProcess;
    al_init(&firstProcess,1);
    arraylist_t secondProcess;
    al_init(&secondProcess,1);

    int pipeExists = 0;
    for(int i=0;i<al_length(&command);i++){
        if(strcmp(al_get(&command,i),"|")==0){
            pipeExists = 1;
            al_init(&firstProcess,1);
            for(int j=0;j<i;j++){
                al_push(&firstProcess,al_get(&command,j)); //fill firstProcess arraylist for command to the left of "|"
            }
           
            for (int k = i+1; k < al_length(&command); k++) {
                al_push(&secondProcess, al_get(&command, k)); //fill firstProcess arraylist for command to the right of "|"
            }
            
        }
    }

    if(pipeExists == 1){
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe error");
            exitStatus = 1;
        }
        pid_t pid = fork(); //forking for first process
        if (pid == -1) {
            perror("fork error");
            exitStatus = 1;
        } else if (pid == 0) { 
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);

            checkForRedirection(firstProcess,1);
        }else{ 
            close(pipefd[1]);

            pid_t pid2 = fork(); //forking for second process
            if (pid2 == -1) {
                perror("fork error");
                exitStatus = 1;
            } else if (pid2 == 0) { 
                dup2(pipefd[0], STDIN_FILENO);

                close(pipefd[0]);

            checkForRedirection(secondProcess,1);
            } else { 
                close(pipefd[0]);

                waitpid(pid, NULL, 0);
                waitpid(pid2, NULL, 0);
            }

            char* a;
            while (al_pop(&firstProcess, &a)) {
                continue;
            }
            char* b;
            while (al_pop(&secondProcess, &b)) {
                continue;
            }
        }
    }else{
        checkForRedirection(command,0); //when there is no pipe symbol
    }

    //free both firstProcess and secondProcess arraylists
    al_destroy(&firstProcess);
    al_destroy(&secondProcess);
    
    if (exitStatus == 2) {
        return;
    }
}

//take in file descriptor and mode and accordingly either read once or read every time user enters a new command
void inputLoop(int fd,int mode) {
    size_t fileSize = lseek(fd, (size_t)0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    if(fileSize == -1){
        fileSize = MAX_INPUT;
    }

    char* buffer= (char*)malloc(fileSize+1); 
    const char commandDelimiters[] = "\n";
    const char wordDelimiters[] = " ";
    char* command;
    char* commandSaveptr = NULL;
    char* wordSaveptr = NULL;
    if(mode==2){ //batch mode, reads the file using the uses strtok to divide buffer by new line
        int bytes = read(fd, buffer, fileSize+1);
        if(bytes <=0){
            exit(EXIT_SUCCESS);
        }
        buffer[bytes]='\0';
        command = strtok_r(buffer, commandDelimiters, &commandSaveptr);
    }
  
    int count = 1;
    do{
        arraylist_t tokensOfCommand;
        al_init(&tokensOfCommand,1);
         if(mode==1){ //interactive mode 
                write(STDOUT_FILENO,"mysh> ",strlen("mysh> "));
                int bytes = read(fd, buffer, fileSize);
                if(bytes <=0 ){ //if end of file reached
                    al_destroy(&tokensOfCommand);
                    printf("\n");
                    break;
                }
                buffer[bytes]='\0';
                command = strtok_r(buffer,commandDelimiters,&commandSaveptr); //traverse through shell commands, which is buffer separated by new line
                }
        
        char* word = strtok_r(command, wordDelimiters, &wordSaveptr); //divide each user-inputted shell command into words
       
        int executeConditional = 1; //1 means failure 
            if(word!=NULL && strcmp(word,"then")==0){ //check if the "then" conditional is called
                if(exitStatus==0 && count!=1){ //checks to make sure then is not the very first command called in shell using count!=1
                    executeConditional = 0;
                    word = strtok_r(NULL, wordDelimiters, &wordSaveptr); //divide words by spaces
                    while(word!=NULL){
                    if(strchr(word, '*')!=NULL) { //uses the glob() function to expand wildcards
                        glob_t globResult;
                        int res = glob(word, 0, NULL, &globResult);
                        if (res == GLOB_NOMATCH) {
                            al_push(&tokensOfCommand, word);
                        } else if (res == 0) {
                            for (size_t i = 0; i < globResult.gl_pathc; i++) {
                                al_push(&tokensOfCommand,globResult.gl_pathv[i]); //pushes expanded wildcard arguments back into the tokensOfCommand arraylist                                globResult.gl_pathv[i] = NULL;                                
                            } 
                            globfree(&globResult);
                        } 

                    }else{
                        al_push(&tokensOfCommand,word); //if no wildcards, push the word from command into the tokensOfCommand arraylist
                    }
                    word = strtok_r(NULL, wordDelimiters, &wordSaveptr);
                }
                }else{ //if then is the first command called in the shell, print an error
                    perror("Cannot execute command starting with then");
                    exitStatus = 1;
                }
                count++;
                
            }else if(word!=NULL && strcmp(word,"else")==0){ //checking for commands with "else"
                if(exitStatus==1 && count != 1){ //using count != 1, to check if else is not the very first command called in shell
                    executeConditional = 0;
                    word = strtok_r(NULL, wordDelimiters, &wordSaveptr);
                    while(word!=NULL){
                    if(strchr(word, '*')!=NULL) { //wildcard expansion
                        glob_t globResult;
                        int res = glob(word, 0, NULL, &globResult);
                        if (res == GLOB_NOMATCH) {
                            al_push(&tokensOfCommand, word);
                        } else if (res == 0) {
                            for (size_t i = 0; i < globResult.gl_pathc; i++) {
                                al_push(&tokensOfCommand,globResult.gl_pathv[i]); //pushes the expanded wildcard arguments into tokensOfCommand arraylist
                                globResult.gl_pathv[i] = NULL;                                
                            } 
                            globfree(&globResult);
                        } 

                    }else{
                        al_push(&tokensOfCommand,word); //if no wildcard 
                    }
                    word = strtok_r(NULL, wordDelimiters, &wordSaveptr);
                }
                }else{ //if else is the first 
                    perror("Cannot execute command starting with else");
                    exitStatus = 1;
                }
                count++;
            }else{
                executeConditional = 0;
                while(word!=NULL){
                    if(strchr(word, '*')!=NULL) { //wildcard expansion
                        glob_t globResult;
                        int res = glob(word, 0, NULL, &globResult);
                        if (res == GLOB_NOMATCH) {
                            al_push(&tokensOfCommand, word);
                        } else if (res == 0) {
                            for (size_t i = 0; i < globResult.gl_pathc; i++) {
                                al_push(&tokensOfCommand,globResult.gl_pathv[i]);
                                globResult.gl_pathv[i] = NULL;                                
                            } 
                            globfree(&globResult);
                        } 

                    }else{
                        al_push(&tokensOfCommand,word);
                    }
                    word = strtok_r(NULL, wordDelimiters, &wordSaveptr);
                }
           }           
        if(executeConditional == 0) {
           checkForPipeline(tokensOfCommand);
        }     
        if(mode == 2){ //batch mode
            command = strtok_r(NULL, commandDelimiters,&commandSaveptr);
        }
        //pop all elements from tokensOfCommand arraylist
        char* n;
        while (al_pop(&tokensOfCommand, &n)) {
            continue;
        }
        if(exitStatus == 2){
            command = NULL;
        }
        //free tokensOfCommand arrayyl
        al_destroy(&tokensOfCommand);
    }while((command != NULL));
    free(buffer);
    return;
}
    
// checks batch vs interactive mode 
// prints appropriate statements for interactive mode 
// calls on inputloop to take in user-commands
int main(int argc, char** argv){
    int interactive = 0;
    int fd;
    if (isatty(STDIN_FILENO)&&argc==1) {
        fd=STDIN_FILENO;
        interactive = 1;
    }else{
        //check if /dev/tty is explicitly provided as an argument
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "/dev/tty") == 0) {
                fd = open("/dev/tty", O_RDONLY);
                interactive = 1;
                break;
            }
        } 
    }

    if (interactive==1) {
        printf("Welcome to my shell!\n");
        inputLoop(fd,1);
        printf("Exiting my shell.\n");
        exit(EXIT_SUCCESS);
    }else{
        if(argc==2){
            fd = open(argv[1],O_RDONLY);
            inputLoop(fd,2);
        }else{
            inputLoop(0,2);
        }
        exit(EXIT_SUCCESS);
    }
    return 0; 
}