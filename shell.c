#include <stdio.h>
#include "string.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

void inputStdin (char **input, size_t *n) {
    if (getline(input , n, stdin) == -1) {
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void tokenize(char *input, char **argument, int size) {
    const char *delimiter = " \n";
    int i = 0;
    argument[0] = strtok(input, delimiter);

    while (argument[i] != NULL) {
        i++;
        argument[i] = strtok(NULL, delimiter);
        if (i > size - 2) {
            size = size * 2;
            argument = realloc(argument, size * sizeof(char*));
        }
    }


}

void  exeBuildin(char **args) {
    int pid;
    int status;

    pid = fork ();
    if (pid == 0)
    {
        if (execv(args[0], args) == -1) {
            fprintf(stderr, "error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        /* The fork failed.  Report failure.  */
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (wait(&status) != pid) {
        status = -1;
    }

}


int main(void) {
    extern int errno;
    char **originStr;
    size_t size = 100;

    char **args;
    int argNumber = 10;
    originStr = malloc(argNumber * sizeof(char*));
    args = malloc(argNumber * sizeof(char*));

    int i = 0;

    while (1) {
        printf("$");
        inputStdin(originStr, &size);
        tokenize(*originStr, args, argNumber);
        exeBuildin(args);

        if (i > 5) {
            return 0;
        }
        i++;
    }
}
