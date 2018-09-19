/* SPDX-License-Identifier: 0BSD
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
#include <stdbool.h>
#include <ctype.h>
#include <shell.h>

struct memo {
int offset;
char *args;
};

struct list {
int first;
int end;
struct memo **m;
int size;
};

struct list *list1;
int iNum;
char *ori = NULL;
char **arg = NULL;

int listNum(int n)
{
	if (n >= 0)
		n = n % HISTORY_SIZE;
	else
		n = n + HISTORY_SIZE;
	return n;
}

bool isInteger(char *test)
{
	int result;
	int p = 0;

	while (test[p] != '\0') {
		result = isdigit(test[p]);
		if (result == 0)
			break;
		p++;
	}
	return result;
}

bool startsWith(const char *pre, const char *str)
{
	size_t lenpre = strlen(pre);
	size_t lenstr = strlen(str);

	return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

void del1Element(void)
{
	if (list1->size == 0) {
		fprintf(stderr, "error: No history to delete\n");
		return;
	}

	list1->size = list1->size - 1;
	list1->end = list1->end - 1;
	if (list1->end < 0)
		list1->end = list1->end + HISTORY_SIZE;
	iNum--;


}

int printHistory(int n)
{
	int p;

	if (list1->size == 0) {
		fprintf(stderr, "error: No history to print\n");
		return EXIT_FAILURE;
	}

	if (n > list1->size)
		n = list1->size;
	else if (n < 0) {
		fprintf(stderr, "error: invalid number\n");
		n = 0;
	}
	p = (list1->first + (list1->size - n)) % HISTORY_SIZE;
	while (n > 0) {
		printf("%d %s", list1->m[p]->offset, list1->m[p]->args);
		p = (p + 1) % HISTORY_SIZE;
		n--;
	}
}

void deleteHistory(void)
{
	list1->first = list1->end;
	list1->size = 0;
}


void addToList(struct list *l, char *argument)
{
	int listNum;

	listNum = iNum % HISTORY_SIZE;
	strcpy(l->m[listNum]->args, argument);
	l->m[listNum]->offset = iNum;
	l->end = (l->end + 1) % HISTORY_SIZE;
	if (l->first == l->end)
		l->first = (l->first + 1) % HISTORY_SIZE;
	else
		list1->size++;
	iNum++;
}

void freeHistory()
{
	for (int itemp = 0; itemp < HISTORY_SIZE; itemp++) {
		free(list1->m[itemp]->args);
		free(list1->m[itemp]);
	}

	free(list1->m);
	free(list1);
}

void freeOriArg()
{
	free(ori);
	for (int itemp = 0; itemp < ARG_NUMBER; itemp++)
		free(arg[itemp]);
	free(arg);
	arg = NULL;
}

int initOriArg() {
	ori = NULL;
	arg = malloc(ARG_NUMBER * sizeof(char *));
	if (arg == NULL) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	for (int itemp = 0; itemp < ARG_NUMBER; itemp++) {
		arg[itemp] = malloc(EACH_ARG_NUMBER * sizeof(char));
		if (arg[itemp] == NULL) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;

}


char *concat(char *s1, char *s2)
{
	char *result = NULL;

	result = malloc(strlen(s1) + strlen(s2) + 1);
	if (result == NULL) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return NULL;
	}

	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

int inputString(char **input, size_t *n)
{
	if (getline(input, n, stdin) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	if (input[0][0] == '\n')
		return EXIT_FAILURE;
	if (strlen(input[0]) > ORIGIN_SIZE - 1){
		fprintf(stderr, "error: Too long argument \n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int tokenize()
{
	const char *delimiter = "\n ";
	int i = 0;
	char *temp = malloc(strlen(ori) + 1);
	char *temp1;

	if (temp == NULL) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	strcpy(temp, ori);

	temp1 = strtok(temp, delimiter);
	if (temp1 != NULL)
		strcpy(arg[0], temp1);
	else {
		free(arg[0]);
		arg[0] = NULL;
	}
	while (arg[i] != NULL) {
		i++;
		temp1 = strtok(NULL, delimiter);
		if (temp1 != NULL)
			strcpy(arg[i], temp1);
		else {
			free(arg[i]);
			arg[i] = NULL;
		}

		if (i > ARG_NUMBER - 2) {
			fprintf(stderr, "error: too much arguments %s\n",
				strerror(errno));
			free(temp);
			return EXIT_FAILURE;
		}
	}
	free(temp);
	return EXIT_SUCCESS;


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
	case -1:
		fprintf(stderr, "error: %s\n", strerror(errno));
		return;
	default:
		if (waitpid(pid, &status, 0) != pid)
			status = -1;
	}


}

void exeNoFork(char **args)
{
	if (execv(args[0], args) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);

}

void exitProcess()
{
	freeHistory();
	freeOriArg();
	exit(EXIT_SUCCESS);
}

void cdProcess()
{
	if (chdir(arg[1]) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return;
	}

}

int historyProcess()
{
	if (arg[1] == NULL) {
		printHistory(list1->size);
	} else if (strcmp(arg[1], "-c") == 0) {
		deleteHistory();
	} else if (isInteger(arg[1])) {
		uintmax_t num = strtoumax(arg[1], NULL, 10);

		if (num == UINTMAX_MAX && errno == ERANGE) {
			fprintf(stderr, "error: invalid command\n");
			return EXIT_FAILURE;
		}
		printHistory(num);
	} else {
		fprintf(stderr, "error: invalid command\n");
		return EXIT_FAILURE;
	}
}

char *bang2Process()
{
	int num = list1->end - 2;
	int n = list1->size - 1;

	while (n) {
		if (startsWith(&arg[0][1], list1->m[num]->args)) {
			del1Element();
			return list1->m[num]->args;
		}
		num = listNum(num - 1);
		n--;
	}
	fprintf(stderr, "error: No match\n");
	del1Element();
	return NULL;
}

char *bang1Process()
{
	if (list1->size == 1) {
		fprintf(stderr, "error: No history\n");
		del1Element();
		return NULL;
	}
	int p;

	p = list1->end - 2;
	if (p < 0)
		p = HISTORY_SIZE + p;
	del1Element();
	return list1->m[p]->args;
}

char *bang2ProcessNoDel(char *argument)
{
	int num = list1->end - 1;
	int n = list1->size;

	while (n) {
		if (startsWith(&argument[1], list1->m[num]->args))
			return list1->m[num]->args;
		num = listNum(num - 1);
		n--;
	}
	fprintf(stderr, "error: No match\n");
	return NULL;
}

char *bang1ProcessNoDel()
{
	if (list1->size == 0) {
		fprintf(stderr, "error: No history\n");
		return NULL;
	}
	int p;

	p = list1->end - 1;
	if (p < 0)
		p = HISTORY_SIZE + p;
	return list1->m[p]->args;

}

void pipeProcess()
{
	char **former;
	char **latter;
	int p = 0;
	pid_t pid;
	int a[10] = {[0 ... 9] = -1};
	int pipenumber;
	int size = 0;
	int c = 0;
	int number = 0;
	int i1 = 0;
	int i2 = 0;

	while (arg[p] != NULL) {
		if (strcmp(arg[p], "|") == 0) {
			a[c++] = number;
			size++;
			free(arg[p]);
			arg[p] = NULL;
			number = 0;
		}
		number++;
		p++;
	}

	pipenumber = c;
	int myPipe[2 * pipenumber];

	for (int i = 0; i < pipenumber; i++) {
		if (pipe(myPipe + i*2) < 0) {
			perror("couldn't pipe");
			exit(EXIT_FAILURE);
		}
	}

	former = &arg[0];
	latter = &arg[a[i1] + 1];

	if (pipe(myPipe)) {
		fprintf(stderr, "Pipe failed.\n");
		exit(EXIT_FAILURE);
	}
	if (pipe(myPipe + 2)) {
		fprintf(stderr, "Pipe failed.\n");
		exit(EXIT_FAILURE);
	}

	pid = fork();

	if (pid == 0) {
		dup2(myPipe[1], STDOUT_FILENO);
		for (int i = 0; i < 2 * pipenumber; i++)
			close(myPipe[i]);
		exeNoFork(former);
	} else if (pid < (pid_t) 0) {
		fprintf(stderr, "Fork failed.\n");
		exit(EXIT_FAILURE);
	} else {
		exePipe(myPipe, latter, a, i1, size, pipenumber, i2);
	}
}

void exePipe(int *file, char **args, int *a, int i1, int size, int pipenumber,
	     int i2)
{
	int pid = fork();
	int status;

	switch (pid) {
	case 0:
		if (size > 1) {
			dup2(file[i2], STDIN_FILENO);
			dup2(file[i2 + 3], STDOUT_FILENO);
			for (int i = 0; i < 2 * pipenumber; i++)
				close(file[i]);
			if (execv(args[0], args) == -1) {
				fprintf(stderr, "error: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}

		} else {
			dup2(file[i2], STDIN_FILENO);
			for (int i = 0; i < 2 * pipenumber; i++)
				close(file[i]);
			if (execv(args[0], args) == -1) {
				fprintf(stderr, "error: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
		}

	case -1:
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	default:
		if (size > 1) {
			i1++;
			size--;
			i2 = i2 + 2;
			exePipe(file, &args[a[i1]], a, i1, size,
				pipenumber, i2);
		}
		for (int i = 0; i < 2 * pipenumber; i++)
			close(file[i]);
		for (int i = 0; i < pipenumber + 1; i++)
			wait(&status);
	}
}

void runNoFork()
{

	if (strcmp(arg[0], "exit") == 0)
		exitProcess();
	else if (strcmp(arg[0], "cd") == 0) {
		cdProcess(arg);
		exit(EXIT_SUCCESS);
	} else if (strcmp(arg[0], "history") == 0) {
		historyProcess(arg);
		exit(EXIT_SUCCESS);
	} else
		exeNoFork(arg);

}

void runWithFork()
{
	if (strcmp(arg[0], "exit") == 0) {
		exitProcess();
	} else if (strcmp(arg[0], "cd") == 0)
		cdProcess();
	else if (strcmp(arg[0], "history") == 0)
		historyProcess();
	else if (strcmp(arg[0], "!!") == 0) {
		ori = bang1Process();
		if (ori == NULL)
			return;
		addToList(list1, ori);
		tokenize();
		runWithFork();
	} else if (arg[0][0] == '!' && arg[0][1] != '!') {
		ori = bang2Process();
		if (ori == NULL)
			return;
		addToList(list1, ori);
		tokenize();
		runWithFork();
	} else
		exe(arg);
}

bool pipeDetect()
{
	int p1 = 0;

	while (ori[p1] != '\0') {
		if (ori[p1] == '|')
			return true;
		p1++;
	}
	return false;
}

int pipeAddBlank()
{
	int p1 = 0;
	int in1;
	char *split1 = NULL;
	char *split2 = NULL;
	char *temp = NULL;

	if (ori == NULL)
		return EXIT_FAILURE;

	while (ori[p1] != '\0') {
		if (ori[p1] == '|') {
			split1 = malloc((p1 + 4) * sizeof(char));
			split2 = malloc(
				(strlen(ori) - p1 + 1) * sizeof(char));

			for (int i1 = 0; i1 < p1; i1++) {
				split1[i1] = ori[i1];
			}


			split1[p1] = ' ';
			split1[p1 + 1] = '|';
			split1[p1 + 2] = ' ';
			split1[p1 + 3] = '\0';
			in1 = p1 + 1;
			for (int i2 = 0; in1 < strlen(ori) + 1; in1++, i2++)
				split2[i2] = ori[in1];

			temp = concat(split1, split2);
			p1++;
		}
		p1++;
	}
	if (temp != NULL) {
		strcpy(ori, temp);
		free(temp);
		free(split1);
		free(split2);
	}
	return EXIT_SUCCESS;
}

char *alterBangBang(int p)
{
	char *temSum = NULL;
	char *temSum1 = NULL;
	char *b = bang1ProcessNoDel();

	if (b == NULL)
		return NULL;
	char *temp = malloc(strlen(b) + 1);
	char *temp1 = malloc(strlen(ori) + 1);
	char *temp2 = malloc(strlen(ori) + 1);

	temp = strcpy(temp, b);
	temp[strlen(temp) - 1] = '\0';

	if (temp1 == NULL || temp2 == NULL) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return NULL;
	}
	strcpy(temp1, ori);
	strcpy(temp2, ori);

	temp1[p] = '\0';
	temp2 = temp2 + p + 2;

	temSum = concat(temp1, temp);
	temSum1 = concat(temSum, temp2);
	free(temp);
	free(temp1);
	free(temSum);
	free(temp2 - p - 2);
	return temSum1;
}

char *alterBang(int p)
{
	char *temSum = NULL;
	char *temSum1 = NULL;
	char *temp1 = malloc(strlen(ori) + 1);
	char *temp2 = malloc(strlen(ori) + 1);
	char *temp3 = malloc(strlen(ori) + 1);

	if (temp1 == NULL || temp2 == NULL || temp3 == NULL) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	temp1 = strcpy(temp1, ori);
	temp2 = strcpy(temp2, ori);
	temp3 = strcpy(temp3, ori);

	int endPosition = p;

	while (ori[endPosition] != '\0') {
		if (ori[endPosition] == '\n' ||
		ori[endPosition] == ' ' || ori[endPosition] == '|')
			break;
		endPosition++;
	}
	temp3[endPosition] = '\0';
	temp3 = temp3 + p;
	char *b = bang2ProcessNoDel(temp3);

	if (b == NULL)
		return NULL;
	char *temp = malloc(strlen(b) + 1);

	strcpy(temp, b);
	temp[strlen(temp) - 1] = '\0';

	temp1[p] = '\0';
	temp2 = temp2 + endPosition;

	temSum = concat(temp1, temp);
	temSum1 = concat(temSum, temp2);

	free(temp);
	free(temp1);
	free(temp2 - endPosition);
	free(temp3 - p);
	free(temSum);
	return temSum1;
}


int AddHistory()
{
	int p = 0;
	char *temSum = ori;

	while (ori[p] != '\0') {
		if (ori[p] == '!' && ori[p + 1] == '!') {
			temSum = alterBangBang(p);
		}
		if (ori[p] == '!' &&
		(isalpha(ori[p + 1]) || ori[p + 1] == '/')) {
			temSum = alterBang(p);
		}
		p++;
	}
	if (temSum == NULL)
		return EXIT_FAILURE;
	addToList(list1, temSum);
	if (temSum != ori) {
		strcpy(ori, temSum);
		free(temSum);
	}
	return EXIT_SUCCESS;
}

void run()
{
	if (AddHistory(ori) == EXIT_FAILURE)
		return;
	if (pipeDetect()) {
		if(pipeAddBlank() == EXIT_FAILURE)
			return;
		if(tokenize() == EXIT_FAILURE)
			return;
		pipeProcess();
	} else {
		if(tokenize() == EXIT_FAILURE)
			return;
		runWithFork();
	}


}

int init()
{
	list1 = malloc(sizeof(struct list));
	if (list1 == NULL) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	list1->m = malloc(HISTORY_SIZE * sizeof(struct memo*));
	if (list1->m == NULL) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	for (int itemp = 0; itemp < HISTORY_SIZE; itemp++) {
		list1->m[itemp] = malloc(sizeof(struct memo));
		if (list1->m[itemp] == NULL) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}

		list1->m[itemp]->args = malloc(ORIGIN_SIZE);
		if (list1->m[itemp]->args == NULL) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}

		list1->m[itemp]->offset = 0;
	}
	list1->first = 0;
	list1->end = 0;
	list1->size = 0;
	return EXIT_SUCCESS;
}

int main(void)
{
	size_t size = 100;

	while (1) {
		if (init() == EXIT_FAILURE) {
			freeOriArg();
			freeHistory();
			continue;
		}

		while (1) {
			if (initOriArg() == EXIT_FAILURE){
				freeOriArg();
				break;
			}
			printf("$");
			if (inputString(&ori, &size) == EXIT_FAILURE)
				continue;
			run(ori);
			freeOriArg();

		}
	}
}
