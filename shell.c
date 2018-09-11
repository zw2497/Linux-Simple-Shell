#include <stdio.h>
#include "string.h"
#include <errno.h>
#include <stdlib.h>

extern int errno;

void inputStdin (char **input, size_t *n) {
    if (getline(input , n, stdin) == -1) {
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void tokenize(char *input, char **argument, int size) {
    const char *delimiter = " ";
    argument[0] = strtok(input, delimiter);
    int i = 1;
    while (argument[i] != NULL) {
        argument[i] = strtok(NULL, delimiter);
        i++;

        if (i >= size) {
            size = size * 2;
            argument = realloc(argument, size * sizeof(char*));
            if (!argument) {
                fprintf(stderr, "error: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    }

}


int main(void) {

    char **originStr;
    size_t size = 100;

    char **args;
    int argNumber = 2;
    args = malloc(argNumber * sizeof(char*));

    while (1) {
        printf("$");
        inputStdin(originStr, &size);
        printf("%s", *originStr);
        tokenize(*originStr, args, argNumber);
        printf("%s\n", args[2]);
    }
}
