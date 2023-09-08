/**
 * Write a C program (named labtest1a.c) that corresponds to the above process tree. You are
 * to use the fork() system call to create child processes. Additionally, your program should
 * ensure that the parent processes wait for their child processes to complete. As you can see
 * there are a total of 6 processes (including the parent process). 
 *
 * @group Group 36
 * @author Ben Sun, sunb26
 * @author Matthew Wilker, wilkem1
 * @course SFWRENG 3SH3
 * @date March 7th, 2023
 */

#include <unistd.h>    // Contains the `pid_t` type
#include <stdio.h>     // Used for the `print_f()` function
#include <sys/wait.h>  // Required for the `wait()` system call


int main(int argc, char const *argv[]) {


    pid_t p2, p3;
    printf("%d\n", getpid()); // Print P1 pid
    p2 = fork(); // Create P2

    if (p2 == 0) printf("%d\n", getpid()); // Print P2 pid without P1
    else wait(NULL);
    p3 = fork(); // Create P3 and P4

    if (p3 == 0) printf("%d\n", getpid()); // Print P3 and P4 pid without P2 and P1
    else wait(NULL);


    if (p2 == 0 && p3 > 0) // Enter just P2
    {
        pid_t p4 = fork(); // Create P5

        if (p4 == 0) printf("%d\n", getpid()); // Print just P5 pid
        else wait(NULL);

        if (p4 > 0) // Enter just P2
        {
            pid_t p5 = fork(); // Create P6

            if (p5 == 0) printf("%d\n", getpid()); // Print just P6 pid
            else wait(NULL);
        }
        else
        {

            wait(NULL);
        }
    }
    else
    {
        wait(NULL);
    }

    return 0;
}
