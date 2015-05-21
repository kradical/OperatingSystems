#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INITIAL_PATH_SIZE 128
#define INITIAL_COMMAND_BUFFER_SIZE 8

void prompt_user(char **command, char **current_directory);
void parse_command(char *command, char *current_directory);
void process_command(char **commandArray, int *arguements, char *current_directory);
void command_arbitrary(char **commandArray, int *arguements);
void command_change_directory(char **commandArray, int *arguements, char *current_directory);
void command_background_execution(char **commandArray, int *arguements, char *current_directory);

/*
    parse_command takes a string array and places each arguement into one
    element. It does so doubling the buffer size when it fills up and 
    dynamically allocating space for each string which means every arguement
    can be as long as the user likes.
*/
void parse_command(char *command, char *current_directory){ 
    char *token = NULL;
    int command_array_size = INITIAL_COMMAND_BUFFER_SIZE;
    int arguements = 0;
    int i = 0;
    char **command_array = malloc(sizeof(char*) * command_array_size);
    
    if(command_array == NULL){
        perror("Error allocating memory");
        exit(1);
    }

    token = strtok(command, " ");

    while(token != NULL){
        
        if(arguements == command_array_size){
            command_array_size = command_array_size * 2;
            command_array = realloc(command_array, sizeof(char*) * command_array_size);
            
            if(command_array == NULL){
                perror("Error allocating memory");
                exit(1);
            }
        }

        command_array[arguements] = malloc((strlen(token)+1) * sizeof(char));
        
        if(command_array[arguements] == NULL){
            perror("Error allocating memory");
            exit(1);
        }        

        strcpy(command_array[arguements], token);
        
        arguements++;
        token = strtok(NULL, " ");
    }
        
    if(arguements > 0)
       process_command(command_array, &arguements, current_directory);
    
    for(i=0; i<arguements; i++){
        free(command_array[i]);
        command_array[i] = NULL;
    }
    free(command_array);
    command_array = NULL;

    return;
}

/*
    command_arbitrary takes the user provided arguements and searches the path
    for a binary with the same name. If fork() or execv() has an error it will 
    print to stderr.
*/
void command_arbitrary(char **commandArray, int *arguements){       
    char *argv[*arguements];
    int i;
    int status;

    for(i=0;i<*arguements;i++){
        argv[i] = commandArray[i];
    }
    argv[*arguements] = NULL;
    
    int pid = fork();    
    if(pid == 0){
        if(execvp(argv[0], argv) < 0){
            perror("Error on execv");
        }
    }else if(pid < 0){
        perror("Failed to fork process");
    }
    wait(&status);
  
    return;
}

/*
    command_change_directory gives errors for more than 2 arguements, invalid 
    path and invalid HOME variable. Otherwise works on (cd ~ cd . cd .. cd ). 
    Tries absolute path first and if that doesn't work allocates a buffer for 
    new path and concatenates current path with the arguement.
*/
void command_change_directory(char **commandArray, int *arguements, char *current_directory){
    if(*arguements > 2){
        fprintf(stderr, "Too many arguements. cd accepts 1 pathname\n");
    }else if(*arguements == 2){
        if(strcmp(commandArray[1], "~") == 0){
            char *home = getenv("HOME");            
            int result = chdir(home);
            
            if(result == 0){
                return;
            }else if(result == -1){
                fprintf(stderr, "Could not change directory HOME environment variable error\n");
            }
        }else{
            int result = chdir(commandArray[1]);
            
            if(result == 0){
                return;
            }else if(result == -1){
                char *new_directory = malloc(strlen(current_directory)+strlen(commandArray[1]) + 1);
                
                if(new_directory == NULL){
                    perror("Error allocating memory");
                    exit(1);
                }
                
                strcpy(new_directory, current_directory);
                strcat(new_directory, commandArray[1] );               

                result = chdir(new_directory);
                
                free(new_directory);
                new_directory = NULL;
    
                if(result == 0){
                    return;
                }else if(result == -1){
                    fprintf(stderr, "Could not change directory invalid path\n");
                }
            }
        }        
    }else if(*arguements == 1){
        char *home = getenv("HOME");            
        int result = chdir(home);
        if(result == 0){
            return;
        }else if(result == -1){
            fprintf(stderr, "Could not change directory HOME environment variable error\n");
        }
    }
    return;
}

/*
    command_background_execution
*/
void command_background_execution(char **commandArray, int *arguements, char *current_directory){
    
    printf("BACKGROUND EXECUTION");    
    return;
}
/*
    process_command looks the command and arguements and calls the appropriate
    function to deal with it. 
    Shell takes commands:
        ls
*/
void process_command(char **commandArray, int *arguements, char *current_directory){
    char *command = commandArray[0];
    if(strcmp(command, "cd") == 0){
        command_change_directory(commandArray, arguements, current_directory);
    }else if(strcmp(command, "bg") == 0){
        command_background_execution(commandArray, arguements, current_directory);
    }else{
        command_arbitrary(commandArray, arguements);
    }

    return;
}

/*
    prompt_user with readline, dynamically allocates space for current working
    directory.
*/
void prompt_user(char **command, char **current_directory){
    int path_size = INITIAL_PATH_SIZE;
    *current_directory = malloc(sizeof(char) * path_size);
    
    if(*current_directory == NULL){
        perror("Error allocating memory");
        exit(1);
    }
    
    while(getcwd(*current_directory, path_size) == NULL){
            path_size *= 2;
            printf("%d\n", path_size);
            *current_directory = realloc(*current_directory, sizeof(char) * path_size);
            
            if(*current_directory == NULL){
                perror("Error allocating memory");
                exit(1);
            }
    }

    printf("RSI: %s >", *current_directory);
    *command = readline(NULL);                  

    return;
}

int main(void){
    int active = 1;
    char *command = NULL;
    char *current_directory = NULL;

    while(active){
        prompt_user(&command, &current_directory);
        parse_command(command, current_directory);
        free(current_directory);
        current_directory = NULL;
        free(command);
        command = NULL;
    }
        
    printf("\n");
    return 0;
}
