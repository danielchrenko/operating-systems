#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

//globals, these are necassary, and will need locking with multi threading
float highest_temp = -500; // these values are higher/lower than the lowest/highest possible temp
float lowest_temp = 500; // so these values make sense in applicability
char highest_city[50];
char lowest_city[50];
int total_values = 0;

// mutex lock -- global so all threads share, and the functions do as well
pthread_mutex_t m;
int status;

void *process_file(void* arg) {
	
	int index;
	index = (intptr_t)arg;
	
	char cities[10][20] = {"Charlottetown", "Edmonton", "Halifax", "Montreal", "Ottawa", "Quebec", "Toronto", "Vancouver", "Victoria", "Winnipeg"};
	char dat[10] = ".dat";
	char city[20];
	char file_name[30] = "";
	
	strcpy(city, cities[index]);
	strcat(file_name, city);
	strcat(file_name, dat);
	
	float max_temp = -500; // lowest possible temp is like -275 or smth so -500 should be good
	float min_temp = 500; // highest recorded is 56C so 500 should be good vals
	int entry_count = 0;
	float sum_temp = 0.0;
	
	float max;
	float min;
	
	// now we have to parse the file
	char line[1000];
	
	if (access(file_name, F_OK) == 0) {
		
		// checks to see if the file exists
	
		FILE *file_handle = fopen(file_name, "r");

		fgets(line, 1000, file_handle);
		// ignore the first line of the file

		while(fgets(line, 1000, file_handle)) {
			// fgets is fine here since a single thread has 1 file
			int scanned = sscanf(line, "%f %f", &max, &min);
			
			if (scanned == 2) { 
				// ignore the lines without values // or wrong values
				entry_count += 1;
				
				if (max > max_temp) {
					max_temp = max;
					// change max record if needed
				}
				
				if (min < min_temp) {
					min_temp = min;
					// change min record if needed
				}
				
				sum_temp += max;
				sum_temp += min;
				// will do sum temp / entry_count*2 for average
			}
		}

		fclose(file_handle);

		// print out the data

		printf("===================================================\n\
Data for: %s city\n\
%s's lowest temperature is: %.1f degrees Celsius\n\
%s's highest temperature is: %.1f degrees Celsius\n\
The average temperature for %s is: %f degrees Celsius\n\
Total values processed for %s are: %d\n\n", city, city, min_temp, city, \
				max_temp, city, (sum_temp/((entry_count)*2)), city, entry_count);
		
		/* CRITICAL SECTION BEGINS */
		status = pthread_mutex_init(&m, NULL);
		pthread_mutex_lock(&m);
		
		if (highest_temp < max_temp) {
			highest_temp = max_temp;
			strcpy(highest_city, city);
		}
		if (lowest_temp > min_temp) {
			lowest_temp = min_temp;
			strcpy(lowest_city, city);
		}
		total_values += entry_count;
		/* END OF CRITICAL SECTION */
		pthread_mutex_unlock(&m);
		
	}
}

// have arg count and char array (arg count for error control)
int main(int argc, char* argv[]) {
	
	clock_t start_t = clock();
	bool proper_arg = false;
	// number of arguments check
	
	if (argc == 2) {
		
		if (strcmp("-m", argv[1]) == 0) {
			proper_arg = true;
			// multithreading
			pthread_t *thread_ids;
			
			int num_threads = 10;
			thread_ids = (pthread_t*)malloc(sizeof(pthread_t)*num_threads);
			// allocate mem for number of threads
			
			for (int i = 0; i < num_threads; i++) {
				pthread_create(&thread_ids[i], NULL, process_file, (void*)(intptr_t)i);
			}
			
			for (int i = 0; i < num_threads; i++) {
				pthread_join(thread_ids[i], NULL);
			}

		} else {
			printf("Incorrect parameter.\n");
		}
	} else {
		// single threading
		proper_arg = true;
		for (int i = 0; i < 10; i++) {
			process_file((void*)(intptr_t)i);
		}
		
	}
	
	clock_t end_t = clock();
	
	if (proper_arg == true) {
		
		printf("===================================================\n");
		printf("Total values evaluated is: %d\n", total_values);
		printf("The lowest temperature overall is: %.2f reported in: %s\n", lowest_temp, lowest_city);
		printf("The highest temperature overall is: %.2f reported in: %s\n", highest_temp, highest_city);
		printf("Elapsed time: %ld clocks\n", (end_t - start_t));
	}
	
	exit(0);
}