/*******************************************************************************
 * Name        : mtsieve.c
 * Author      : Kaiqi Chee
 * Date        : 04/22/2021
 * Description : Execute sieve with multithreading.
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
#include <math.h>
#include <ctype.h>
#include <sys/sysinfo.h>
#include <pthread.h>

int total_count;
pthread_mutex_t lock;

typedef struct arg_struct {
	int start;
	int end;
} thread_args;

void segmentPrint(int segmentNumber, int *segments){
	if (segmentNumber==1){
		printf("%d segment:\n", segmentNumber);
	}
	else{
		printf("%d segments:\n", segmentNumber);
	}
	for (int i=0; i<segmentNumber*2-1; i=i+2){
		printf("   [%d, %d]\n", segments[i], segments[i+1]);
	}
}

bool threes(int value){
	int count=0;
	int len=0;
	int temp=value;
	while(temp>0){
		len++;
		temp=temp/10;
	}
	temp=value;
	while(len>0){
		if(temp%10==3){
			count++;
		}
		temp=temp/10;
		len--;
	}
	if (count>=2){
		return true;
	}
	return false;
}

void esieve(int start, int limit, bool* is_prime) {
	for (int i=0; i<=limit; i++){
		is_prime[i]=true;
	}
	is_prime[0]=false;
	is_prime[1]=false;
	for (int i=2; i<=sqrt(limit);i++){
		if (is_prime[i]==true){
			for (int j=i*i; j<=limit; j=j+i){
				is_prime[j]=false;
			}
		}
	}
}

int segSieve(int size, int start, int end, bool *low_primes, bool *high_primes){
	for (int i=0; i<=size; i++){
		high_primes[i]=true;
	}

	for(int p=0; p<=sqrt(end); p++){
		if(low_primes[p]){
			int i=ceil((double) start/p)*p-start;
			if (start<=p){
				i=i+p;
			}
			for(int j=i; j<size+1; j=j+p){
				high_primes[j]=false;
			}
		}
	}
	int count=0;
	for (int i=0; i<=size; i++){
		if(high_primes[i] && threes(i+start)){
			count++;
		}
	}
	return count;
}

void *sieve(void *ptr){
	thread_args *t=(thread_args *)ptr;
	bool *low_primes;
	if((low_primes=(bool *)malloc(t->end*sizeof(bool)))==NULL){
		fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	esieve(t->start,sqrt(t->end),low_primes);
	int size=t->end-t->start;
	bool *high_primes;
	if((high_primes=(bool *)malloc((size+1)*sizeof(bool)))==NULL){
		fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	int count=segSieve(size, t->start, t->end, low_primes, high_primes);
	int retval;
	if ((retval = pthread_mutex_lock(&lock)) != 0) {
		fprintf(stderr, "Warning: Cannot lock mutex. %s.\n",
				strerror(retval));
	}
	total_count=total_count+count;
	if ((retval = pthread_mutex_unlock(&lock)) != 0) {
		fprintf(stderr, "Warning: Cannot unlock mutex. %s.\n",
				strerror(retval));
	}
	free(low_primes);
	free(high_primes);
	pthread_exit(NULL);

}

void usage_message(){
	printf("Usage: ./mtsieve -s <starting value> -e <ending value> -t <num threads>\n");
}

bool overflow(char *number){
	long long int temp;
	if(sscanf(number, "%lld", &temp)==1){
		int temp2=(int)temp;
		if (temp==temp2){
			return false;
		}
		return true;
	}
	return true;
}

bool isNumber(char *num){
	bool b=true;
	for(int i=0; i<strlen(num); i++){
		if(isdigit(num[i])==false){
			b=false;
		}
	}
	return b;
}

int main(int argc, char **argv) {
	int sflag=0;
	int eflag=0;
	int tflag=0;
	int start;
	int end;
	int num_threads;
	int flag;
	opterr=0;
	if (argc==1){
		usage_message();
		return EXIT_FAILURE;
	}
	while((flag=getopt(argc, argv, "s:e:t:")) != -1){
		switch(flag){
		case 's':
			sflag=1;
			if(isNumber(optarg)==false){
				fprintf(stderr, "Error: Invalid input '%s' received for parameter '-%c'.\n", optarg, 's');
				return EXIT_FAILURE;
			}
			else if (overflow(optarg)){
				fprintf(stderr, "Error: Integer overflow for parameter '-%c'.\n", 's');
				return EXIT_FAILURE;
			}
			start=atoi(optarg);
			break;
		case 'e':
			eflag=1;
			if(isNumber(optarg)==false){
				fprintf(stderr, "Error: Invalid input '%s' received for parameter '-%c'.\n", optarg, 'e');
				return EXIT_FAILURE;
			}
			else if (overflow(optarg)){
				fprintf(stderr, "Error: Integer overflow for parameter '-%c'.\n", 'e');
				return EXIT_FAILURE;
			}
			end=atoi(optarg);
			break;
		case 't':
			tflag=1;
			if(isNumber(optarg)==false){
				fprintf(stderr, "Error: Invalid input '%s' received for parameter '-%c'.\n", optarg, 't');
				return EXIT_FAILURE;
			}
			else if (overflow(optarg)){
				fprintf(stderr, "Error: Integer overflow for parameter '-%c'.\n", 't');
				return EXIT_FAILURE;
			}
			num_threads=atoi(optarg);
			break;
		case '?':
			if (optopt == 'e' || optopt == 's' || optopt == 't') {
				fprintf(stderr, "Error: Option -%c requires an argument.\n",
						optopt);
			} else if (isprint(optopt)) {
				fprintf(stderr, "Error: Unknown option '-%c'.\n", optopt);
			} else {
				fprintf(stderr, "Error: Unknown option character '\\x%x'.\n",
						optopt);
			}
			return EXIT_FAILURE;
		}
	}
	int num_processors=get_nprocs();
	if (optind<argc){
		fprintf(stderr, "Error: Non-option argument '%s' supplied.\n", argv[optind]);
		return EXIT_FAILURE;
	}
	else if(sflag==0){
		fprintf(stderr, "Error: Required argument <starting value> is missing.\n");
		return EXIT_FAILURE;
	}
	else if(start<2){
		fprintf(stderr, "Error: Starting value must be >= 2.\n");
		return EXIT_FAILURE;
	}
	else if(eflag==0){
		fprintf(stderr, "Error: Required argument <ending value> is missing.\n");
		return EXIT_FAILURE;
	}
	else if(end<2){
		fprintf(stderr, "Error: Ending value must be >= 2.\n");
		return EXIT_FAILURE;
	}
	else if(end<start){
		fprintf(stderr, "Error: Ending value must be >= starting value.\n");
		return EXIT_FAILURE;
	}
	else if(tflag==0){
		fprintf(stderr, "Error: Required argument <num threads> is missing.\n");
		return EXIT_FAILURE;
	}
	else if(num_threads<1){
		fprintf(stderr, "Error: Number of threads cannot be less than 1.\n");
		return EXIT_FAILURE;
	}
	else if(num_threads>2*num_processors){
		fprintf(stderr, "Error: Number of threads cannot exceed twice the number of processors(%d).\n", num_processors);
		return EXIT_FAILURE;
	}
	//create segments
	int size=end-start;
	int numberChecked=size+1;
	if(num_threads>numberChecked){
		num_threads=numberChecked;
	}
	int segmentSize=numberChecked/num_threads;
	int segmentNum=numberChecked/segmentSize;
	int segments[segmentNum*2];
	int count=0;
	int temp=start;
	int rem=numberChecked%segmentNum;
	//if the count can be split into num_thread segments.
	for(int i=0; i<segmentNum*2; i++){
		segments[i]=temp;
		count++;
		if(i%2==0){
			temp=temp+segmentSize-1;
		}
		else{
			temp++;
		}
	}
	//if it can't be split into num thread increments;
	if(rem!=0){
		int temp=1;
		while(rem>0){
			for(int i=temp; i<segmentNum*2; i++){
				segments[i]++;
			}
			temp=temp+2;
			rem--;
		}
	}
	printf("Finding all prime numbers between %d and %d.\n", start, end);
	segmentPrint(segmentNum, segments);

	pthread_t threads[num_threads];
	thread_args targs[num_threads];
	int c = 0;
	for (int i = 0; i < num_threads; i++) {
		targs[i].start=segments[c];
		targs[i].end=segments[c+1];
		c=c+2;
		int retval;
		if ((retval = pthread_create(&threads[i], NULL, sieve, (void*) (&targs[i]))) != 0) {
			fprintf(stderr,
					"Error: Cannot create thread %d. " "No more threads will be created. %s.\n",
					i + 1, strerror(retval));
			break;
		}
	}

	for (int i = 0; i < num_threads; i++) {
		if (pthread_join(threads[i], NULL) != 0) {
			fprintf(stderr, "Warning: Thread %d did not join properly.\n",
					i + 1);
		}
	}
	int retval;
	if ((retval = pthread_mutex_destroy(&lock)) != 0) {
		fprintf(stderr, "Warning: Cannot destroy mutex. %s.\n", strerror(retval));
	}
	printf("Total primes between %d and %d with two or more '3' digits: %d\n", start, end, total_count);
	return EXIT_SUCCESS;

}
