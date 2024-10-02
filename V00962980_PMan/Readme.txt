Daniel Chrenko V00962980

PMan program for assignment 1 for CSC360

Instructions:

	First compile with makefile, so type 'make' in same directory as makefile and PMan.c

	to run type: ./PMan
	
	NOTE: Capital letter matters when running ./PMan

Commands:

	bg:

		type: bg [./program] [OPTIONAL: arguments]
		
		EXAMPLE: PMan: > bg ./inf hi 10
		EXAMPLE: PMan: > bg ./foo

		where 'program' is a already compiled program that is an executable.

		-->> IMPORTANT: program needs to be of the format './program' with the './'

			THIS IS CRUCIAL FOR THE COMMAND TO WORK

		-->> IMPORTANT: EXECUTABLE MUST BE IN THE SAME DIRECTORY AS PMAN


	bglist:

		type: bglist
	
		EXAMPLE: PMan: > bglist

		prints out a list of the current bg processes initialized by PMan using bg
		
		-->> IMPORTANT: bglist will print out all killed processes right before jobs list


	bgkill:
		
		type: bgkill [pid]

		EXAMPLE: PMan: > bgkill 514

		Sends the TERM signal to process with matching pid

	
	bgstop: 
	
		type: bgstop [pid]

		EXAMPLE: PMan: > bgstop 514

		Sends the STOP signal to process with matching pid

	bgstart:

		type: bgstart [pid]

		EXAMPLE: PMan: > bgstart 514

		Sends the CONT signal to process with matching pid

	pstat:

		type: pstat [pid]

		EXAMPLE: PMan: > pstat 514

		Prints out status information given a PID



	
