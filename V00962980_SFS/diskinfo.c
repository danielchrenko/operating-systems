#include "sfs.h"

entry_t find_label(FILE *file_handle) {
	
	entry_t label_entry;
	
	fseek(file_handle, 0x2600, SEEK_SET);
	for (int i = 0; i < 208; i++) {
		
		fread(&label_entry, sizeof(label_entry), 1, file_handle);
		
		if (label_entry.attributes == 0x08) {
			break;
		}
	
	}
	return label_entry;
	
}

int num_files(FILE *file_handle, int address, int count, char* filename, int prev_cluster) {

	int file_count = 0;

	entry_t label_entry;
	int dir_count = 0;
	int dir_address[1000];
	char dir_names[1000][8];
	int clusters[1000];
	// max subfolders 1000
	
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
				int new_address = ((label_entry.cluster) - 2 + 33)*512;
				if ((int)label_entry.filename[0] != 46) {
					clusters[dir_count] = label_entry.cluster;
					dir_address[dir_count] = new_address;
					// printf("folder address: %d", new_address);
					snprintf(dir_names[dir_count], 8, "%s", label_entry.filename);
					dir_count++;
				}
			} else {
				file_count++;
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
					int new_address = ((label_entry.cluster) - 2 + 33)*512;
					if ((int)label_entry.filename[0] != 46) {
						clusters[dir_count] = label_entry.cluster;
						dir_address[dir_count] = new_address;
						// printf("folder address: %d", new_address);
						snprintf(dir_names[dir_count], 8, "%s", label_entry.filename);
						dir_count++;
					}
				} else {
					file_count++;
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
		return file_count + num_files(file_handle, dir_address[i], -1, dir_names[i], clusters[i]);
	}
	
	
	return file_count;
}


int main(int argc, char* argv[]) {
	
	FILE *file_handle;
	boot_t boot_sector;
	entry_t volume_entry;
	
	if (access(argv[1], F_OK) == 0) {
		
		file_handle = fopen(argv[1], "r");
		// open the file in arg / file exists
		
		fread(&boot_sector, sizeof(boot_sector), 1, file_handle);
		int free_sectors = boot_sector.total_sector_count - 33 + 2;
		int free = free_space(file_handle, free_sectors);
		
		volume_entry = find_label(file_handle);
		char filename[] = "r";
		
		int file_count = num_files(file_handle, 0x2600, 208, filename, -1);
		
		
		fclose(file_handle);
		// close the file
		
		printf("OS Name: %.*s\n", 8, boot_sector.os_name);
		printf("Label of the disk: %.*s\n", 11, volume_entry.filename);
		// need to check the label, in root
		printf("Total size of the disk: %d\n", (boot_sector.total_sector_count*boot_sector.bytes_per_sector));
		printf("Free size of the disk: %d \n", free*boot_sector.bytes_per_sector);
		printf("Number of files on the disk: %d \n", file_count);
		printf("Number of FAT copies: %d\n", boot_sector.num_fats);
		printf("Sectors per FAT: %d\n", boot_sector.sectors_per_fat);
		
	} else {
		printf("Disk: '%s' does not exist.\n", argv[1]);
	}
}