/*******************************************************************************
 * Name        : pfind.c
 * Author      : Kaiqi Chee
 * Date        : 03/16/2021
 * Description : Uses pfind to find files with the given permissions.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>

void usage_message() {
	printf ("%s", "Usage: ./pfind -d <directory> -p <permissions string> [-h]\n");
}

bool check_perm(char *perm){
	if(strlen(perm) != 9){
		return false;
	}
	for (int i=0; i<strlen(perm); i++){
		if (i%3==0){
			if(perm[i] != 'r' && perm[i] != '-'){
				return false;
			}
		}
		else if (i%3==1){
			if(perm[i] != 'w' && perm[i] != '-'){
				return false;
			}
		}
		else{
			if(perm[i] != 'x' && perm[i] != '-'){
				return false;
			}
		}
	}
	return true;
}

bool permMatch(struct stat sb, char* perm){
	char permOfFile[9] = "000000000";
	if ((sb.st_mode & S_IRUSR) != 0){
		permOfFile[0]='1';
	}
	if ((sb.st_mode & S_IWUSR) != 0){
		permOfFile[1]='1';
	}
	if ((sb.st_mode & S_IXUSR) != 0){
		permOfFile[2]='1';
	}
	if ((sb.st_mode & S_IRGRP) != 0){
		permOfFile[3]='1';
	}
	if ((sb.st_mode & S_IWGRP) != 0){
		permOfFile[4]='1';
	}
	if ((sb.st_mode & S_IXGRP) != 0){
		permOfFile[5]='1';
	}
	if ((sb.st_mode & S_IROTH) != 0){
		permOfFile[6]='1';
	}
	if ((sb.st_mode & S_IWOTH) != 0){
		permOfFile[7]='1';
	}
	if ((sb.st_mode & S_IXOTH) != 0){
		permOfFile[8]='1';
	}
	if (strcmp(permOfFile, perm) == 0){
		return true;
	}
	return false;
}

//convert perm to binary representation
void convert(char *perm){
	for (int i=0; i<strlen(perm); i++){
		if (perm[i] != '-'){
			perm[i]='1';
		}
		else{
			perm[i]='0';
		}
	}
}

int findFiles(char* dir_given, char* perm){
	char path[PATH_MAX+1];
	if (realpath(dir_given, path) == NULL) {
		fprintf(stderr, "Error: Cannot get full path of file '%s'. %s\n",
				dir_given, strerror(errno));
		return EXIT_FAILURE;
	}
	DIR *dir;
	if ((dir=opendir(path)) == NULL){
		fprintf(stderr, "Error: Cannot open directory '%s'. %s.\n",
				path, strerror(errno));
		return EXIT_FAILURE;
	}

	struct dirent *entry;
	struct stat sb;
	char full_filename[PATH_MAX+1];
	size_t pathlen = 0;

	if (strcmp(path, "/")) {
		strncpy(full_filename, path, PATH_MAX+1);
	}

	pathlen = strlen(full_filename) + 1;
	full_filename[pathlen - 1] = '/';
	full_filename[pathlen] = '\0';

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0
				|| strcmp(entry->d_name, "..") == 0) {
			continue;
		}
		strncpy(full_filename + pathlen, entry->d_name, PATH_MAX+1 - pathlen);

		if (lstat(full_filename, &sb) < 0) {
			fprintf(stderr, "Error: Cannot stat '%s'. %s\n", full_filename,
					strerror(errno));
			continue;
		}

		if(permMatch(sb, perm)){
			printf("%s\n", full_filename);
		}
		if (entry->d_type == DT_DIR) {
			findFiles(full_filename, perm);
		}

	}
	closedir(dir);
	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
	int dflag=0;
	int pflag=0;
	int flag;
	opterr=0;
	if (argc==1){
		usage_message();
		return EXIT_FAILURE;
	}
	while((flag=getopt(argc, argv, ":d:p:h")) != -1){
		switch(flag){
		case 'd':
			dflag=1;
			break;
		case 'p':
			pflag=1;
			break;
		case 'h':
			usage_message();
			return EXIT_SUCCESS;
		case '?':
			printf("Error: Unknown option '-%c' received.\n", optopt);
			return EXIT_FAILURE;
		}
	}

	//dflag not found
	if (dflag == 0){
		printf("Error: Required argument -d <directory> not found.\n");
		return EXIT_FAILURE;
	}

	//pflag not found
	if (pflag == 0){
		printf("Error: Required argument -p <permissions string> not found.\n");
		return EXIT_FAILURE;
	}

	char* dir_given=argv[2];
	char* perm=argv[4];

	//if directory is invalid
	struct stat sb_given;
	if(stat(dir_given,&sb_given)<0){
		fprintf(stderr, "Error: Cannot stat '%s'. %s.\n",
				dir_given, strerror(errno));
		return EXIT_FAILURE;
	}

	if (check_perm(perm)==false){
		printf("Error: Permissions string '%s' is invalid.\n", perm);
		return EXIT_FAILURE;
	}

	//convert perm to binary representation
	 convert(perm);
	 findFiles(dir_given, perm);
}
