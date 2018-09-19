//
// Created by w4118 on 9/15/18.
//

#ifndef F18_HMWK1_ZW2497_SHELL_H
#define F18_HMWK1_ZW2497_SHELL_H

#endif //F18_HMWK1_ZW2497_SHELL_H

#include <stdbool.h>
void runWithFork();
void runNoFork();
bool pipeDetect();
int pipeAddBlank();
void pipeProcess();
void run();
void exePipe(int *file, char **args, int *a, int i1, int size, int pipenumber,
	     int i2);

#define HISTORY_SIZE 101
#define ARG_NUMBER 101
#define EACH_ARG_NUMBER 101
#define ORIGIN_SIZE 101