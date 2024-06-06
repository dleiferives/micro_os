#include "fat_helper.h"
#include "lru_cache.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

unsigned int read_bytes_to_int(FILE *fp, int *cursor_loc, int offset_start,  int offset, int size) {
	if (size > 4) {
		fprintf(stderr,
		  "Size missmatch reading bytes at cursor_loc=%d offset_start=%d "
		  "offset=%d size=%d\n",
		  *cursor_loc, offset_start, offset, size);
		exit(1);
	}
	int result = 0;
	while (*cursor_loc < (offset_start + offset)) {
		getc(fp);
		*cursor_loc = *cursor_loc + 1;
	}

	for (int i = 0; i < size; i++) {
		result += (unsigned char)getc(fp) << (i * 8);
		*cursor_loc = *cursor_loc + 1;
	}
	return result;
}
void read_bytes_to_char_arr(FILE *fp, unsigned char *source, int *cursor, int offset_start, int offset, int size) {
	printf("Cursor pre %d, ", *cursor);
	while (*cursor < (offset_start + offset)) {
		getc(fp);
		*cursor = *cursor + 1;
	}
	printf("Cursor loc %d ofset %d, offset_start %d\n", *cursor, offset,
		offset_start);
	for (int i = 0; i < size; i++) {
		source[i] = getc(fp);
		putchar(source[i]);
		*cursor = *cursor + 1;
	}
}

int FAT_Device_identify_fat_type_and_init(FAT_Device *d, int *c, FILE *fp) {
	// Calculate RootDirSectors
	long RootDirSectors;
	RootDirSectors =
		((d->BPB_RootEntCnt * 32) + (d->BPB_BytsPerSec - 1)) / d->BPB_BytsPerSec;
	d->RootDirSectors = RootDirSectors;

	// Calculate DataSec
	long FATSz, TotSec, DataSec;
	if (d->BPB_FATSz16 != 0) {
		// init for fat 16 and 12 since they share the same types of data
		FAT_16_or_12_Device device;
		device.BS_DrvNum = read_bytes_to_int(fp, c, 0, 36, 1);
		device.BS_Reserved1 = read_bytes_to_int(fp, c, 0, 37, 1);
		device.BS_BootSig = read_bytes_to_int(fp, c, 0, 38, 1);
		device.BS_VolID = read_bytes_to_int(fp, c, 0, 39, 4);
		read_bytes_to_char_arr(fp, device.BS_VolLab, c, 0, 43, 11);
		read_bytes_to_char_arr(fp, device.BS_FilSysType, c, 0, 54, 8);
		FAT_Type ft;
		ft.FAT_16_or_12 = device;
		d->FAT_Type_Specifics = ft;
		FATSz = d->BPB_FATSz16;
		d->FATSz = FATSz;
	} else {
		// init for fat 32  since they share the same types of data
		FAT_32_Device device;
		device.BPB_FATSz32 = read_bytes_to_int(fp, c, 0, 36, 4);
		device.BPB_ExtFlags = read_bytes_to_int(fp, c, 0, 40, 2);
		device.BPB_FSVer = read_bytes_to_int(fp, c, 0, 42, 2);
		device.BPB_RootClus = read_bytes_to_int(fp, c, 0, 44, 4);
		device.BPB_FSInfo = read_bytes_to_int(fp, c, 0, 48, 2);
		device.BPB_BkBootSec = read_bytes_to_int(fp, c, 0, 50, 2);
		read_bytes_to_char_arr(fp, device.BPB_Reserved, c, 0, 52, 12);
		// device.BPB_Reserved = read_bytes_to_int(fp, c, 0, 52, 12);
		device.BS_DrvNum = read_bytes_to_int(fp, c, 0, 64, 1);
		device.BS_Reserved1 = read_bytes_to_int(fp, c, 0, 65, 1);
		device.BS_BootSig = read_bytes_to_int(fp, c, 0, 66, 1);
		device.BS_VolID = read_bytes_to_int(fp, c, 0, 67, 1);
		read_bytes_to_char_arr(fp, device.BS_VolLab, c, 0, 71, 11);
		read_bytes_to_char_arr(fp, device.BS_FilSysType, c, 0, 82, 8);
		FAT_Type ft;
		ft.FAT_32 = device;
		d->FAT_Type_Specifics = ft;
		FATSz = d->FAT_Type_Specifics.FAT_32.BPB_FATSz32;
		d->FATSz = FATSz;
	}

	if (d->BPB_TotSec16 != 0) {
		TotSec = d->BPB_TotSec16;
	} else {
		TotSec = d->BPB_TotSec32;
	}

	DataSec =
		TotSec - (d->BPB_RsvdSecCnt + (d->BPB_NumFATs * FATSz) + RootDirSectors);

	// Calculate CountofClusters
	int CountofClusters = DataSec / d->BPB_SecPerClus;

	// Determine the FAT type based on the cluster count
	if (CountofClusters < 4085) {
		return 12; // Volume is FAT12
	} else if (CountofClusters < 65525) {
		return 16; // Volume is FAT16
	} else {
		return 32; // Volume is FAT32
	}
}
FAT_Device FAT_Device_init(FILE *fp, Cache_t *cache) {
	int cursor = 0;
	FAT_Device d;
	d.sector_cache = cache;
	d.get_sector = Cache_get;


	// Read and initialize the structure fields from the binary file
	read_bytes_to_char_arr(fp, d.BS_jmpBoot, &cursor, 0, 0, 3);
	read_bytes_to_char_arr(fp, d.BS_OEMName, &cursor, 0, 3, 8);
	d.BPB_BytsPerSec = read_bytes_to_int(fp, &cursor, 0, 11, 2);

	// Initialize more fields based on the provided table
	d.BPB_SecPerClus = read_bytes_to_int(fp, &cursor, 0, 13, 1);
	d.BPB_RsvdSecCnt = read_bytes_to_int(fp, &cursor, 0, 14, 2);
	d.BPB_NumFATs = read_bytes_to_int(fp, &cursor, 0, 16, 1);
	d.BPB_RootEntCnt = read_bytes_to_int(fp, &cursor, 0, 17, 2);
	d.BPB_TotSec16 = read_bytes_to_int(fp, &cursor, 0, 19, 2);
	d.BPB_Media = read_bytes_to_int(fp, &cursor, 0, 21, 1);
	d.BPB_FATSz16 = read_bytes_to_int(fp, &cursor, 0, 22, 2);
	d.BPB_SecPerTrk = read_bytes_to_int(fp, &cursor, 0, 24, 2);
	d.BPB_NumHeads = read_bytes_to_int(fp, &cursor, 0, 26, 2);
	d.BPB_HiddSec = read_bytes_to_int(fp, &cursor, 0, 28, 4);
	d.BPB_TotSec32 = read_bytes_to_int(fp, &cursor, 0, 32, 4);

	// Set the FAT_Type_Val based on your logic (e.g., identify FAT12, FAT16, or
	// FAT32)
	d.FAT_Type_Val = FAT_Device_identify_fat_type_and_init(&d, &cursor, fp);

	d.FirstDataSector =  d.BPB_RsvdSecCnt + (d.BPB_NumFATs * d.FATSz) + d.RootDirSectors;

	  printf("FAT TYPE = %ld\n", d.FAT_Type_Val);
	  printf("FAT #sectors %d\n", d.BPB_TotSec32);
	  printf("RESERVED SECTORS %d \n", d.BPB_RsvdSecCnt);
	  printf("NUM FATS %ld \n", d.BPB_NumFATs);
	  printf("FATZ %ld\n", d.FATSz);
	  printf("ROOT DIR SECTOR %ld \n", d.RootDirSectors);
	  printf("First data sector %ld\n", d.FirstDataSector);
	  printf("FILENAME %s\n", d.BS_OEMName);
	  printf("BYTES PER SEC %d\n", d.BPB_BytsPerSec);
	  printf("SEC PER CLUSTER %d\n", d.BPB_SecPerClus);
	//
	//    // get the root directory
	//    // FirstRootDirSecNum = BPB_ResvdSecCnt + (BPB_NumFATs * BPB_FATSz16)
	//
	long FirstRootDirSecNum = d.BPB_RsvdSecCnt + (d.BPB_NumFATs * d.BPB_FATSz16);
	    printf("FirstRootDirSecNum %ld\n", FirstRootDirSecNum);
	//    // RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec - 1)) / BPB_BytsPerSec;


	for(int i =0 ;i < 20; i++){
		FAT_Device_sector_print_recursive(&d, fp, FirstRootDirSecNum, i, 0);
	}
    char *filename = "test.txt";
    char *content = "DINOMANorld";
    int written_file = FAT_Device_write_file(&d, fp, FirstRootDirSecNum, filename ,content, 11);

    for(int i =0 ;i < 20; i++){
        FAT_Device_sector_print_recursive(&d, fp, FirstRootDirSecNum, i, 0);
    }
    if(written_file != -1){
        printf("File written at %d\n", written_file);
    }
    // remvoe the file
    FAT_Dir_Entry entry = FAT_Device_get_dir(&d, fp, FirstRootDirSecNum, written_file);
    FAT_Device_remove_dir(&d, fp,entry, FirstRootDirSecNum, written_file);
    for(int i =0 ;i < 20; i++){
        FAT_Device_sector_print_recursive(&d, fp, FirstRootDirSecNum, i, 0);
    }
	return d;
}

