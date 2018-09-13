/*
 * SPDX-License-Identifier: 0BSD
 * Shell.c
 * Shell
 *
 * Created by Zhicheng WU on 9/10/2018
 * Copyright 2018 All rights reserved
 */

#include <stdio.h>
#include "string.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <inttypes.h>
#define HISTORY_SIZE 101
#define ARG_NUMBER 10


typedef struct history{
    int offset;
    char *args;
} memo;

typedef struct list {
    int first;
    int end;
    memo **m;
    int size;
} list;

list *list1;
int i = 0;

void deloneElement()
{
	if (list1->size == 0) {
		return;
	} else {
		list1->size = list1->size - 1;
		list1->end = list1->end - 1;
		if (list1->end < 0) {
			list1->end = list1->end + HISTORY_SIZE;
		}
		i--;
	}

}

void printHistory(int n)
{
	int p;
	if (n > list1->size) {
		n = list1->size;
	} else if (n < 0) {
		n = 0;
	}
	p = (list1->first + (list1->size - n)) % HISTORY_SIZE;
	while (n > 0) {
		printf("%d %s", list1->m[p]->offset, list1->m[p]->args);
		p = (p + 1) % HISTORY_SIZE;
		n --;
	}
}

void deleteHistory()
{
	list1->first = list1->end;
	list1->size = 0;
}


void addToList(int i, list *l, char **argument){

	memo *m = malloc(sizeof(memo));
	m->args = malloc(100 * sizeof(char));
	strcpy(m->args, *argument);
	m->offset = i;


	i = i % HISTORY_SIZE;
	l->m[i] = m;
	l->end = (l->end + 1) % HISTORY_SIZE;
	if (l->first == l->end) {
		l->first = (l->first + 1) % HISTORY_SIZE;
	} else {
		list1->size++;
	}
}



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
	char *temp = malloc(strlen(input) + 1);
	strcpy(temp, input);


	argument[0] = strtok(temp, delimiter);
	while (argument[i] != NULL) {
		i++;
		argument[i] = strtok(NULL, delimiter);
		if (i > size - 2) {
			size = size * 2;
			argument = realloc(argument, size * sizeof(char *));
		}
	}


}

void exe(char **args)
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

void run(char **args, char **origin)
{
	if (strcmp(args[0], "exit") == 0) {
		free(args);
		free(origin);
		exit(EXIT_SUCCESS);
	} else if (strcmp(args[0], "cd") == 0) {
		if (chdir(args[1]) == -1) {
			fprintf(stderr, "error: %s\n", strerror(errno));
		}
	} else if (strcmp(args[0], "history") == 0){
		if (args[1] == NULL) {
			printHistory(list1->size);
		} else if (strcmp(args[1], "-c") == 0) {
			deleteHistory();
		} else {
			uintmax_t num = strtoumax(args[1], NULL, 10);
			if (num == UINTMAX_MAX && errno == ERANGE) {
				fprintf(stderr, "error: %s\n", strerror(errno));
			}
			printHistory(num);
		}
	} else if (strcmp(args[0], "!!") == 0) {
		if (list1->size == 0) {
			fprintf(stderr, "error: No history command\n");
		} else {
			char **arg_new;
			arg_new = malloc(sizeof(char *));
			int p = list1->end;
			p = list1->end - 2;
			if (p < 0) {
				p = HISTORY_SIZE + p;
			}
			deloneElement();
			addToList(i++, list1, &list1->m[p]->args);
			tokenize(list1->m[p]->args, arg_new, ARG_NUMBER);
			run(arg_new, &list1->m[p]);
		}
	} else {
		exe(args);
	}

}


int main(void)
{
	char **originStr;
	char **args;




	originStr = malloc(ARG_NUMBER * sizeof(char *));
	args = malloc(ARG_NUMBER * sizeof(char *));
	size_t size = 100;
	list1 = malloc(sizeof(list));
	list1->m = malloc(HISTORY_SIZE * sizeof(memo));
	list1->first = 0;
	list1->end = 0;


	while (1) {
		printf("$");
		inputStdin(originStr, &size);
		addToList(i++, list1, originStr);
		tokenize(*originStr, args, ARG_NUMBER);
		run(args, originStr);

	}
}
