/*******************************************************************************
 * Name        : sort.c
 * Author      : Kaiqi Chee
 * Date        : 03/02/2021
 * Description : Uses quicksort to sort a file of either ints, doubles, or
 *               strings.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quicksort.h"

#define MAX_STRLEN     64 // Not including '\0'
#define MAX_ELEMENTS 1024

typedef enum {
    STRING,
    INT,
    DOUBLE
} elem_t;

void usage_message() {
	printf ("%s", "Usage: ./sort [-i|-d] filename\n");
	printf ("%s", "   -i: Specifies the file contains ints.\n   -d: Specifies the file contains doubles.\n");
	printf("%s", "   filename: The file to sort.\n   No flags defaults to sorting strings.\n");
}

/**
 * Found on Canvas: Assignments: QuickSelect in C
 * Reads data from filename into an already allocated 2D array of chars.
 * Exits the entire program if the file cannot be opened.
 */
size_t read_data(char *filename, char **data) {
    // Open the file.
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open '%s'. %s.\n", filename,
                strerror(errno));
        free(data);
        exit(EXIT_FAILURE);
    }

    // Read in the data.
    size_t index = 0;
    char str[MAX_STRLEN + 2];
    char *eoln;
    while (fgets(str, MAX_STRLEN + 2, fp) != NULL) {
        eoln = strchr(str, '\n');
        if (eoln == NULL) {
            str[MAX_STRLEN] = '\0';
        } else {
            *eoln = '\0';
        }
        // Ignore blank lines.
        if (strlen(str) != 0) {
            data[index] = (char *)malloc((MAX_STRLEN + 1) * sizeof(char));
            strcpy(data[index++], str);
        }
    }

    // Close the file before returning from the function.
    fclose(fp);
    return index;
}

int main(int argc, char **argv) {
	int iflag=0;
	int dflag=0;
	int flag;
	opterr=0;
	if (argc==1){
		usage_message();
		return EXIT_FAILURE;
	}
	while((flag=getopt(argc, argv, "id")) != -1){
		switch(flag){
		case 'i':
			iflag=1;
			break;
		case 'd':
			dflag=1;
			break;
		case '?':
			printf("Error: Unknown option '-%c' received.\n", optopt);
			usage_message();
			return EXIT_FAILURE;
		case ':':
			printf("Error: No input file specified.\n");
			return EXIT_FAILURE;
		}
	}

	if (iflag+dflag>1){
		printf("Error: Too many flags specified.\n");
		return EXIT_FAILURE;
	}
	int i=0;
	char *file;
	if (optind < argc-1){
		printf("Error: Too many files specified.\n");
		return EXIT_FAILURE;
	}
	else if(optind>argc-1){
		printf("Error: No input file specified.\n");
		return EXIT_FAILURE;
	}
	else{
		file=argv[optind];
	    i=1;
	  }

	char **info = (char **)malloc(MAX_ELEMENTS*sizeof(char*));
	size_t length=read_data(file, info);

	if(i+iflag==2){
		int *info2 = (int *)malloc(length*sizeof(int));
		for (int i=0; i<length; i++){
			info2[i]=atoi(info[i]);
		}
		quicksort(info2, length, sizeof(int), int_cmp);
		for(int i=0; i<length;i++){
			printf("%d\n", info2[i]);
		}
		free(info2);
	}
	else if(i+dflag==2){
		double *info2 = (double *)malloc(length*sizeof(double));
		for (int i=0; i<length; i++){
			info2[i]=atof(info[i]);
		}
		quicksort(info2, length, sizeof(double), dbl_cmp);
		for(int i=0; i<length;i++){
			printf("%f\n", info2[i]);
		}
		free(info2);
	}
	else{
		quicksort(info, length, sizeof(char*), str_cmp);
		for(int i=0; i<length;i++){
			printf("%s\n", info[i]);
		}
	}

	if (info){
		for (int i=0; i<length; i++){
			free(info[i]);
		}
	        free(info);
	    }
    return EXIT_SUCCESS;
}