void FAT_generate_short_name(char *long_filename, char *short_name)
{
    // This is a very simplified version of generating a short filename
    int i = 0;
    for (; i < 6 && long_filename[i] && long_filename[i] != '.'; i++)
    {
        short_name[i] = toupper((unsigned char) long_filename[i]);
    }
    short_name[i++] = '~';
    short_name[i++] = 'A';
    short_name[i] = '\0';
}

void FAT_write_lfn_entry(FAT_Device *d,FILE *fp, int dir_sector, int entry_index, FAT_Dir_Entry_Long *lfn_entry) {
    // Seek to the correct position in the sector and write the LFN entry
    fseek(fp, (dir_sector * d->BPB_BytsPerSec) + (entry_index * sizeof(char) * 32), SEEK_SET);
    fwrite(lfn_entry, sizeof(char), 32, fp);
}

void FAT_write_sfn_entry(FAT_Device *d, FILE *fp, int dir_sector, int entry_index, FAT_Dir_Entry *sfn_entry) {
    // Seek to the correct position in the sector and write the SFN entry
    fseek(fp, (dir_sector * d->BPB_BytsPerSec) +(entry_index * sizeof(char) * 32), SEEK_SET);
    fwrite(sfn_entry, sizeof(char), 32, fp);
}

unsigned int FAT_Device_write_dir_entry(FAT_Device *d, FILE *fp, int dir_sector, int entry_pos, char *long_filename, FAT_Dir_Entry *new_entry) {
    // Calculate the number of LFN entries needed
    size_t len = strlen(long_filename);
    int num_lfn_entries = (len + 12) / 13;  // 13 characters per LFN entry

    // Generate a short filename (Simplified conversion, not handling all edge cases)
    char short_name[12];
    FAT_generate_short_name(long_filename, short_name);

    // Pointer to the last LFN entry to write (we write from last to first)
    int entry_index = num_lfn_entries;
    for(int i =0 ; i < 11; i++){
        new_entry->Base.DIR_Name[i] = short_name[i];
    }
    // Write LFN entries
    for (int i = 0; i < num_lfn_entries; i++) {
        FAT_Dir_Entry_Long lfn_entry;
        memset(&lfn_entry, 0, sizeof(FAT_Dir_Entry_Long));
        lfn_entry.LDIR_Attr = 0x0F;  // LFN attribute
        lfn_entry.LDIR_Ord = num_lfn_entries - i;  // Order of this LFN entry
        if (i == 0) {
            lfn_entry.LDIR_Ord |= 0x40;  // Last logical LFN entry
        }
        // checksum calculation
        unsigned char checksum = 0;
        for (int i_ch = 0; i_ch < 11; i_ch++) {
            checksum = ((checksum & 1) ? 0x80 : 0) + (checksum >> 1) + short_name[i_ch];
        }

        lfn_entry.LDIR_Chksum = FAT_Dir_Entry_Base_checksum(&new_entry->Base);

        // Fill the name fields
        int start_char = i * 13;
        for (int j = 0; j < 12 && (start_char + j) < len; j++) {
            if( j < 5){
                lfn_entry.LDIR_Name1[j<<1] = long_filename[start_char + j];
                lfn_entry.LDIR_Name1[1 + (j<<1)] = 0;
            } else if(j < 10){
                lfn_entry.LDIR_Name2[(j-5) << 1] = long_filename[start_char + j];
                lfn_entry.LDIR_Name1[1 + ((j-5)<<1)] = 0;
            } else {
                lfn_entry.LDIR_Name3[(j-10) << 1] = long_filename[start_char + j];
                lfn_entry.LDIR_Name1[1 + ((j-10)<<1)] = 0;
            }
        }

        // Write the LFN entry to disk
        FAT_write_lfn_entry(d,fp, dir_sector,  entry_pos + i, &lfn_entry);
        entry_index--;
    }

    // Write the short name entry
    for(int i =0 ; i < 11; i++){
        new_entry->Base.DIR_Name[i] = short_name[i];
    }
    new_entry->Base.DIR_Attr = 0x20;  // Archive attribute, for example

    // Write the SFN entry to disk
    FAT_write_sfn_entry(d,fp, dir_sector, entry_pos + num_lfn_entries, new_entry);
    return entry_pos + num_lfn_entries;
}

