//
// Created by w4118 on 9/15/18.
//

#ifndef F18_HMWK1_ZW2497_SHELL_H
#define F18_HMWK1_ZW2497_SHELL_H

#endif //F18_HMWK1_ZW2497_SHELL_H

#include <stdbool.h>
void runwithfork(char **args);
void runnofork(char **args);
bool pipedetect(char *origin);
char *pipeaddblank(char *origin);
void pipeProcess(char **args);

#define HISTORY_SIZE 101
#define ARG_NUMBER 10