#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

/* Linked list for args and procs list (also file parsing!) */
struct node {
	char *data;
	// string pointer
	struct node *next;
	// pointer to next node
	pid_t pid;
	// used processes linked list
};

struct list {
	struct node *head;
	int count;
};

void new_list(struct list *list) {
	list->head = NULL;
	list->count = 0;
}

void print_list(struct list *list) {
	
	printf("\n ** BACKGROUND JOBS ** \n");
	
	struct node *pointer = list->head;
	// point to the head
	
	while (pointer != NULL) {
		
		printf("%d : %s\n", pointer->pid, pointer->data);
		pointer = pointer->next;
		// keep iterating pointer through nodes while printing
	}
	
	printf("\n");
}

void insert_node(struct list *list, char *string, pid_t pid) {
	list->count = list->count + 1;
	struct node *new_node = (struct node*)malloc(sizeof(struct node));
	new_node->data = strdup(string);
	new_node->next = NULL;
	new_node->pid = pid;
	// created node's data is the string and next is null
	
	if (list->head == NULL) {
		// if there is nothing in list
		list->head = new_node;
		// make new node the header
	} else {
		// else go through the list till end
		struct node *last = list->head;
		while (last->next != NULL) {
			last = last->next;
			// keep iterating
		}
		last->next = new_node;
		// make the last's next node the newly created one
	}
}

char* get_index(struct list *list, int position) {
	
	if (position >= list->count) {
		return NULL;
		// avoid indexing a non existent node
	}

	struct node *pointer = list->head;
	// point to the head
	
	for (int i = 0; i < position; i++) {
		pointer = pointer->next;
		// pointer is now on the position
	}
	
	return pointer->data;
}

int get_node(struct list *list, pid_t pid) {
	// returns -1 if not in list
	// returns index otherwise
	
	if (1 > list->count) {
		return -1;
		// avoid indexing a non existent node
	}
	
	struct node *pointer = list->head;
	// point to the head
	
	for (int i = 0; i < list->count; i++) {
		pointer = pointer->next;
		if (pointer->pid == pid) {
			return i;
		}
	}
	
	return -1;
}

void delete_node(struct list *list, pid_t pid) {
	// note: if PID dne then nothing happens
	
	struct node *current = list->head;
	struct node *prev = NULL;
	
	while (current != NULL) {
		
		if (pid == current->pid) {
			// if pid matches
			if (prev == NULL) {
				// if first pid matches
				list->head = current->next;
				// make head the next one from the first
			} else {
				prev->next = current->next;
				// link prev's next to currents next
			}
			free(current->data);
			free(current);
			// free the pointers for mem
			
			list->count = list->count - 1;
			return;
		}
		
		prev = current;
		current = current->next;
		// iterate for the while loop
	}
	
}


/* PMan */
void bg(struct list *args, struct list *procs) {
	char *program = get_index(args, 1);
	
	if (access(program, F_OK) == 0) {
		
		char *argv[args->count];
		
		for (int i = 1; i <= args->count; i++) {
			if (i == args->count) {
				argv[i - 1] = NULL;
			} else {
				argv[i - 1] = get_index(args, i);
			}
		}
		// populate the arg array for execvp
		
		pid_t pid = fork();
		// fork the process
		
		if (pid < 0) {
			printf("Fork Failed\n");
			return;
		} else if (pid == 0) {
			int i = execvp(program, argv);
			if (i < 0) {
				printf("Encountered Problem, fork still created, process terminated.\n");
				exit(-1);
			}
		} else {
			insert_node(procs, program, pid);
		}
		
	} else {
		printf("PMan: > Error: File not Found\n");
	}
}

void bglist(struct list *procs) {
	
	int status;
	int pid = 0;
		
	for (int i = 0; i < procs->count + 1; i++) {
		// loop over all the possible procs to see which ones are zombies
	
		int pid = waitpid(-1, &status, WNOHANG);
		
		if (pid != -1 && pid != 0) {
			if (WIFSIGNALED(status)) {
				printf("\nProcess %d was killed\n", pid);
				delete_node(procs, pid);
			}
			if (WIFEXITED(status)) {
				printf("\nProcess %d exits\n", pid);
				delete_node(procs, pid);
			}
			// check if proc was killed or exited naturally
		}
		
	}
	
	print_list(procs);
	printf("Total background jobs: %d\n", procs->count);
}

void bgkill(pid_t pid) {
	int code = kill(pid, SIGTERM);
	if (code == -1 ) {
		printf("There was an error with command.\n");
	}
}

void bgstop(pid_t pid) {
	int code = kill(pid, SIGSTOP);
	if (code == -1 ) {
		printf("There was an error with command.\n");
	}
}

void bgstart(pid_t pid) {
	int code = kill(pid, SIGCONT);
	if (code == -1 ) {
		printf("There was an error with command.\n");
	}
}