unsigned int FAT_Device_allocate_cluster(FAT_Device *d, FILE *fp) {

    unsigned int fat_start_sector = d->BPB_RsvdSecCnt;
    unsigned int fat_sector_count = d->BPB_FATSz16;
    // if FAT32
    if(d->FAT_Type_Val == 32){
        fat_sector_count = d->FATSz;
    }

    unsigned int total_clusters = (d->BPB_TotSec32 - d->BPB_RsvdSecCnt) / d->BPB_SecPerClus;

    for (unsigned int cluter_index = 3; cluter_index < total_clusters; cluter_index++) {
        unsigned int cluster = FAT_Device_get_cluster_entry_val(d, fp, cluter_index);
        if (cluster == 0) {
            FAT_Device_set_cluster_entry_val(d, fp, cluter_index, 0xFFFFFFFF);
            return cluter_index;
        }
    }
    printf("No free clusters available\n");
    exit(1);
    return 0xFFFFFFFF; // Indicate failure to allocate cluster
}

// FIXME: tihs function will only look through the given sector, it will not look through the entire directory, This puts a limit on the number of files that can be stored in a directory
int FAT_Device_find_free_dir_entry(FAT_Device *d, FILE *fp, unsigned int dir_sector, const char *long_filename) {
    size_t name_len = strlen(long_filename);
    unsigned int lfn_entries_needed = (name_len + 12) / 13;  // Each LFN entry can store up to 13 UTF-16 characters
    unsigned int total_entries_needed = lfn_entries_needed + 1;  // Total includes the short file name entry

    int free_entries_found = 0;
    FAT_Dir_Entry entry;
    int entry_index = 0;

    int max_entries = d->BPB_BytsPerSec / 32;
    while (entry_index < max_entries) {
        if (entry.Base.DIR_Name[0] == 0x00 || entry.Base.DIR_Name[0] == 0xE5) {  // Check if entry is free or marked as deleted
            free_entries_found++;
            if (free_entries_found == total_entries_needed) {
                return entry_index - (total_entries_needed - 1);  // Return the index of the first free entry
            }
        } else {
            free_entries_found = 0;  // Reset the counter as we need contiguous entries
        }
        entry = FAT_Device_get_dir(d, fp, dir_sector, entry_index);
        entry_index++;
    }

    return -1;  // No suitable space found
}
void FAT_Device_write_sector(FAT_Device *d, FILE *fp, unsigned int sector, const uint8_t *data, size_t size) {
    if (size > d->BPB_BytsPerSec) {
        fprintf(stderr, "Error: Write size exceeds sector size.\n");
        return;
    }

    // Calculate the byte offset to the target sector
    long offset = (long)sector * d->BPB_BytsPerSec;

    // Seek to the correct location
    if (fseek(fp, offset, SEEK_SET) != 0) {
        perror("Failed to seek to sector");
        return;
    }

    // Write the data to the sector
    if (fwrite(data, 1, size, fp) != size) {
        perror("Failed to write data to sector");
        return;
    }

    // Flush the changes to ensure data is written to the disk or disk image
    fflush(fp);
}
void FAT_Device_set_next_cluster(FAT_Device *d, FILE *fp, unsigned int current_cluster, unsigned int next_cluster) {
    FAT_Device_set_cluster_entry_val(d, fp, current_cluster, next_cluster);
}

