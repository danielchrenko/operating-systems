#include <sys/stat.h>
#include <time.h>

#include "sfs.h"

int write_multi_sector(FILE* disk_handle, FILE *read_handle, entry_t final_entry, boot_t boot_sector, int rq) {
	
	char buffer[512];
	for (int i = 0; i < 512; i++) {
		buffer[i] = (char)20;
	}
	int address_location = ((final_entry.cluster) - 2 + 33)*512;
	fseek(disk_handle, address_location, SEEK_SET);
	fread(&buffer, 512, 1, read_handle);
	fwrite(buffer, 1, 512, disk_handle);
	
	uint16_t next_entry;
	int free_sectors = boot_sector.total_sector_count - 33 + 2;
	write_address(disk_handle, final_entry.cluster, 0xFFF);
	// temp write this so when it searches through to find new, it finds
	
	for (int i = 2; i < free_sectors; i++) {

		uint16_t fe = get_address(disk_handle, i);
		
		if (fe == 0x0000) {
			next_entry = i;
			break;
		}
	}
	
	write_address(disk_handle, final_entry.cluster, next_entry);
	uint16_t current_entry;
	
	/* looping through meat of the file */
	for (int i = 0; i < rq - 2; i++) {
		
		for (int i = 0; i < 512; i++) {
			buffer[i] = (char)20;
			// pad buffer with 0s
		}
		address_location = ((next_entry) - 2 + 33)*512;
		fseek(disk_handle, address_location, SEEK_SET);
		fread(&buffer, 512, 1, read_handle);
		fwrite(buffer, 1, 512, disk_handle);
		// reaed and write into next cluster
		
		current_entry = next_entry;
		write_address(disk_handle, current_entry, 0xFFF);
		// temp write this so when it searches through to find new, it finds
		
		for (int i = 2; i < free_sectors; i++) {

			uint16_t fe = get_address(disk_handle, i);
			
			if (fe == 0x0000) {
				next_entry = i;
				break;
			}
		}
		
		write_address(disk_handle, current_entry, next_entry);

	}
	
	for (int i = 0; i < 512; i++) {
			buffer[i] = (char)20;
			// pad buffer with 0s
		}
	address_location = ((next_entry) - 2 + 33)*512;
	fseek(disk_handle, address_location, SEEK_SET);
	fread(&buffer, 512, 1, read_handle);
	fwrite(buffer, 1, 512, disk_handle);
	// reaed and write into next cluster
	
	current_entry = next_entry;
	write_address(disk_handle, current_entry, 0xFFF);
	// final write to close off the file
}


entry_t write_entry(FILE *disk_handle, FILE *read_handle, char *filename, int address, boot_t boot_sector) {
	
	struct stat fstat;
	stat(filename, &fstat);
	entry_t file_entry;
	
	/* filename padding */
	char* token = strtok(filename, ".");
	
	char entry_filename[8];
	char entry_extension[3];
	
	strncpy(entry_filename, filename, 8);
	
	bool found_null = false;
	for (int i = 0; i < 8; i++) {
		if (found_null == false) {
			if (entry_filename[i] == '\0') {
				entry_filename[i] = (char)0x20;
				found_null = true;
			}
		} else {
			entry_filename[i] = (char)0x20;
		}
	}
	
	/* extension padding */
	token = strtok(NULL, " ");
	strncpy(entry_extension, token, 3);

	found_null = false;
	for (int i = 0; i < 3; i++) {
		if (found_null == false) {
			if (entry_extension[i] == '\0') {
				entry_extension[i] = (char)0x20;
				found_null = true;
			}
		} else {
			entry_extension[i] = (char)0x20;
		}
	}
	
	// now entry_extension and entry_filename have strings for struct
	strcpy(file_entry.filename, entry_filename);
	strcpy(file_entry.extension, entry_extension);
	
	file_entry.attributes = 0x00;
	// attr and the unused empty
	
	// now for time stuff, for now keep as 0
	file_entry.create_time_us = 0x00;
	file_entry.create_time = 0x00;
	file_entry.create_date = 0x00;
	file_entry.last_access_date = 0x00;
	file_entry.last_modified_time = 0x00;
	file_entry.last_modified_date = 0x00;
	
	// size
	uint32_t size = fstat.st_size;
	
	file_entry.size = size;
	
	// cluster
	uint16_t free_entry;
	int free_sectors = boot_sector.total_sector_count - 33 + 2;

	for (int i = 2; i < free_sectors; i++) {
		
		uint16_t fe = get_address(disk_handle, i);
		
		if (fe == 0x0000) {
			free_entry = i;
			// might be a problem with i??
			break;
		}
	}
	
	// HE LP -> LP HE
	uint16_t he = (free_entry & 0b1111111100000000);
	uint16_t lp = (free_entry & 0b0000000011111111);
	//file_entry.cluster = (he >> 8) | (lp << 8);
	
	file_entry.cluster = free_entry;
	
	
	// now we are done we need to write to the disk
	return file_entry;
	
}



