//Name: Ashish M. Agarwal
//UT EID: ama7677

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

char error_message[30] = "An error has occurred\n";

struct charArr{
    char *arr[10000];
    int size;
};

struct charArr *path = NULL;


struct charArr* stringSplitter(char *str, char c){
    struct charArr* splitString = (struct charArr*)malloc(sizeof(struct charArr));
    if(splitString == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    splitString->size = 0;
    char sep[2];
    sep[0] = c;
    sep[1] = (char) 0;

    char* next;
    while( (next = strtok( str, sep )) )
    {
      str = NULL;
      splitString->arr[ splitString->size++ ] = next;
    }

    return splitString;
}

 struct charArr* removeWhiteSpaces(char *str){
    struct charArr* splitString = (struct charArr*)malloc(sizeof(struct charArr));
    if(splitString == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    int flag = 0;

    for(int i=0; str[i]; i++){
        if(str[i] == ' ' || str[i] == '\t' || str[i] == '\n'){
            flag = 1;
        }
    }

    if(flag == 0){
        splitString->arr[0] = &str[0];
        splitString->size = 0;
        return splitString;
    }

    splitString->size = 0;
    int i=0;
    while(str[i] == ' ' || str[i] == '\t' || str[i] == '\n'){
        i++;
    }

    if(str[i] == '\0'){
        return splitString;
    }

    int startIndex = i;


    while(str[i] != '\0'){
        if(str[i] == ' ' || str[i] == '\t' || str[i] == '\n'){
            str[i] = '\0';
            i++;
            splitString->arr[splitString->size] = &str[startIndex];
            splitString->size++;

            while(str[i] == ' ' || str[i] == '\t' || str[i] == '\n'){
                i++;
            }

            if(str[i] == '\0'){
                return splitString;
            }

            startIndex = i;
        }else{
            i++;
        }
    }
    return splitString;
}


void executeFromChild(char * fullPath, struct charArr* splitString){
    pid_t childPID = fork();
    if(childPID < 0){
        write(STDERR_FILENO, error_message, strlen(error_message));
    }else if(childPID == 0){
        if(execv(fullPath, splitString->arr)){
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        exit(0);
    }else{
        wait(NULL);
    }
}


void executeCommandFromPath(struct charArr* splitString){


    char* command = splitString->arr[0];

    
    //splitString->arr[splitString->size] = NULL;

    //(splitString->size)++;


    char fullPath[200000];

    for(int i=0; i<path->size; i++){
        strcpy(fullPath, path->arr[i]);
        strcat(fullPath, "/");
        strcat(fullPath, command);

        if(access(fullPath, X_OK) == 0){
        executeFromChild(fullPath, splitString);
        return;
        }
    }
    

    //if executable file was not found on any path
    write(STDERR_FILENO, error_message, strlen(error_message));
    return;
}


void executeCommand(struct charArr* splitString){
    //exit command implementation
    if(strcmp(splitString->arr[0], "exit") == 0){
        if(splitString->size > 1){
            write(STDERR_FILENO, error_message, strlen(error_message));
        }else{
            exit(0); //else exit with code 0
        }
    //cd command implementation
    }else if(strcmp(splitString->arr[0], "cd") == 0){
        if(splitString->size != 2){
                write(STDERR_FILENO, error_message, strlen(error_message));
        }else{
            char *dir = splitString->arr[1];
            if(chdir(dir) == -1){
                    write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
    //path command implementation
    }else if(strcmp(splitString->arr[0], "path") == 0){
        if(splitString->size == 1){
            char *p = "";
            path->arr[0] = p;
            path->size = 0;
        }else{
            path->size = 0;
            for(int i=1; i<splitString->size; i++){
                char *p = splitString->arr[i];
                path->arr[i-1] = p;
                (path->size)++;
            }
        }

    }else{  //execv commands here

        executeCommandFromPath(splitString);
    }
    return;
}

void executeRedirectFromChild(char * fullPath, struct charArr* splitString, char *fileName){
    pid_t childPID = fork();
    if(childPID < 0){
        write(STDERR_FILENO, error_message, strlen(error_message));
    }else if(childPID == 0){
        int fd = open(fileName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

        dup2(fd, 1);
        dup2(fd, 2); 
        close(fd);
        
        if(execv(fullPath, splitString->arr)){
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        exit(0);
    }else{
        wait(NULL);
    }
}


void executeRedirectCommand(struct charArr *splitStringCommand, struct charArr *splitStringFileName){
    char* command = splitStringCommand->arr[0];

    
    //splitString->arr[splitString->size] = NULL;

    //(splitString->size)++;


    char fullPath[200000];

    for(int i=0; i<path->size; i++){
        strcpy(fullPath, path->arr[i]);
        strcat(fullPath, "/");
        strcat(fullPath, command);

        if(access(fullPath, X_OK) == 0){
            executeRedirectFromChild(fullPath, splitStringCommand, splitStringFileName->arr[0]);
            return;
        }
    }
    

    //if executable file was not found on any path
    write(STDERR_FILENO, error_message, strlen(error_message));
    return;
}

void redirectionCheck(char *str){

    int flag = 0;
    int countRedirectors = 0;
    for(int i=0; str[i]; i++){
        if(str[i] == '>'){
            flag = 1;
            countRedirectors++;
            i++;
            if(str[i] == '\0' || str[i] == '\n'){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return;
            }
        }
    }
    if(countRedirectors > 1){
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    struct charArr* splitString = stringSplitter(str, '>');
    if(splitString->size > 1){
        //multiple redirects means errors
        if(splitString->size > 2){
            write(STDERR_FILENO, error_message, strlen(error_message));
            return;
        }else{

            struct charArr* splitStringCommand = removeWhiteSpaces(splitString->arr[0]);

            struct charArr* splitStringFileName = removeWhiteSpaces(splitString->arr[1]);

            
            //multiple files means error
            if(splitStringFileName->size != 1){
                write(STDERR_FILENO, error_message, strlen(error_message));
            }else{
                executeRedirectCommand(splitStringCommand, splitStringFileName);
            }
        }
    }else{
        //error if redirect operator present but no filename
        if(flag == 1){
            write(STDERR_FILENO, error_message, strlen(error_message));
            return;
        }else{
            struct charArr* splitString2 = removeWhiteSpaces(splitString->arr[0]);

            executeCommand(splitString2);
        }
        
    }
    return;
 }

int main(int argc, char *argv[]){


    //set path
    path = (struct charArr*)malloc(sizeof(struct charArr));
    if(path == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    path->arr[0] = "/bin";
    path->size = 1;

    //if wish is opened with more than 1 file
    if(argc > 2){
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    

    //running wish in interactive mode (i.e; when a batch file is not specified)
    if(argc == 1){
        
        //buffer to store lines
        size_t buffer_size = 128;
        char *buffer = malloc(buffer_size * sizeof(char));
        if(buffer == NULL){
            fprintf(stderr, "malloc failed\n");
            exit(1);
        }

        //loop continuously till user enters exit command
        while(1){
            printf("wish> ");
            getline(&buffer, &buffer_size, stdin);

            char *str = strdup(buffer);

            //don't close shell on hitting return/enter key and take in next line
            if(strcmp(str, "\n") == 0){
                continue;
            }

            int flag = 0;

            for(int i=0; str[i]; i++){
                if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '&' && str[i] != '>'){
                    flag = 1;
                }
            }

            if(flag == 0){
                return 0;
            }

            //first split for parallel commands i.e; on &
            struct charArr* splitString = stringSplitter(str, '&');




            //loop over individual commands
            for(int i=0; i<splitString->size; i++){
                    redirectionCheck(splitString->arr[i]);
            }

        }
        return 0;

    //batch mode
    }else{
        FILE *fi = fopen(argv[1], "r");
        if(fi == NULL){
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }

        size_t buffer_size = 128;
        char *buffer = malloc(buffer_size * sizeof(char));

        if(buffer == NULL){
            fprintf(stderr, "malloc failed\n");
        }


        //execute till end of file
        while(getline(&buffer, &buffer_size, fi) != -1){

            char *str = strdup(buffer);

            //don't close shell on encountering new line
            if(strcmp(str, "\n") == 0){
                continue;
            }

            int flag = 0;

            for(int i=0; str[i]; i++){
                if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '&' && str[i] != '>'){
                    flag = 1;
                }
            }

            if(flag == 0){
                return 0;
            }

            //first split for parallel commands i.e; on &
            struct charArr* splitString = stringSplitter(str, '&');



            //loop over individual commands
            for(int i=0; i<splitString->size; i++){
                redirectionCheck(splitString->arr[i]);
            }
        }

    }
    return 0;
}