// Writes a file to the directory at the given sector
int FAT_Device_write_file(FAT_Device *d, FILE *fp, int dir_sector, uint8_t *filename, const uint8_t *content, size_t content_size) {
    // Step 1: Find a free directory entry in the directory sector
    int entry_pos = FAT_Device_find_free_dir_entry(d, fp, dir_sector, filename);
    if (entry_pos < 0) {
        fprintf(stderr, "No free directory entries available\n");
        return -1;
    }
    printf("Entry position: %d\n", entry_pos);

    // Step 2: Allocate a cluster for the file data
    unsigned int first_cluster = FAT_Device_allocate_cluster(d, fp);
    if (first_cluster == 0xFFFFFFFF) {
        fprintf(stderr, "No free clusters available\n");
        return -1;
    }

    // Step 3: Prepare the directory entry
    FAT_Dir_Entry new_entry;
    memset(&new_entry, 0, sizeof(FAT_Dir_Entry));
    memcpy(new_entry.Base.DIR_Name, filename, 11);
    new_entry.Base.DIR_FstClusLO = first_cluster & 0xFFFF;
    new_entry.Base.DIR_FstClusHI = (first_cluster >> 16) & 0xFFFF;
    new_entry.Base.DIR_FileSize = content_size;
    new_entry.Base.DIR_Attr = 0x20; // Archive attribute

    // Step 4: Write the directory entry
    unsigned int des_entry = FAT_Device_write_dir_entry(d, fp, dir_sector, entry_pos, filename, &new_entry);

    // Step 5: Write file content to the cluster
    unsigned int current_cluster = first_cluster;
    unsigned int bytes_written = 0;
    while (content_size > 0) {
        unsigned int sector_to_write = FAT_Device_first_sector_of_cluster(d, current_cluster);
        size_t bytes_to_write = (content_size > d->BPB_BytsPerSec) ? d->BPB_BytsPerSec : content_size;

        FAT_Device_write_sector(d, fp, sector_to_write, content + bytes_written, bytes_to_write);
        bytes_written += bytes_to_write;
        content_size -= bytes_to_write;

        if (content_size > 0) {
            unsigned int next_cluster = FAT_Device_allocate_cluster(d, fp);
            FAT_Device_set_next_cluster(d, fp, current_cluster, next_cluster);
            current_cluster = next_cluster;
        } else {
            // Mark the end of the cluster chain
            FAT_Device_set_next_cluster(d, fp, current_cluster, 0xFFFFFFFF);
        }
    }

    // load the sector
    FAT_Device_load_sector(d, fp, dir_sector);
    return des_entry;
}

int FAT_Device_sector_print_recursive(FAT_Device *d, FILE *fp, int sector_number, int entry_number, int indent){
	FAT_Dir_Entry first = FAT_Device_get_dir(d, fp, sector_number, entry_number);
	// skip any non directories/ files
	if(first.Base.DIR_Name[0] == 0x00 || first.Base.DIR_Name[0] == 0xE5 || first.Base.DIR_Attr == 0x0F){
		return 0;
	}
	// print out the indent
	for(int i = 0; i < indent; i++){
		putchar('-');
	}
	FAT_Device_print_dir_name(d, fp,  first, sector_number, entry_number);

	// check if its a directory
	if(first.Base.DIR_Attr & 0x10)
	{
		unsigned int cluster = first.Base.DIR_FstClusLO;
		cluster += first.Base.DIR_FstClusHI << 16;
		if(first.Base.DIR_Name[0] == '.'){
			putchar(10);
			return 0;
		}
		//        printf("Cluster Folder %d\n", cluster);
		int sector = FAT_Device_first_sector_of_cluster(d, cluster);
		//        printf("Sector Folder %d\n", sector);
		unsigned int entries = d->BPB_BytsPerSec / 32;
		do{
			sector = FAT_Device_first_sector_of_cluster(d, cluster);
			for(int i =0; i < d->BPB_SecPerClus; i++){
				FAT_Device_load_sector(d, fp, sector + i);
				for(int j = 0; j < entries; j++){
					//                    FAT_Dir_Entry second = FAT_Device_get_dir(d, fp, sector, j);
					FAT_Device_sector_print_recursive(d, fp, sector + i, j, indent + 1);
				}
			}


			cluster = FAT_Device_get_next_cluster(d, fp, cluster);
		}while(cluster != 0xFFFFFFFF);


	} else {
		// this is a file
		unsigned int cluster = first.Base.DIR_FstClusLO;
		cluster += first.Base.DIR_FstClusHI << 16;
		//        printf("Cluster File %d\n", cluster);
		int sector = FAT_Device_first_sector_of_cluster(d, cluster);
		//        printf("Sector File %d\n", sector);
		unsigned int size = first.Base.DIR_FileSize;
		unsigned int num_sectors = 1 + (size / d->BPB_BytsPerSec);
		unsigned int clusters = num_sectors / d->BPB_SecPerClus;
		unsigned int remainder = num_sectors % d->BPB_SecPerClus;
		unsigned int b_cursor = 0;
		for(int j = 0; j < clusters && b_cursor < size; j++){
			sector = FAT_Device_first_sector_of_cluster(d, cluster);
			FAT_Device_load_sector(d, fp, sector);
			for(int k = 0; k < d->BPB_SecPerClus && b_cursor < size; k++){
				for(int l = 0; l < d->BPB_BytsPerSec && b_cursor < size; l++){
					uint8_t *sector_data = d->get_sector(d->sector_cache, sector + k);
					putchar(sector_data[l]);
					b_cursor++;
				}
			}
			cluster = FAT_Device_get_next_cluster(d, fp, cluster);
			if(cluster == 0xFFFFFFFF){
				break;
			}
		}
		if(remainder){
			sector = FAT_Device_first_sector_of_cluster(d, cluster);
			FAT_Device_load_sector(d, fp, sector);
			for(int j = 0; j < d->BPB_BytsPerSec && b_cursor < size; j++){
				uint8_t *sector_data = d->get_sector(d->sector_cache, sector);
				putchar(sector_data[j]);
                b_cursor++;
			}
		}
		putchar(10);
	}
}

