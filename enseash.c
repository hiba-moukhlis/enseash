#include <unistd.h>  
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/wait.h>

#define MAX_INPUT_SIZE 256

// File Descriptors
int terminal = STDOUT_FILENO; // Sending arguments to terminal
int fd_input = STDIN_FILENO;  // Getting arguments
int status;
// Input variable
char input[MAX_INPUT_SIZE];
int bytesRead;
char waitingPrompt[] = "enseash % ";

// Function to display the welcome message and prompt
void shellDisplay(void) {
    // Informational Messages
    char welcomeMessage[] = "Welcome to ShellENSEA! \nType 'exit' to quit\n";
    // Displaying
    write(terminal, welcomeMessage, strlen(welcomeMessage));
}
// Function to execute the command entered by the user
void command(char input[]) {
    pid_t pid = fork();

    if (pid <= -1) {
        close(fd_input);
        close(terminal);
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Child code
        execlp(input, input, NULL);
        close(fd_input);
        close(terminal);   
        exit(EXIT_SUCCESS);
    } else { // Parent code
        wait(&status);
        // Check if the command exited normally or by signal
        if (WIFEXITED(status)) {
            int exitCode = WEXITSTATUS(status);
            char exitMessage[50];
            snprintf(exitMessage, sizeof(exitMessage), "[exit:%d] ", exitCode);
            write(terminal, exitMessage, strlen(exitMessage));
        } else if (WIFSIGNALED(status)) {
             int signalCode = WTERMSIG(status);
            char signalMessage[50];
            snprintf(signalMessage, sizeof(signalMessage), "[sign:%d] ", signalCode);
            write(terminal, signalMessage, strlen(signalMessage));
        }
    }
}int main(int argc, char **argv) {
    shellDisplay();
    
    while (1) {
        write(terminal, waitingPrompt, sizeof(waitingPrompt)-1);
        
        bytesRead = read(fd_input, input, sizeof(input));
        
        // Si EOF (Ctrl+D) ou erreur de lecture, quitter le shell
        if (bytesRead == 0) {
            write(terminal, "Bye bye...\n", 11);
            break;
        }
         // Removing the '\n' at the end
        input[bytesRead-1] = '\0';

        // If the user enters "exit", we break the loop and end the program
        if (strcmp(input, "exit") == 0) {
            write(terminal, "Bye bye...\n", 11); // Optional: Goodbye message
            break;
        }

        // Execute the command
        command(input);
    }

    close(terminal);
    close(fd_input);
    return EXIT_SUCCESS;
}




