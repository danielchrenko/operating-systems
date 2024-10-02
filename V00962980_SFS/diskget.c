#include "sfs.h"


void write_data(FILE *file_handle, FILE *write_handle, int location, int bytes, int sectors, int leftover) {

	char last[leftover];
	char buffer[512];
	int address_location = ((location) - 2 + 33)*512;
	if (sectors == 1) {
		goto onewrite;
	}
	fseek(file_handle, address_location, SEEK_SET);
	fread(&buffer, 512, 1, file_handle);
	fwrite(buffer, 1, 512, write_handle);

	for (int i = 0; i < sectors - 2; i++) {
		
		// minus 2 because you read 2 sectors (last and first beforehand)
		int sector_location = get_address(file_handle, (location));
		address_location = ((sector_location) - 2 + 33)*512;
	
		fseek(file_handle, address_location, SEEK_SET);
		fread(&buffer, 512, 1, file_handle);
		fwrite(buffer, 1, 512, write_handle);
		
		location++;
		
	}
	
	int sector_location = get_address(file_handle, (location));
	address_location = ((sector_location) - 2 + 33)*512;
	
	onewrite:
	fseek(file_handle, address_location, SEEK_SET);
	fread(&last, leftover, 1, file_handle);
	fwrite(last, 1, leftover, write_handle);
	// last sector for writing (use last bytes)
}



int main(int argc, char* argv[]) {
	
	FILE *file_handle;
	boot_t boot_sector;
	entry_t first_entry;
	
	if (access(argv[1], F_OK) == 0) {
		
		file_handle = fopen(argv[1], "r");
		// open the file in arg / file exists
		
		int entry_address = get_entry_address(file_handle, argv[2]);
		
		if (entry_address == -1) {
			printf("File not Found.\n");
			fclose(file_handle);
			exit(0);
		} else {
		
			// printf("Entry Address: %d\n", entry_address);
		
			// file was found!
			fseek(file_handle, entry_address, SEEK_SET);
			fread(&first_entry, sizeof(first_entry), 1, file_handle);
			
			// first entry now contains the file
			int location = first_entry.cluster;
			int file_bytes = first_entry.size;
			int file_sectors = (file_bytes / 512) + 1;
			int leftover_bytes = file_bytes - ((file_sectors - 1) * 512);
			
			// printf("file bytes: %d\nfile sectors: %d\nfile leftover: %d\n", file_bytes, file_sectors, leftover_bytes);
			
			FILE *write_handle = fopen(argv[2], "w");
			// now we read the first file in the physical sector (cluster + 33 - 2)*512 then we use that
			write_data(file_handle, write_handle, location, file_bytes, file_sectors, leftover_bytes); 
			// to find the next data sector in the fat table, and keep finding next until
			fclose(write_handle);
			
		}
		
		fclose(file_handle);
		// close the file
	} else {
		
		printf("Disk: '%s' does not exist!\n", argv[1]);
		
	}
	
}