int find_free_entry(FILE *disk_handle) {
	
	int entry_address = 0x00;
	
	entry_t label_entry;
	
	fseek(disk_handle, 0x2600, SEEK_SET);
	for (int i = 0; i < 208; i++) {
		
		fread(&label_entry, sizeof(label_entry), 1, disk_handle);
		
		if ((int)label_entry.filename[0] == 0xE5 || (int)label_entry.filename[0] == 0x00) {
			
			entry_address = 0x2600 + (i*32);
			// if unused directory, set the entry address as the address free
			break;
			
		}
		
	}
	
	return entry_address;
	
}



int main(int argc, char* argv[]) {
	
	/* load variables */
	boot_t boot_sector;
	
	FILE *disk_handle;
	FILE *read_handle;

	/* check file access */
	if (!((access(argv[1], F_OK) == 0) && (access(argv[2], F_OK) == 0))) {
		printf("Error, either disk or write file does not exist\n");
		exit(0);
	} 
	
	/* load fstat */
	struct stat fstat;
	stat(argv[2], &fstat);
	
	disk_handle = fopen(argv[1], "r+");
	read_handle = fopen(argv[2], "r");
	
	int entry_address = get_entry_address(disk_handle, argv[2]);
	
	fseek(disk_handle, 0x00, SEEK_SET);
	fread(&boot_sector, sizeof(boot_sector), 1, disk_handle);
	
	if (entry_address != -1) {
		printf("File already exists!\n");
		fclose(disk_handle);
		fclose(read_handle);
		exit(0);
	} else {
		printf("File does not exist on disk, writing to disk...\n");
		
		/* check free space here and quit accordingly */
		
		int first_free_address = find_free_entry(disk_handle);
		
		if (first_free_address == 0x00) {
			printf("No entries available! exiting...\n");
			exit(0);
		}
		
		entry_t final_entry = write_entry(disk_handle, read_handle, argv[2], first_free_address, boot_sector);
		// final entry now contains all info for the directory entry
		
		fseek(disk_handle, first_free_address, SEEK_SET);
		fwrite(&final_entry, 32, 1, disk_handle);
		// size is 32 and write the final entry into the free dir space
		
		uint32_t size = fstat.st_size;
		int rq_sectors = (size/512 + 1);
		
		if (rq_sectors == 1) {
			
			/* for special case of 1 sector file (<512 bytes) */
			
			char buffer[512];
			for (int i = 0; i < 512; i++) {
				buffer[i] = (char)20;
			}
			// fill buffer with padded spaces
			int address_location = ((final_entry.cluster) - 2 + 33)*512;
			fseek(disk_handle, address_location, SEEK_SET);
			// write entry to the cluster
			fread(&buffer, 512, 1, read_handle);
			fwrite(buffer, 1, 512, disk_handle);
			
			
			// write entry to the fat table
			write_address(disk_handle, final_entry.cluster, 0xFFF);
			// for now as EOF
		
		} else {
			
			int i = write_multi_sector(disk_handle, read_handle, final_entry, boot_sector, rq_sectors);
			
		}
		
		
		
		fclose(disk_handle);
		fclose(read_handle);
	}
}