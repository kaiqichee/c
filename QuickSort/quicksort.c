/*******************************************************************************
 * Name        : quicksort.c
 * Author      : Kaiqi Chee
 * Date        : 03/02/2021
 * Description : Quicksort implementation.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "quicksort.h"

/* Static (private to this file) function prototypes. */
static void swap(void *a, void *b, size_t size);
static int lomuto(void *array, int left, int right, size_t elem_sz,
                  int (*comp) (const void*, const void*));
static void quicksort_helper(void *array, int left, int right, size_t elem_sz,
                             int (*comp) (const void*, const void*));


int int_cmp(const void *a, const void *b) {
	const int *x=(int *)a;
	const int *y=(int *)b;
	if (*x<*y){
		return -1;
	}
	else if (*x>*y){
		return 1;
	}
	else{
		return 0;
	}
}

int dbl_cmp(const void *a, const void *b) {
	const double *x=(double *)a;
	const double *y=(double *)b;
	if (*x<*y){
		return -1;
	}
	else if (*x>*y){
		return 1;
	}
	else{
		return 0;
	}
}

int str_cmp(const void *a, const void *b) {
	return strcmp(*(const char**)a,*(const char**)b);
}

static void swap(void *a, void *b, size_t size) {
	char *x=(char *)a;
	char *y=(char *)b;
	char temp;

	while (size!=0){
		temp=*x;
		*x++=*y;
		*y++=temp;
		size--;
	}
}

static int lomuto(void *array, int left, int right, size_t elem_sz,
                  int (*comp) (const void*, const void*)) {
	char *array2=(char *)array;
	char *p=(char *)(array2+left*elem_sz);
	int s=left;
	for (int i=left+1; i<right+1; i++){
		if (comp((array2+i*elem_sz), p)<0){
			s=s+1;
			swap((array2+s*elem_sz), (array2+i*elem_sz), elem_sz);
		}
	}
	swap((array2+left*elem_sz), (array2+s*elem_sz), elem_sz);
	return s;
}


static void quicksort_helper(void *array, int left, int right, size_t elem_sz,
                             int (*comp) (const void*, const void*)) {
	if (left<right){
		int s=lomuto(array, left, right, elem_sz, comp);
		quicksort_helper(array, left, s-1, elem_sz, comp);
		quicksort_helper(array, s+1, right, elem_sz, comp);
	}
}

void quicksort(void *array, size_t len, size_t elem_sz,
               int (*comp) (const void*, const void*)) {
	quicksort_helper(array, 0, len-1, elem_sz, comp);
}