int FAT_Device_first_sector_of_cluster(FAT_Device *d, int n) {
	return ((n - 2) * d->BPB_SecPerClus) + d->FirstDataSector;
}

int FAT_Device_get_cluster_sector_number(FAT_Device *d, int cluster_number) {
	if (d->FAT_Type_Val == 16) {
		int FATOffset = cluster_number * 2;
		return d->BPB_RsvdSecCnt + (FATOffset / d->BPB_BytsPerSec);
	}
	if (d->FAT_Type_Val == 32) {
		int FATOffset = cluster_number * 4;
		return d->BPB_RsvdSecCnt + (FATOffset / d->BPB_BytsPerSec);
	}
	int FATOffset = cluster_number + (cluster_number / 2);
	return d->BPB_RsvdSecCnt + (FATOffset / d->BPB_BytsPerSec);
}

unsigned int FAT_Device_delete_cluster_chain(FAT_Device *d, FILE *fp, unsigned int cluster) {
    if(cluster == 0 || cluster == 0xFFFFFFFF){
        return 0;
    }
    unsigned int next_cluster = FAT_Device_get_next_cluster(d, fp, cluster);
    FAT_Device_set_next_cluster(d, fp, cluster, 0);
    return FAT_Device_delete_cluster_chain(d, fp, next_cluster) + 1;
}

unsigned int  FAT_Device_get_next_cluster(FAT_Device *d, FILE *fp, int cluster_number) {
	unsigned int sector_number = FAT_Device_get_cluster_sector_number(d, cluster_number);
	unsigned int sector_offset = FAT_Device_get_cluster_entry_offset(d, cluster_number);
	// load the sector
	FAT_Device_load_sector(d, fp, sector_number);
	unsigned int clust;
	if(d->FAT_Type_Val == 12){
		clust = FAT_Device_Sector_read_to_int(d, fp, sector_number, 0, sector_offset, 2);
		if(cluster_number & 1){
			clust = clust >> 4;
		} else {
			clust = clust & 0x0FFF;
		}
		if(clust >= 0xFF8){
			return 0xFFFFFFFF;
		} else {
			return clust;
		}
	} else if (d->FAT_Type_Val == 16){
		clust = FAT_Device_Sector_read_to_int(d, fp, sector_number, 0, sector_offset, 2);
		if(clust >= 0xFFF8){
			return 0xFFFFFFFF;
		} else {
			return clust;
		}
	} else {
		clust = FAT_Device_Sector_read_to_int(d, fp, sector_number, 0, sector_offset, 4);
		if(clust >= 0x0FFFFFF8){
			return 0xFFFFFFFF;
		} else {
			return clust;
		}
	}
}

int FAT_Device_get_cluster_entry_offset(FAT_Device *d, int cluster_number) {
	return FAT_Device_get_cluster_fat_sector_offset(d,cluster_number) % d->BPB_BytsPerSec;
}

int FAT_Device_get_cluster_fat_sector_offset(FAT_Device *d, int cluster_number) {
	if (d->FAT_Type_Val == 16) {
		int FATOffset = cluster_number * 2;
		return FATOffset - (d->BPB_BytsPerSec * (FATOffset / d->BPB_BytsPerSec));
	}
	if (d->FAT_Type_Val == 32) {
		int FATOffset = cluster_number * 4;
		return FATOffset - (d->BPB_BytsPerSec * (FATOffset / d->BPB_BytsPerSec));
	}
	int FATOffset = cluster_number + (cluster_number / 2);
	return FATOffset - (d->BPB_BytsPerSec * (FATOffset / d->BPB_BytsPerSec));
}

void FAT_Device_load_sector(FAT_Device *d, FILE *fp, int sector_num) {



	uint8_t *sector_data = d->get_sector(d->sector_cache, sector_num);
	// might as well just read it now from file istead of setting to zeros
	// TODO implement global reading functions rather than some FSEEKs and such
	fseek(fp, sector_num * d->BPB_BytsPerSec, SEEK_SET);

	for (int i = 0; i < d->BPB_BytsPerSec; i++) {
		sector_data[i] = getc(fp);
	}
	return;
}

unsigned short FAT_Device_12_get_cluster_entry_val(FAT_Device *d, FILE *fp, int cluster_number) {
	if (d->FAT_Type_Val != 12) {
		printf("wrong fat entry routine called\n");
		exit(1);
	}
	int sector_number = FAT_Device_get_cluster_sector_number(d, cluster_number);
	if(!Cache_contains(d->sector_cache, sector_number)){
		FAT_Device_load_sector(d, fp, sector_number);
	}
	uint8_t *sector_data = d->get_sector(d->sector_cache, sector_number);
	int sector_offset = FAT_Device_get_cluster_fat_sector_offset(d, cluster_number);
	unsigned short cluster_first_entry =
		*((u16 *)&(sector_data[sector_offset]));
	if (cluster_first_entry & 0x1) {
		return cluster_first_entry >> 4;
	} else {
		return cluster_first_entry & 0x0FFF;
	}
}

