#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>

#include <ctype.h>

typedef struct {
	uint8_t _a[3];
	char os_name[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t num_fats;
	uint16_t root_entries;
	uint16_t total_sector_count;
	uint8_t _b;
	uint16_t sectors_per_fat;
	uint16_t sectors_per_track;
	uint16_t number_of_heads;
	uint8_t _c[4];
	uint32_t total_fat32;
	uint8_t _d[2];
	uint8_t boot_signature;
	uint32_t volume_id;
	char volume_label[11];
	char file_system_type[8];
} __attribute__ ((packed)) boot_t;

typedef struct {
	char filename[8];
	char extension[3];
	uint8_t attributes;
	uint8_t _a;
	uint8_t create_time_us;
	uint16_t create_time;
	uint16_t create_date;
	uint16_t last_access_date;
	uint8_t _b[2];
	uint16_t last_modified_time;
	uint16_t last_modified_date;
	uint16_t cluster;
	uint32_t size;
} __attribute__ ((packed)) entry_t;

typedef struct {
	uint8_t b0;
	uint8_t b1;
	uint8_t b2;
} __attribute__ ((packed)) fat_entry_t;


/* will return either the address or -1 if does not exist */
int get_entry_address(FILE *file_handle, char* filename) {

	int file_entry_address = -1;

	entry_t label_entry;
	
	fseek(file_handle, 0x2600, SEEK_SET);
	for (int i = 0; i < 208; i++) {
		
		fread(&label_entry, sizeof(label_entry), 1, file_handle);
		
		if ((int)label_entry.filename[0] == 0xE5) {
			continue;
			// if unused directory
		} else if ((int)label_entry.filename[0] == 0x00) {
			break;
			// if end of the directory
		} else if (label_entry.attributes == 0x08) {
			continue;
			// volume label_entry
		} else if (label_entry.attributes == 0x0F ) {
			continue;
			// long file name
		} else if ((label_entry.attributes & 0x10) == 0x10 ) {
			continue;
			// directory (dont want that for this function)
		} else {
			
			/* CONVERT FILE TO LOWERCASE */
			
			char lower_filename[13];
			int next = 0;
			
			for (int j = 0; j < 8; j++) {
			
				if ((int)label_entry.filename[j] != 32) {
				
					// if does not equal space
					lower_filename[next] = tolower((int)label_entry.filename[j]);
					next++;
				
				}
			}

			lower_filename[next] = '.';
			next++;
			
			for (int j = 0; j < 3; j++) {
			
				if ((int)label_entry.extension[j] != 32) {
				
					// if does not equal space
					lower_filename[next] = tolower((int)label_entry.extension[j]);
					next++;
				}
			}
			
			lower_filename[next] = '\0';
			next++;
			
			/* CONVERT INPUT TO LOWERCASE */
			
			char lower_input[13];
			next = 0;
			
			for (int j = 0; j < 12; j++) {
				
				if ((int)filename[j] != 32) {
				
					// if does not equal space
					lower_input[next] = tolower((int)filename[j]);
					next++;
				}
				
			}
			
			lower_input[next] = '\0';
			next++;
			// check this if broken
			
			if (strcmp(lower_filename, lower_input) == 0) {
				
				file_entry_address = (i*32) + 0x2600;
				// address of the found file
			}
			
		}
		
	}
	
	return file_entry_address;
}

void write_address(FILE *file_handle, int n, uint16_t write) {
	
	int i, free_bytes = 0;
	uint8_t X0, X1, X2;
	uint16_t X, Y;
	fat_entry_t fat_entry;
	fat_entry_t write_entry;
	
	if (n & 1) {
		i = (((n - 1)*3)/2);
	} else {
		i = ((n*3)/2);
	}
	
	// printf("%d", i);
	int entry_address = 0x200 + i;
	
	fseek(file_handle, entry_address, SEEK_SET); // this might be wrong
	
	fread(&fat_entry, sizeof(fat_entry), 1, file_handle);
	X0 = fat_entry.b0;
	X1 = fat_entry.b1;
	X2 = fat_entry.b2;
	
	X = ((X1 & 0x0F) << 8) + X0;
	Y = ((X1 & 0xF0) >> 4) + (X2 << 4);
		
	if (n & 1) {
		// lower
		write_entry.b0 = fat_entry.b0;
		uint16_t left = ((0b000000001111 & write) << 4);
		
		write_entry.b1 = (X1 & 0b00001111) + (uint8_t)left;
		
		uint16_t temp = ((write & 0b111111110000) >> 4);
		write_entry.b2 = (uint8_t)temp;
		
		fwrite(&write_entry, 3, 1, file_handle);
		fseek(file_handle, (entry_address+0x1200), SEEK_SET);
		//backup fat
		fwrite(&write_entry, 3, 1, file_handle);
		
		
	} else {
		// higher
		uint16_t temp = (write & 0b000011111111);
		write_entry.b0 = (uint8_t)temp;
		
		uint16_t right = ((0b111100000000 & write) >> 8);
		write_entry.b1 = (X1 & 0b11110000) + (uint8_t)right;

		write_entry.b2 = fat_entry.b2;
		
		fwrite(&write_entry, 3, 1, file_handle);
		fseek(file_handle, (entry_address+0x1200), SEEK_SET);
		//backup fat
		fwrite(&write_entry, 3, 1, file_handle);
		
	}
	
}



uint16_t get_address(FILE *file_handle, int n) {
	
	int i, free_bytes = 0;
	uint8_t X0, X1, X2;
	uint16_t X, Y;
	fat_entry_t fat_entry;
	
	if (n & 1) {
		i = (((n - 1)*3)/2);
	} else {
		i = ((n*3)/2);
	}
	
	// printf("%d", i);
	int entry_address = 0x200 + i;
	
	fseek(file_handle, entry_address, SEEK_SET); // this might be wrong
	
	fread(&fat_entry, sizeof(fat_entry), 1, file_handle);
	X0 = fat_entry.b0;
	X1 = fat_entry.b1;
	X2 = fat_entry.b2;
	
	X = ((X1 & 0x0F) << 8) + X0;
	Y = ((X1 & 0xF0) >> 4) + (X2 << 4);
		
	if (n & 1) {
		return Y;
	} else {
		return X;
	}
	
}


int free_space(FILE *file_handle, int free_sectors) {
	
	int i, free_bytes = 0;
	uint8_t X0, X1, X2;
	uint16_t X, Y;
	fat_entry_t fat_entry;
	
	// fseek(file_handle, 0x218, SEEK_SET); // FAT1 is at 200 offset for the 2 reserved
	
	for (i = 2; i < free_sectors; i++) {
		
		int val = get_address(file_handle, i);
		
		if (val == 0) {
			free_bytes++;
		}
	
	}
	
	return free_bytes;
	
}