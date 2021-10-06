/*******************************************************************************
 * Name        : quicksort.h
 * Author      : Kaiqi Chee
 * Date        : 03/02/2021
 * Description : Quicksort header.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#ifndef QUICKSORT_H
#define QUICKSORT_H

int int_cmp(const void *a, const void *b);
int dbl_cmp(const void *a, const void *b);
int str_cmp(const void *a, const void *b);
void quicksort(void *array, size_t len, size_t elem_sz,
               int (*comp) (const void*, const void*));
#endif