unsigned short FAT_Device_12_set_cluster_entry_val(FAT_Device *d, FILE *fp, int cluster_number, unsigned short value) {
    if (d->FAT_Type_Val != 12) {
        printf("wrong fat entry routine called\n");
        exit(1);
    }
    int sector_number = FAT_Device_get_cluster_sector_number(d, cluster_number);
    if(!Cache_contains(d->sector_cache, sector_number)){
        FAT_Device_load_sector(d, fp, sector_number);
    }
    uint8_t *sector_data = d->get_sector(d->sector_cache, sector_number);
    int sector_offset = FAT_Device_get_cluster_fat_sector_offset(d, cluster_number);
    unsigned short cluster_first_entry =
        *((u16 *)&(sector_data[sector_offset]));
    if (cluster_number & 1) {
        cluster_first_entry = (cluster_first_entry & 0x0FFF) | (value << 4);
    } else {
        cluster_first_entry = (cluster_first_entry & 0xF000) | value;
    }
    *((u16 *)&(sector_data[sector_offset])) = cluster_first_entry;
    // write to the file also to make sure the changes are saved
    fseek(fp, sector_number * d->BPB_BytsPerSec, SEEK_SET);
    fwrite(sector_data, 1, d->BPB_BytsPerSec, fp);
    return cluster_first_entry;
}

unsigned int FAT_Device_32_get_cluster_entry_val(FAT_Device *d, FILE *fp, int cluster_number) {
	if (d->FAT_Type_Val != 32) {
		printf("wrong fat entry routine called\n");
		exit(1);
	}
	int sector_number = FAT_Device_get_cluster_sector_number(d, cluster_number);
	if(!Cache_contains(d->sector_cache, sector_number)){
		FAT_Device_load_sector(d, fp, sector_number);
	}
	int sector_offset = FAT_Device_get_cluster_fat_sector_offset(d, cluster_number);
	uint8_t *sector_data = d->get_sector(d->sector_cache, sector_number);

	return *((u32 *)&(sector_data[sector_offset])) & 0x0FFFFFFF;
}

unsigned int FAT_Device_32_set_cluster_entry_val(FAT_Device *d, FILE *fp, int cluster_number, unsigned int value) {
    if (d->FAT_Type_Val != 32) {
        printf("wrong fat entry routine called\n");
        exit(1);
    }
    int sector_number = FAT_Device_get_cluster_sector_number(d, cluster_number);
    if(!Cache_contains(d->sector_cache, sector_number)){
        FAT_Device_load_sector(d, fp, sector_number);
    }
    int sector_offset = FAT_Device_get_cluster_fat_sector_offset(d, cluster_number);
    uint8_t *sector_data = d->get_sector(d->sector_cache, sector_number);
    *((u32 *)&(sector_data[sector_offset])) = value & 0x0FFFFFFF;
    // write to the file also to make sure the changes are saved
    fseek(fp, sector_number * d->BPB_BytsPerSec, SEEK_SET);
    fwrite(sector_data, 1, d->BPB_BytsPerSec, fp);
    return value;
}

unsigned short FAT_Device_16_get_cluster_entry_val(FAT_Device *d, FILE *fp, int cluster_number) {
	if (d->FAT_Type_Val != 16) {
		printf("wrong fat entry routine called\n");
		exit(1);
	}
	int sector_number = FAT_Device_get_cluster_sector_number(d, cluster_number);
	if(!Cache_contains(d->sector_cache, sector_number)){
		FAT_Device_load_sector(d, fp, sector_number);
	}
	int sector_offset = FAT_Device_get_cluster_fat_sector_offset(d, cluster_number);
	uint8_t *sector_data = d->get_sector(d->sector_cache, sector_number);

	return *((u16 *)&(sector_data[sector_offset]));
}

unsigned short FAT_Device_16_set_cluster_entry_val(FAT_Device *d, FILE *fp, int cluster_number, unsigned short value) {
    if (d->FAT_Type_Val != 16) {
        printf("wrong fat entry routine called\n");
        exit(1);
    }
    int sector_number = FAT_Device_get_cluster_sector_number(d, cluster_number);
    if(!Cache_contains(d->sector_cache, sector_number)){
        FAT_Device_load_sector(d, fp, sector_number);
    }
    int sector_offset = FAT_Device_get_cluster_fat_sector_offset(d, cluster_number);
    uint8_t *sector_data = d->get_sector(d->sector_cache, sector_number);
    *((u16 *)&(sector_data[sector_offset])) = value;
    // write to the file also to make sure the changes are saved
    fseek(fp, sector_number * d->BPB_BytsPerSec, SEEK_SET);
    fwrite(sector_data, 1, d->BPB_BytsPerSec, fp);
    return value;
}

unsigned int FAT_Device_get_cluster_entry_val(FAT_Device *d, FILE *fp, int cluster_number) {
	if (d->FAT_Type_Val == 12) {
		return FAT_Device_12_get_cluster_entry_val(d, fp, cluster_number);
	}
	if (d->FAT_Type_Val == 16) {
		return FAT_Device_16_get_cluster_entry_val(d, fp, cluster_number);
	}
	if (d->FAT_Type_Val == 32) {
		return FAT_Device_32_get_cluster_entry_val(d, fp, cluster_number);
	}
	printf("FAT TYPE NOT RECOGNIZED\n");
	exit(1);
}