void pstat(pid_t pid) {
	
	FILE *file_handle;
	char output;
	
	char path[50];
	char comm_path[50];
	char stat_path[50];
	char status_path[50];
	char comm_output[100];
	char status_output[2000];
	sprintf(path, "/proc/%d", pid);
	sprintf(comm_path, "/proc/%d/comm", pid);
	sprintf(stat_path, "/proc/%d/stat", pid);
	sprintf(status_path, "/proc/%d/status", pid);
	
	if (access(path, F_OK) == 0) {
		// check if the file exists
		
		/* READ THE COMM FILE */
		
		file_handle = fopen(comm_path, "r");
		
		while (fgets(comm_output, 100, file_handle) != NULL) {
			printf("\npstat for pid: %d\n", pid);
			printf(" - comm : %s", comm_output);
		}
		fclose(file_handle);
		
		/* READ THE STAT FILE */
		
		file_handle = fopen(stat_path, "r");
		
		while (fgets(status_output, 2000, file_handle) != NULL) {}
		// reads the file and stores chars into soutput
		
		struct list status_list;
		new_list(&status_list);
		// use same struct for pman input as status parsing
	
		if (strcmp(status_output, "\n") != 0) {
			int len = strlen(status_output);
			if (status_output[len - 1] == '\n') {
				status_output[len - 1] = '\0';
			}
			// same logic as before, del n and rplace w term
		}
		
		char *token = strtok(status_output, " ");
		
		while (token != NULL) {
			insert_node(&status_list, token, -1);
			token = strtok(NULL, " ");
			// same logic as reading args in read_command
		}
		fclose(file_handle);
		
		printf(" - status: %s\n", get_index(&status_list, 2));
		printf(" - utime: %f\n", (float)atoi(get_index(&status_list, 13))/ (float)sysconf(_SC_CLK_TCK));
		printf(" - stime: %f\n", (float)atoi(get_index(&status_list, 14))/ (float)sysconf(_SC_CLK_TCK));
		printf(" - rss: %s\n", get_index(&status_list, 23));
		
		/* READ THE STATUS FILE */
		
		file_handle = fopen(status_path, "r");
		
		struct list vstat;
		new_list(&vstat);
		char line[100];
		
		while (fscanf(file_handle, "%[^\n]\n", line) == 1) {
			insert_node(&vstat, line, -1);
		}
		fclose(file_handle);
		
		printf(" - %s\n", get_index(&vstat, 53));
		printf(" - %s\n", get_index(&vstat, 54));
		
	} else {
		printf("PMan: > Error: Process %d does not exist.\n", pid);
	}
}

int read_command(char *line, struct list *procs) {

	struct list args;
	new_list(&args);
	// create and initialize argument linked list
	
	if (strcmp(line, "\n") != 0) {
		int len = strlen(line);
		if (line[len - 1] == '\n') {
			line[len - 1] = '\0';
		}
		// remove newline and add zero teriminator
	}

	char *token = strtok(line, " ");
	
	while (token != NULL) {
		insert_node(&args, token, -1);
		token = strtok(NULL, " ");
		// iterate through dif args and put into linked list
	}
	
	if (strcmp("quit", args.head->data) == 0) {
		return 0;
	} else if (strcmp("bg", args.head->data) == 0) {
		bg(&args, procs);
		return 1;
	} else if (strcmp("bglist", args.head->data) == 0) {
		bglist(procs);
		return 1;
	} else if (strcmp("bgkill", args.head->data) == 0) {
		if (args.count == 2) {
			bgkill(atoi(args.head->next->data));
		} else if (args.count > 2) {
			printf("Too many arguments, format: bgkill [pid]\n");
		} else {
			printf("Too little arguments, format: bgkill [pid]\n");
		}
		return 1;
	} else if (strcmp("bgstop", args.head->data) == 0) {
		if (args.count == 2) {
			bgstop(atoi(args.head->next->data));
		} else if (args.count > 2) {
			printf("Too many arguments, format: bgstop [pid]\n");
		} else {
			printf("Too little arguments, format: bgstop [pid]\n");
		}
		return 1;
	} else if (strcmp("bgstart", args.head->data) == 0) {
		if (args.count == 2) {
			bgstart(atoi(args.head->next->data));
		} else if (args.count > 2) {
			printf("Too many arguments, format: bgstart [pid]\n");
		} else {
			printf("Too little arguments, format: bgstart [pid]\n");
		}
		return 1;
	} else if (strcmp("pstat", args.head->data) == 0) {
		if (args.count == 2) {
			pid_t pid = atoi(args.head->next->data);
			pstat(pid);
		} else if (args.count > 2) {
			printf("Too many arguments, format: bgstop [pid]\n");
		} else {
			printf("Too little arguments, format: bgstop [pid]\n");
		}
		return 1;
	} else if (strcmp("\n", args.head->data) == 0) {
		return 1;
	} else {
		printf("PMan: > %s: command not found\n", args.head->data);
		return -1;
	}
}

int main(void) {
	
	struct list procs;
	new_list(&procs);
	// process linked list

	while (true) {

		printf("PMan: > ");
		char *line = NULL;
		size_t len = 0;
		ssize_t lineSize = 0;
		lineSize = getline(&line, &len, stdin);
		int command_num;
		command_num = read_command(line, &procs);
		free(line);
		
		if (command_num == 0) {
			exit(0);
			// quit
		}
	}

	return 0;

}
