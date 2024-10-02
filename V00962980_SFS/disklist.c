#include "sfs.h"



void print_files(FILE *file_handle, int address, int count, char* filename, int prev_cluster) {


	printf("\n%.*s\n==================\n", 8, filename);
	entry_t label_entry;
	int dir_count = 0;
	int dir_address[1000];
	char dir_names[1000][8];
	int clusters[1000];
	// MAX directories 1k
	
	if (count == 208) {
		// for the root
		fseek(file_handle, address, SEEK_SET);
		for (int i = 0; i < count; i++) {
			
			fread(&label_entry, sizeof(label_entry), 1, file_handle);
			
			if ((int)label_entry.filename[0] == 0x00) {
				break;
				// if end of directory end
			} else if (label_entry.filename[0] == 0x2E) {
				continue;
				// prev dir entry or root
			} else if ((int)label_entry.filename[0] == 0xE5) {
				continue;
				// if unused dir
			} else if (label_entry.attributes == 0x08) {
				continue;
				// volume label_entry
			} else if (label_entry.attributes == 0x0F ) {
				continue;
				// long file name
			} else if ((label_entry.attributes & 0x10) == 0x10 ) {
				printf("D %31.*s\n", 8, label_entry.filename);
				int new_address = ((label_entry.cluster) - 2 + 33)*512;
				if ((int)label_entry.filename[0] != 46) {
					clusters[dir_count] = label_entry.cluster;
					dir_address[dir_count] = new_address;
					// printf("folder address: %d", new_address);
					snprintf(dir_names[dir_count], 8, "%s", label_entry.filename);
					dir_count++;
				}
			} else {
				// date
				int year = 0;
				int month = 0;
				int day = 0;
				
				// time
				int hour = 0;
				int min = 0;
				
				// mask the time/date
				year = ((label_entry.create_date & 0xFE00) >> 9) + 1980;
				month = ((label_entry.create_date & 0x1E0) >> 5);
				day = ((label_entry.create_date & 0x1F));
				
				hour = ((label_entry.create_time & 0xF800) >> 11);
				min = ((label_entry.create_time & 0x7E0) >> 5);
				
				// setup strings
				char date[16];
				char time [16];
				snprintf(date, 16, "%04d/%02d/%02d", year, month, day);
				snprintf(time, 16, "%02d:%02d", hour, min);
				
				printf("F %10d %16.*s.%.*s %8s %8s\n", label_entry.size, 8, label_entry.filename, 3, label_entry.extension, date, time);
			}	
		}
	} else {
		
		fseek(file_handle, address, SEEK_SET);
		for (;;) {
			
			for (int i = 0; i < 16; i++) {
			
				fread(&label_entry, sizeof(label_entry), 1, file_handle);
				
				if ((int)label_entry.filename[0] == 0x00) {
					goto calldirs;
					// if end of directory end
				} else if (label_entry.filename[0] == 0x2E) {
					continue;
					// prev dir entry or root
				} else if ((int)label_entry.filename[0] == 0xE5) {
					continue;
					// if unused dir
				} else if (label_entry.attributes == 0x08) {
					continue;
					// volume label_entry
				} else if (label_entry.attributes == 0x0F ) {
					continue;
					// long file name
				} else if ((label_entry.attributes & 0x10) == 0x10 ) {
					printf("D %31.*s\n", 8, label_entry.filename);
					int new_address = ((label_entry.cluster) - 2 + 33)*512;
					if ((int)label_entry.filename[0] != 46) {
						clusters[dir_count] = label_entry.cluster;
						dir_address[dir_count] = new_address;
						// printf("folder address: %d", new_address);
						snprintf(dir_names[dir_count], 8, "%s", label_entry.filename);
						dir_count++;
					}
				} else {
					// date
					int year = 0;
					int month = 0;
					int day = 0;
					
					// time
					int hour = 0;
					int min = 0;
					
					// mask the time/date
					year = ((label_entry.create_date & 0xFE00) >> 9) + 1980;
					month = ((label_entry.create_date & 0x1E0) >> 5);
					day = ((label_entry.create_date & 0x1F));
					
					hour = ((label_entry.create_time & 0xF800) >> 11);
					min = ((label_entry.create_time & 0x7E0) >> 5);
					
					// setup strings
					char date[16];
					char time [16];
					snprintf(date, 16, "%04d/%02d/%02d", year, month, day);
					snprintf(time, 16, "%02d:%02d", hour, min);
					
					printf("F %10d %16.*s.%.*s %8s %8s\n", label_entry.size, 8, label_entry.filename, 3, label_entry.extension, date, time);
				}	
		
			}
			
			// increment address to next mem location
			int next_mem = get_address(file_handle, ((address/512+2-33)));
			address = ((next_mem) - 2 + 33)*512;
			fseek(file_handle, address, SEEK_SET);
		
		}
		
	}
	
	// now we recursively call the sub dirs
	calldirs:
	for (int i = 0; i < dir_count; i++) {
		print_files(file_handle, dir_address[i], -1, dir_names[i], clusters[i]);
	}
}


int main(int argc, char* argv[]) {
	
	FILE *file_handle;
	boot_t boot_sector;
	entry_t volume_entry;
	
	if (access(argv[1], F_OK) == 0) {
		
		file_handle = fopen(argv[1], "r");
		// open the file in arg / file exists
		
		char filename[] = "ROOTDIR";
		
		print_files(file_handle, 0x2600, 208, filename, -1);
		
		
		fclose(file_handle);
		// close the file
	} else {
		
		printf("Disk: '%s' does not exist!\n", argv[1]);
		
	}
	
}