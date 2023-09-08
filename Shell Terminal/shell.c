/*
 * 3SH3 Operating Systems Assignment 1, Question 2
 * Benjamin Sun and Mathew Wilker
 * 
 * This program implements the osh shell. A few things to note:
 *  - the command "history" will not show up in the history the first time
 *    rather it will show up the next time history is run. This is to show off
 *    the error handling message for when there is no previous history
 * 
 *  - !! will add the previous command to the history as if you ran the command
 *    itself
 * 
 *
 *  Citations:
 *  https://stackoverflow.com/questions/26597977/split-string-with-multiple-delimiters-using-strtok-in-c
 *  https://stackoverflow.com/questions/27541910/how-to-use-execvp 
 *  https://www.programiz.com/c-programming/library-function/string.h/strcmp
 *  Dr Neerja Mhaskar's Practice Labs and Lecture Notes
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 80 /* Max command length*/

void showHistory(char* input_cmd);
void updateHistory(char* input_cmd);

int history_count, history_length, isFull;
char history[7][MAX_LINE];


int main()
{
    char *args[MAX_LINE/2 + 1];
    int should_run = 1, amp;
    char user_cmd[MAX_LINE/2];
    char *p, *newline;
    char *cmd;

    for (int k=0; k < 7; k++) // Populate history to avoid seg fault later on
    {
        history[k][0] = '\0';
    }

    history_count = 0; // Keeps track of count over lifetime
    history_length = 1; // makes sure history length stays at 5


    while(should_run)
    {
        amp = 0;
        printf("osh>");

        fgets(user_cmd, sizeof(user_cmd), stdin); // Get user input
        int length = 0;

        int test = 0;
        

        if(strstr(user_cmd, "exit")) // Checks for history command
        {
            exit(1);
        }

        while(user_cmd[length] != '\0') length++;

        if(user_cmd[0] == '\n') continue; // Eat up multiple enter presses

        newline = strchr(user_cmd, '\n');
        if (newline) *newline = '\0'; // remove newline character created by fgets()
        

        if(strstr(user_cmd, "!!")) // Checks for !! command
        {
            if (history_length > 1)
                strcpy(user_cmd, history[history_length]);
            else 
            {
                printf("ERROR: No previous command available\n");
                continue;
            }
        }

        if(strstr(user_cmd, "history")) // Checks for history command
        {
            showHistory(user_cmd);
            continue;
        }
    
        updateHistory(user_cmd);

        if(user_cmd[length-2] == '&') // Removes ampersand and tells parent to
        {                             // execute concurrently
            user_cmd[length-2] = '\0';
            amp = 1;
        }

        p = strtok(user_cmd, " \n");
        strcpy(cmd, p); // copy program command
        
        int i = 0;
        while (p != NULL) // Populate arguments array with args
        {
            args[i] = p;
            p = strtok(NULL, " \n");
            i++;
        }

        args[i] = NULL; // null terminate arguments string

        pid_t p2;
        p2 = fork(); // create child process
        
        if (p2 == 0)
        {
            printf("%s \t %s", cmd, *args);
            int status_code = execvp(cmd, args); // execute command and arguments
            if (status_code == -1)
            {
                printf("Process did not terminate correctly\n");
            }
        }
        else if(amp != 1) // if no ampersand in command - wait for child
        {
            wait(NULL);
        }
        fflush(stdout);
    }
    return 0;
}

/*
 * This function simply displays the current history (last 5 commands)
*/
void showHistory(char* input_cmd)
{
    
    int j = history_length, numbering = history_count;
    if (history_length > 1)
    {
        while(history[j][0] != '\0')
        {
            printf("%d. %s\n", numbering, history[j]);

            numbering--;
            j--;
        }
    }
    else
    {
        printf("No Commands in History\n");
    }
    updateHistory(input_cmd);
}

/*
 * This function simply updates the current history, maintaining 5 commands at
 * a time.
*/
void updateHistory(char* input_cmd)
{

    if(history_length < 5) history_length++;
    else isFull = 1;

    if (isFull)
    {
        int i = 1;
        while(i < history_length)
        {
            strcpy(history[i], history[i+1]);
            i++; 
        }
    }

    strcpy(history[history_length], input_cmd);

    history_count++;
}