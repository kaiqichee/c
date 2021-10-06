/*******************************************************************************
 * Name        : spfind.c
 * Author      : Kaiqi Chee
 * Date        : 03/31/2021
 * Description : Returns sorted pfind
 * Pledge	: I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
	if (argc != 5) {
		fprintf(stderr, "Usage: ./spfind -d <directory> -p <permissions string> [-h]\n");
		return EXIT_FAILURE;
	}
	int pfind_to_sort[2];
	int sort_to_parent[2];
	if(pipe(pfind_to_sort)<0){
		fprintf(stderr, "Error: Failed to create pipe. %s.\n", strerror(errno));
		return EXIT_FAILURE;
	}
	if (pipe(sort_to_parent)<0){
		fprintf(stderr, "Error: Failed to create pipe. %s.\n", strerror(errno));
		return EXIT_FAILURE;
	}
	pid_t pid[2];

	if ((pid[0] = fork()) == 0) {
		close(pfind_to_sort[0]);
		if(dup2(pfind_to_sort[1], STDOUT_FILENO)==-1){
			fprintf(stderr, "Error: dup2 failed. %s.\n", strerror(errno));
			return EXIT_FAILURE;
		}
		close(sort_to_parent[0]);
		close(sort_to_parent[1]);

		if (execv("pfind", argv)==-1){
			fprintf(stderr, "Error: pfind failed.\n");
			return EXIT_FAILURE;
		}
	}

	if ((pid[1] = fork()) == 0) {
		close(pfind_to_sort[1]);
		if(dup2(pfind_to_sort[0], STDIN_FILENO)==-1){
			fprintf(stderr, "Error: dup2 failed. %s.\n", strerror(errno));
			return EXIT_FAILURE;
		}
		close(sort_to_parent[0]);
		if (dup2(sort_to_parent[1], STDOUT_FILENO)==-1){
			fprintf(stderr, "Error: dup2 failed. %s.\n", strerror(errno));
			return EXIT_FAILURE;
		}

		if(execlp("sort", "sort", NULL)==-1){
			fprintf(stderr, "Error: sort failed. %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
	}
	close(sort_to_parent[1]);
	if(dup2(sort_to_parent[0], STDIN_FILENO)==-1){
		fprintf(stderr, "Error: dup2 failed. %s.\n", strerror(errno));
		return EXIT_FAILURE;
	}
	close(pfind_to_sort[0]);
	close(pfind_to_sort[1]);

	int lines=0;
	char buffer[128];
	int count;
	while ((count=read(STDIN_FILENO, buffer, sizeof(buffer)))!=0){
		if (count == -1) {
			fprintf(stderr, "Error: read failed. %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
		write(1, buffer, count);
		for (int i=0; i<count; i++){
			if (buffer[i]=='\n'){
				lines++;
			}
		}
	}

	int status;
	close(sort_to_parent[0]);
	waitpid(pid[0], &status, 0);
	if (WIFEXITED(status)) {
			if(WEXITSTATUS(status)==EXIT_FAILURE){
				return EXIT_FAILURE;
			}
	}
	waitpid(pid[1], &status, 0);
	if (WIFEXITED(status)) {
		if(WEXITSTATUS(status)==EXIT_FAILURE){
			return EXIT_FAILURE;
		}
		else{
			printf("Total matches: %d\n", lines);
		}
	}
	return EXIT_SUCCESS;
}
