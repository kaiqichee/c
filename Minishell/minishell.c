/*******************************************************************************
 * Name        : minishell.c
 * Author      : Kaiqi Chee
 * Date        : 04/14/2021
 * Description : Create a mini shell.
 * Pledge	: I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFSIZE 128
#define BRIGHTBLUE "\x1b[34;1m"
#define DEFAULT "\x1b[0m"

volatile sig_atomic_t interrupted = 0;

void catch_signal(int sig) {
	printf("\n");
	interrupted = sig;
}

int arrayLen(char **array) {
	int length = 0;
	while (array[length] != NULL) {
		printf("array: %s\n", array[length]);
		length++;
		printf("length:%d\n", length);
	}
	return length;
}

int splitString(char **split, char *string) {
	char temp[BUFSIZE];
	memset(temp, '\0', BUFSIZE);
	int count = 0;
	for (int i = 0; i < strlen(string); i++) {
		if (string[i] == ' ' || string[i] == '\n') {
			if(i==0 && string[i]!=' '){
				if ((split[count] = (char*) malloc((BUFSIZE * sizeof(char)) + 1))==NULL){
					fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
					return EXIT_FAILURE;
				}
				strcpy(split[count], temp);
				memset(temp, '\0', BUFSIZE);
				count++;
			}
			if (i > 0 && string[i - 1] != ' ') {
				if ((split[count] = (char*) malloc((BUFSIZE * sizeof(char)) + 1))==NULL){
					fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
					return EXIT_FAILURE;
				}
				strcpy(split[count], temp);
				memset(temp, '\0', BUFSIZE);
				count++;
			}
		}
		else {
			strncat(temp, &(string[i]), 1);
		}
	}
	return count;
}

void freeSplit(char **split, int count) {
	if (split) {
		for (int i = 0; i < count; i++) {
			free(split[i]);
		}
		free(split);
	}
}

void changeDir(char *dir) {
	if (dir == NULL) {
		uid_t id;
		id = getuid();
		struct passwd *pwd;
		if ((pwd = getpwuid(id))==NULL){
			fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
		}
		if (chdir(pwd->pw_dir) == -1){
			fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", pwd->pw_dir, strerror(errno));
		}
	} else {
		if (chdir(dir) == -1){
			fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", dir, strerror(errno));
		}
	}
}

int extraCdSpace(char *string){
	char path[BUFSIZE];
	memset(path, '\0', BUFSIZE);
	int count=0;
	for (int i=1;i<strlen(string);i++){
		if(string[i]!='"'){
			path[count]=string[i];
			count++;
		}
	}
	if(strcmp(path,"~")==0){
		changeDir(NULL);
	}
	else{
	changeDir(path);
	}
	return 0;
}

int extraTilda(char *string){
	char path[BUFSIZE];
	memset(path, '\0', BUFSIZE);
	int count=0;
	for(int i=2; i<strlen(string);i++){
		if(string[i]!='"'){
		path[count]=string[i];
		count++;}
	}
	changeDir(NULL);
	changeDir(path);
	return 1;
}

int extraTilda2(char *string){
	char path[BUFSIZE];
	memset(path, '\0', BUFSIZE);
	int count=0;
	int check=0;
	for(int i=3; i<strlen(string)-1;i++){
		if(string[i]=='"'){
			check++;
		}
		if(string[i]!='"'){
			path[count]=string[i];
			count++;
		}
	}
	if(check%2!=0){
		fprintf(stderr, "Error: Malformed command.\n");
		return -1;
	}
	changeDir(NULL);
	changeDir(path);
	return 1;
}

int putBack(char **array, int count){
	int check=0;
	char path[BUFSIZE];
	memset(path, '\0', BUFSIZE);
	int c=0;
	for (int i=1; i<count; i++){
		for (int j=0;j<strlen(array[i]);j++){
			path[c]=array[i][j];
			c++;
		}
		if(i!=count-1){
			path[c]=' ';
			c++;
		}
	}
	if (path[1]=='~'){
		extraTilda2(path);
		return 0;
	}
	for (int i=0; i<strlen(path);i++){
		if(path[i]=='"'){
			check++;
		}
	}
	if(check%2==0 && (path[0]=='"' && path[strlen(path)-1]=='"')){
		extraCdSpace(path);
	}
	else {
		fprintf(stderr, "Error: Malformed command.\n");
		return -1;
	}
	return 0;
}

void wrap(char **array, int count){
	char temp[BUFSIZE];
	memset(temp, '\0', BUFSIZE);
	temp[0]='"';
	for(int i=0; i<strlen(array[1]);i++){
		strncat(temp,&(array[1][i]),1);
	}
	strcpy(array[1],temp);
	strncat(array[count-1],temp,1);
}

bool quotes(char *string){
	for (int i=0; i<strlen(string);i++){
		if(string[i]=='"'){
			return true;
		}
	}
	return false;
}

int main(int argc, char **argv) {
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = catch_signal;
	if (sigaction(SIGINT, &action, NULL) == -1) {
		fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
		return EXIT_FAILURE;
	}
	while (true) {
		if (!interrupted){

			char buf[BUFSIZE];
			if (getcwd(buf, sizeof(buf)) == NULL) {
				fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
				return EXIT_FAILURE;
			}
			fflush(stdout);
			printf("[");
			printf("%s%s", BRIGHTBLUE, buf);
			printf("%s]$ ", DEFAULT);
			fflush(stdout);

			//read
			char buf2[BUFSIZE];
			memset(buf2, '\0', BUFSIZE);
			int bytes=0;
			if ((bytes=read(STDIN_FILENO, buf2, BUFSIZE-1))==-1 && !interrupted){
				fprintf(stderr, "Error: Failed to read from stdin. %s.\n", strerror(errno));
				return EXIT_FAILURE;
			}
			if(bytes==1 && !interrupted){
				continue;
			}
			if (interrupted==SIGINT){
				interrupted=0;
				fflush(stdout);
				continue;
			}
			char **split;
			if ((split = (char**) malloc((2048 * sizeof(char*))+1))==NULL){
				fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
				return EXIT_FAILURE;
			}
			memset(split, '\0', 2048 * sizeof(char*));
			int count = splitString(split, buf2);

			//cd (change directory)
			if (count>=1 && strcmp(split[0], "cd") == 0) {
				if(count >= 2 && (split[1][0]=='"' || quotes(buf2)) ){
					wrap(split, count);
					putBack(split, count);
				}
				else if (count == 2 && strcmp(split[1], "~") == 0) {
					changeDir(NULL);
				}
				else if(count >= 2 && split[1][0]=='~'){
					if(strcmp(split[1], "~") == 0 && quotes(split[1])==false){
						changeDir(NULL);
					}
					else{
						extraTilda(split[1]);
					}
				}
				else if (count == 1) {
					changeDir(NULL);
				}
				else if (count == 2) {
					changeDir(split[1]);
				}
				else{
					printf("Error: Too many arguments to cd.\n");
				}
			}

			//exit (stops the loop)
			else if (count>=1 && strcmp(split[0], "exit") == 0) {
				freeSplit(split, count);
				return EXIT_SUCCESS;
			}

			//other commands (use fork/exec)
			else if (count>=1) {
				pid_t pid;
				if ((pid = fork()) < 0) {
					fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
					freeSplit(split, count);
					return EXIT_FAILURE;
				}
				else if (pid == 0) {
					if (count == 1) {
						if (execlp(split[0], split[0], NULL) == -1) {
							fprintf(stderr, "Error: exec() failed. %s.\n",
									strerror(errno));
							freeSplit(split, count);
							exit(EXIT_FAILURE);
						}
					}
					else {
						if (execvp(split[0], split) == -1) {
							fprintf(stderr, "Error: exec() failed. %s.\n",
									strerror(errno));
							freeSplit(split, count);
							exit(EXIT_FAILURE);
						}
					}
				}
				else {
					int status;
					if(waitpid(pid, &status, 0)==-1 && !interrupted){
						fprintf(stderr, "Error: wait() failed. %s.\n",
								strerror(errno));
					}
				}
			}

			freeSplit(split, count);
		}
		if (interrupted==SIGINT){
			interrupted=0;
			fflush(stdout);
		}
	}

	return EXIT_SUCCESS;
}

