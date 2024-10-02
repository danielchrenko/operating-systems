Daniel Chrenko V00962980

disk utility programs for Assignment 3 for CSC360

Instructions:

	!! First compile with makefile, so type: 'make' in same directory as makefile and .c files
	
	Also make sure that the disk files are in the same directory as diskinfo.c, disklist.c, diskget.c and diskput.c and make

	to run diskinfo type: ./diskinfo disk.IMA
	
	to run disklist type: ./disklist disk.IMA
	
	to run diskget type: ./diskget disk.IMA <filename>
	
	to run diskput type: ./diskput disk.IMA <filename>
	
	*FOR DISKPUT ONLY WORKS WITH ROOT DIRECTORY AND FILES <512 BYTES
	MAY WORK WITH LARGER, BUT CAN CORRUPT FAT TABLE
	
Important Notes:

	!! IMPORTANT: PLEASE TEST PROGRAMS FOR PARTS I, II, III before testing PART IV, as PART IV may corrupt the disk!

	disk.IMA can be replaced with any FAT12 formatted disk of choosing, such as something.IMA or disk2.img etc.

	Commands such as diskinfo, disklist are CASE SENSITIVE as well as the disk name and other args, 
	BUT filename is NOT case sensitive FOR ONLY diskget meaning hi.txt and HI.txt and hi.TXT will all refer
	to the same file. Extension also matters, so if you want figure1.jpg
	you need to type 'figure1.jpg' and not 'figure1'. Also for ease of use for diskget, however you type the filename as,
	that is how it will save the name as on the machine, so './diskget disk.IMA hEllo.txt' will get HELLO.TXT from disk 
	and save it as 'hEllo.txt'
	
	Files in sub directories are not accessable using diskget, as per assignment spec.
	
	AGAIN part 4 (diskput) can corrupt the FAT table in disk with files larger than 512 bytes. so please test
	parts 1-3 before part 4 if using one disk.


	also has make clean option in make file