unsigned int FAT_Device_set_cluster_entry_val(FAT_Device *d, FILE *fp, int cluster_number, unsigned int value) {
    if (d->FAT_Type_Val == 12) {
        return FAT_Device_12_set_cluster_entry_val(d, fp, cluster_number, value);
    }
    if (d->FAT_Type_Val == 16) {
        return FAT_Device_16_set_cluster_entry_val(d, fp, cluster_number, value);
    }
    if (d->FAT_Type_Val == 32) {
        return FAT_Device_32_set_cluster_entry_val(d, fp, cluster_number, value);
    }
    printf("FAT TYPE NOT RECOGNIZED\n");
    exit(1);
}

void FAT_Device_Sector_read_to_char_arr(FAT_Device *d, FILE *fp, int sector_number, unsigned char *source, unsigned int start_offset, unsigned int offset, unsigned int size) {
	if(!Cache_contains(d->sector_cache, sector_number)){
		FAT_Device_load_sector(d, fp, sector_number);
	}
	uint8_t *sector_data = d->get_sector(d->sector_cache, sector_number);
	for (int i = 0; i < size; i++) {
		source[i] = sector_data[start_offset + offset + i];
	}
}

unsigned int FAT_Device_Sector_read_to_int(FAT_Device *d, FILE *fp, int sector_number,unsigned int start_offset, unsigned int offset, unsigned int size) {
	if(!Cache_contains(d->sector_cache, sector_number)){
		FAT_Device_load_sector(d, fp, sector_number);
	}
	unsigned int result = 0;
	uint8_t *sector_data = d->get_sector(d->sector_cache, sector_number);
	for (int i = 0; i < size; i++) {
		result += (unsigned char)sector_data[start_offset + offset + i] << (i * 8);
	}
	return result;
}

FAT_Dir_Entry FAT_Device_get_dir(FAT_Device *d, FILE *fp, int sector_number, int entry_number)
{
	FAT_Dir_Entry_Base e;
	FAT_Device_load_sector(d, fp, sector_number);

	unsigned int offset = 32 * entry_number;
	FAT_Device_Sector_read_to_char_arr(d, fp, sector_number, e.DIR_Name,offset, 0, 11);
	e.DIR_Attr = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 11, 1);
	e.DIR_NTRes = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 12, 1);
	e.DIR_CrtTimeTenth = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 13, 1);
	e.DIR_CrtTime = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 14, 2);
	e.DIR_CrtDate = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 16, 2);
	e.DIR_LstAccDate = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 18, 2);
	e.DIR_FstClusHI = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 20, 2);
	e.DIR_WrtTime = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 22, 2);
	e.DIR_WrtDate = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 24, 2);
	e.DIR_FstClusLO = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 26, 2);
	e.DIR_FileSize = FAT_Device_Sector_read_to_int(d, fp, sector_number,offset, 28, 4);

	//    putchar(10);
	//    printf("DIR_Attr: %d\n", e.DIR_Attr);
	//    printf("DIR_NTRes: %d\n", e.DIR_NTRes);
	//    printf("DIR_CrtTimeTenth: %d\n", e.DIR_CrtTimeTenth);
	//    printf("DIR_CrtTime: %d\n", e.DIR_CrtTime);
	//    printf("DIR_CrtDate: %d\n", e.DIR_CrtDate);
	//    printf("DIR_LstAccDate: %d\n", e.DIR_LstAccDate);
	//    printf("DIR_FstClusHI: %d\n", e.DIR_FstClusHI);
	//    printf("DIR_WrtTime: %d\n", e.DIR_WrtTime);
	//    printf("DIR_WrtDate: %d\n", e.DIR_WrtDate);
	//    printf("DIR_FstClusLO: %d\n", e.DIR_FstClusLO);
	//    printf("DIR_FileSize: %d\n", e.DIR_FileSize);

	FAT_Dir_Entry result;
	result.Base = e;
	return result;
}

void FAT_Device_set_dir(FAT_Device *d, FILE *fp, int sector_number, int entry_number, FAT_Dir_Entry e)
{
    FAT_write_sfn_entry(d, fp, sector_number, entry_number, &e);
    FAT_Device_load_sector(d, fp, sector_number);
}

unsigned char FAT_Dir_Entry_Base_checksum(FAT_Dir_Entry_Base *e) {
	unsigned char sum = 0;
	for (int i = 0; i < 11; i++) {
		sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + e->DIR_Name[i];
	}
	return sum;
}

unsigned char FAT_Device_print_dir_name(FAT_Device *d, FILE *fp,FAT_Dir_Entry e, int sector_number, int entry_number){
	// if empty dont print 
	if(e.Base.DIR_Name[0] == 0xE5)
	{
		printf("Empty entry\n");
		return 0;
	}
	if(e.Base.DIR_Name[0] == 0x00)
	{
		printf("Empty entry\n");
		return 0;
	}
	// if its a long file name dont print
	if(e.Base.DIR_Attr == 0x0F)
	{
		printf("Long file name\n");
		return 0;
	}

	unsigned char checksum = FAT_Dir_Entry_Base_checksum(&e.Base);
	if(entry_number == 0){
		// print out the dir name as short
		printf("DIR_Name1: ");
		for (int i = 0; i < 11; i++)
		{
			putchar(e.Base.DIR_Name[i]);
		}
		return 1;
	}

	// check if the previous entry is the same file
	FAT_Dir_Entry prev = FAT_Device_get_dir(d, fp, sector_number, entry_number - 1);
	// first check if its a long file name
	if(prev.Base.DIR_Attr == 0x0F)
	{
		// check if the checksum is the same
		if(checksum == prev.Long.LDIR_Chksum)
		{
			// call print long 
			return FAT_Device_print_dir_long_name(d, fp, prev.Long, sector_number, entry_number -1, checksum);
		}

		// print out the dir name as short
		printf("DIR_Name:2 ");
		for (int i = 0; i < 11; i++)
		{
			putchar(e.Base.DIR_Name[i]);
		}
		return 1;

	}

	// print out the dir name as short
	printf("DIR_Name3: ");
	for (int i = 0; i < 11; i++)
	{
		putchar(e.Base.DIR_Name[i]);
	}
	return 1;
}

