#include <unistd.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/wait.h>
#include <time.h> 
#include <stdbool.h>
#include <sys/types.h>
#define MAX_INPUT_SIZE 256 
#define MAX_COMMAND_LENGTH 100

// File Descriptors
int terminal = STDOUT_FILENO; // Sending arguments to terminal
int fd_input = STDIN_FILENO;  // Getting arguments
int status;
// Input variable
char input[MAX_INPUT_SIZE];
int bytesRead;

char waitingPrompt[MAX_INPUT_SIZE] = "";
char exitSucesss[] = "\nEnd of ShellENSEA\nBye bye...\n";

struct timespec starttime, endtime;

//Time control
int exit_signal_status;
double time_elapsed;
void shellDisplay(void) {
    //Informational Messages
    char welcomeMessage[] = "Welcome to ShellENSEA! \nType 'exit' to quit\n";
    
    //Displaying
    write(terminal,welcomeMessage,strlen(welcomeMessage));

}

void command(char input[], int bytesRead){
    if(strcmp(input,"exit") == 0 || bytesRead == 0){   //Enter in if exit or ctrl+d
        write(terminal,exitSucesss,sizeof(exitSucesss));
        exit(EXIT_SUCCESS);

    }
 pid_t pid = fork();
       

    if (pid <= -1) {
        close(fd_input);
        close(terminal);
        exit(EXIT_FAILURE);

    } else if (pid == 0) { // Child code
        char *token = strtok(input, " "); //Splitting the input according the " " separator
        char *args[MAX_INPUT_SIZE];

        int index = 0;
        while (token != NULL){
            args[index++] = token;
            token = strtok(NULL, " "); //Moving to the next value
        }
        args[index] = NULL;

        execvp(args[0], args); //Execution of every arguments

        close(fd_input);
        close(terminal);
        exit(EXIT_SUCCESS);

    } else {
        wait(&status);
        

    }
}
void return_code(void){

    int sprintfvalue;

    //Time conversion takes into account seconds and nanoseconds
    time_elapsed = (endtime.tv_sec-starttime.tv_sec)*1000+(endtime.tv_nsec-starttime.tv_nsec)/1e6;
    
    //Return code for exit and signal
    if (WIFEXITED(status)){
         exit_signal_status = WEXITSTATUS(status);
        sprintfvalue = sprintf(waitingPrompt, "enseash [exit:%d|%.0f ms] %% ",exit_signal_status,time_elapsed);
    }
    else if(WIFSIGNALED(status)){
        exit_signal_status = WTERMSIG(status);
        sprintfvalue = sprintf(waitingPrompt, "enseash [sign:%d|%.0f ms] %% ",exit_signal_status, time_elapsed);
    }
    
}
void redirections_func(char* command) {
    char* input_file = NULL;
    char* output_file = NULL;

    char* token = strtok(command, " ");
    int i = 0;
    char* args[MAX_COMMAND_LENGTH];

    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
             input_file = token; // the next token will be considered as input file
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            output_file = token; // the next token will be considered as output file
        } else {
            args[i++] = token; // Collect the arguments + command
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
      if (input_file != NULL) {
        int in = open(input_file, O_RDONLY);
        dup2(in, STDIN_FILENO);
        close(in);
    }
    if (output_file != NULL) {
        int out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(out, STDOUT_FILENO);
        close(out);
    }
    execvp(args[0], args);	// EXecute the first argument (command) + other args (Arguments)
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    shellDisplay(); 
    while (1) {
        return_code(); 
        write(terminal, waitingPrompt, sizeof(waitingPrompt) - 1); // Affiche le prompt

        
        bytesRead = read(fd_input, input, sizeof(input));
        input[bytesRead - 1] = '\0'; 
        clock_gettime(CLOCK_MONOTONIC, &starttime); 

        // Détection et gestion des redirections
        if (strstr(input, "<") != NULL || strstr(input, ">") != NULL) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("Erreur de fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                redirections_func(input);
                exit(EXIT_FAILURE); // Si execvp échoue
            } else {
                int status;
                waitpid(pid, &status, 0); // Attente de la fin du processus enfant
                return_code();
            }
        } else {            // Exécution normale des commandes
            command(input, bytesRead);
        }

        clock_gettime(CLOCK_MONOTONIC, &endtime); 
    }

    return EXIT_SUCCESS;
}













