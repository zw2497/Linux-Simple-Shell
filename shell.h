//
// Created by w4118 on 9/15/18.
//

#ifndef F18_HMWK1_ZW2497_SHELL_H
#define F18_HMWK1_ZW2497_SHELL_H

#endif //F18_HMWK1_ZW2497_SHELL_H

#include <stdbool.h>
void runWithFork(char **args);
void runNoFork(char **args);
bool pipeDetect(char *origin);
char *pipeAddBlank(char *origin);
void pipeProcess(char **args);
void run(char * origin, char** args);
char *bangProcess(char **);
char *bangProcessNoDel(char **);
bool bangDetected(char **);
void exePipe(int *file, char **args, int *a, int i1, int size, int pipenumber,
	     int i2);

#define HISTORY_SIZE 101
#define ARG_NUMBER 10