unsigned char FAT_Device_print_dir_long_name(FAT_Device *d, FILE *fp,FAT_Dir_Entry_Long l, int sector_number, int entry_number, unsigned char checksum){
	// check that l is a long file name
	while(1){
		if(l.LDIR_Attr != 0x0F)
		{
			printf("Error, not a long file name\n");
			return 0;
		}
		if(l.LDIR_Chksum != checksum)
		{
			printf("Checksum does not match\n");
			return 0;
		}
		// print out the name!
		for(int i = 0; i < 10; i+=2)
		{
			putchar(l.LDIR_Name1[i]);
		}
		for(int i = 0; i < 10; i+=2)
		{
			putchar(l.LDIR_Name2[i]);
		}
		for(int i = 0; i < 4; i+=2)
		{
			putchar(l.LDIR_Name3[i]);
		}
		// check if its the last entry
		if(l.LDIR_Ord & 0x40)
		{
			putchar(10);
			return 1;
		}
		if(entry_number == 0)
		{
			printf("Error, long file name entry is first entry\n");
			return 0;
		}
		// get the prev entry
		FAT_Dir_Entry prev = FAT_Device_get_dir(d, fp, sector_number, entry_number - 1);
		l = prev.Long;
		entry_number = entry_number - 1;
	}
}

unsigned char FAT_Device_remove_dir_long(FAT_Device *d, FILE *fp,FAT_Dir_Entry *l, int sector_number, int entry_number, unsigned char checksum){
    // check that l is a long file name
    while(1){
        if(l->Long.LDIR_Attr != 0x0F)
        {
//            printf("Error, not a long file name\n");
            return 0;
        }
        if(l->Long.LDIR_Chksum != checksum)
        {
//            printf("Checksum does not match\n");
            return 0;
        }

        // check if its the last entry
        if(l->Long.LDIR_Ord & 0x40)
        {
            l->Base.DIR_Name[0] = 0xE5;
            FAT_Device_set_dir(d, fp, sector_number, entry_number, *l);
            return 1;
        }
        if(entry_number == 0)
        {
            printf("Error, long file name entry is first entry\n");
            return 0;
        }
        // get the prev entry
        FAT_Dir_Entry prev = FAT_Device_get_dir(d, fp, sector_number, entry_number - 1);
        l = &prev;
        entry_number = entry_number - 1;
        l->Base.DIR_Name[0] = 0xE5;
        FAT_Device_set_dir(d, fp, sector_number, entry_number, *l);
    }
}

unsigned char FAT_Device_remove_dir(FAT_Device *d, FILE *fp,FAT_Dir_Entry e, int sector_number, int entry_number){
    // if empty dont print
    unsigned int cluster = e.Base.DIR_FstClusLO;
    cluster += e.Base.DIR_FstClusHI << 16;
    if(e.Base.DIR_Name[0] == 0xE5)
    {
        return 0;
    }
    if(e.Base.DIR_Name[0] == 0x00)
    {
        return 0;
    }
    // if its a long file name dont print
    if(e.Base.DIR_Attr == 0x0F)
    {

        e.Base.DIR_Name[0] = 0xE5;
        FAT_Device_set_dir(d, fp, sector_number, entry_number, e);
        FAT_Device_delete_cluster_chain(d, fp, cluster);
        return 0;
    }
    e.Base.DIR_Name[0] =0xE5;
    FAT_Device_set_dir(d, fp, sector_number, entry_number, e);
    FAT_Device_delete_cluster_chain(d, fp, cluster);

    unsigned char checksum = FAT_Dir_Entry_Base_checksum(&e.Base);
    if(entry_number == 0){
        e.Base.DIR_Name[0] =0xE5;
        FAT_Device_set_dir(d, fp, sector_number, entry_number, e);
        FAT_Device_delete_cluster_chain(d, fp, cluster);
        return 1;
    }

    // check if the previous entry is the same file
    FAT_Dir_Entry prev = FAT_Device_get_dir(d, fp, sector_number, entry_number - 1);
    // first check if its a long file name
    if(prev.Base.DIR_Attr == 0x0F)
    {
        // check if the checksum is the same
        if(checksum == prev.Long.LDIR_Chksum)
        {
            // call print long
            return FAT_Device_remove_dir_long(d, fp, &prev, sector_number, entry_number -1, checksum);
        }

        e.Base.DIR_Name[0] =0xE5;
        FAT_Device_set_dir(d, fp, sector_number, entry_number, e);
        FAT_Device_delete_cluster_chain(d, fp, cluster);
        return 1;

    }

    e.Base.DIR_Name[0] =0xE5;
    FAT_Device_set_dir(d, fp, sector_number, entry_number, e);
    FAT_Device_delete_cluster_chain(d, fp, cluster);
    return 1;
}



