Daniel Chrenko V00962980

data_av.c program for Assignment 2 for CSC360

Instructions:

	First compile with makefile, so type 'make' in same directory as makefile and data_av.c
	Also make sure that the dat files are in the same directory as data_av.c and make

	to run SINGLE THREADED type: ./data_av
	
	to run MULTI THREADED type: ./data_av -m
	
	NOTE: case sensitive when running ./data_av or ./data_av -m
	
Important Notes:
	
	Files included in the submission: data_av.c, README, makefile, and data files

	Program does not work with just any .dat city files, and only works with the same city names as the 
	ones given to us in the assignment zip.
	
	That being said, if there is LESS .dat files, for example, if only 2 cities are in the same directory as data_av,
	it is still able to run and will produce appropriate results (Error handling). Number of threads is set to max possible
	amount of files (which is the original 10 given) but the threads that cannot find their file will terminate.
	
Report:

	PERFORMANCE ON LAPTOP (WSL linux machine)
	DUAL CORE i3-7100U (2C/4T)
	
	MULTI THREADED AVERAGE RUNTIME: 249897.3 CLOCKS
	SINGLE THREADED AVERAGE RUNTIME: 137958.3 CLOCKS

	Multi Threading vs Single threading
	
	For some reason there is more time elapsed with multi threading than single threading, multi threading
	on average seems to use about 250,000 clocks vs the approximate 138,000 clocks single threading takes.
	This is weird since, one would think that multi threaded would see better performance. I suspect it might
	have something to do with the time it takes for the threads to be created might actually outweigh the
	benefits of multi threading, ultimately making it slower in this specific case. Another problem might be that
	my laptop CPU only has 2 cores and 4 threads, so multi threading for this task might not be suited for such
	specifications.
	
	I also noticed that the linux machine had performance similar to single threading with multi threading,
	both options had very similar results when testing. Ran appropriately 180,000 clocks for both single 
	and multi on the UVic linux machine.
	
	Mutex Locking
	
	From the assignment 2 outline, it talks about how locks can cause the multi threading exection to stop running
	in parallel. If you have too much or even all of your code in a critical section portion, around locks, this 
	makes it so that each thread needs to wait for the previous thread to leave the critical section which ultimately
	ruins the point of having threads, since the entire program is running as if it was single threaded, because
	the threads are taking turns.

	I added mutex locks for the brief critical section of the code, where global variables for max overall and min
	overall are being accessed by the threads.

	Inconsistency

	Running the code without locking, can produce inaccurate results due to the max temp and min temp overall being
	open to access to all threads. This creates a race scenario. Without locks the max and min temp will produce
	INCONSISTENT results because when each thread checks what the current max and min are, they may access one
	of the variables at the same time which can ruin the comparisons in the code, when it checks if its higher or lower.
	To avoid this I added the mutex locks around the critical section as mentioned before.
	
	Another thing I want to add is that the print statements for each thread arrive at different times when
	running the code multiple times. This is because the threads don't always perform equally and time to complete
	the task can fluctuate.

	