/*
 * SPDX-License-Identifier: 0BSD
 * Shell.c
 * Shell
 *
 * Created by Zhicheng WU on 9/10/2018
 * Copyright 2018 Zhicheng Wu.
 * All rights reserved
 */

#include <stdio.h>
#include "string.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

char* concat(const char *s1, const char *s2)
{
	char *result = malloc(strlen(s1) + strlen(s2) + 1);
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

void inputStdin(char **input, size_t *n)
{
	if (getline(input, n, stdin) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void tokenize(char *input, char **argument, int size)
{
	const char *delimiter = "\n ";
	int i = 0;

	argument[0] = strtok(input, delimiter);
	while (argument[i] != NULL) {
		i++;
		argument[i] = strtok(NULL, delimiter);
		if (i > size - 2) {
			size = size * 2;
			argument = realloc(argument, size * sizeof(char *));
		}
	}


}

void exeBuildin(char **args)
{
	int pid;
	int status;


	pid = fork();
	switch (pid) {
	case 0:
		if (execv(args[0], args) == -1) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
		break;
	case -1:
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
		break;
	default:
		wait(&status);
	}


}

void buildin(char **args, char **origin)
{
	if (strcmp(args[0], "exit") == 0) {
		free(args);
		free(origin);
		exit(EXIT_SUCCESS);
	} else if (strcmp(args[0], "cd") == 0) {
		if (chdir(args[1]) == -1) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else {
		exeBuildin(args);
	}

}


int main(void)
{
	char **originStr;
	char **args;
	int argNumber = 10;

	originStr = malloc(argNumber * sizeof(char *));
	args = malloc(argNumber * sizeof(char *));
	size_t size = 100;

	while (1) {
		printf("$");
		inputStdin(originStr, &size);
		tokenize(*originStr, args, argNumber);
		buildin(args, originStr);

	}
}
