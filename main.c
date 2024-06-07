#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fat_helper.h"
#include "lru_cache.h"
#include "ansi_repl.h"

//TODO: verify BPB_Bytes_Per_Sector * BPB_Sectors_Per_Cluster <= 1024 * 32;
//TODO: verify sector[510] and sector[511] are equal to 0xAA



void EX_PRINT_STR(uint8_t *str){
    printf("%s",str);
}

void EX_PRINT_STR_LEN(uint8_t *str, uint32_t len){
    for(uint32_t i = 0; i < len; i++){
        printf("%c",str[i]);
    }
}

void EX_PRINT_CHAR(uint8_t c){
    if(c != 0xFF) printf("%c",c);
}

void EX_ERROR(uint8_t *str){
    ANSI_String_Set(&ANSI_ERROR);
    EX_PRINT_STR(str);
    ANSI_Reset();
}
typedef enum{
    Command_LS_k,
    Command_CD_k,
    Command_CAT_k,
    Command_TOUCH_k,
    Command_MKDIR_k,
    Command_RM_k,
    Command_Unknown_k,
}Command_k;
void Command_LS(FAT_Device *device, FAT_Directory *dir, FILE* fp);

void Command_CD(FAT_Device *device, FAT_Directory *dir, FILE* fp, uint8_t *path);

void Command_CAT(FAT_Device *device, FAT_Directory *dir, FILE* fp, uint8_t *path);

void Command_TOUCH(FAT_Device *device, FAT_Directory *dir, FILE* fp, uint8_t *path, uint8_t *data, uint32_t data_len);

void Command_MKDIR(FAT_Device *device, FAT_Directory *dir, FILE* fp, uint8_t *path);

void Command_RM(FAT_Device *device, FAT_Directory *dir, FILE* fp, uint8_t *path);

void Exec_Command_Data(FAT_Device *device, FAT_Directory *dir, FILE* fp, uint8_t *path, uint8_t *data, uint32_t data_len, Command_k command){
    switch(command){
        case Command_LS_k:
            Command_LS(device, dir, fp);
            EX_PRINT_CHAR('\n');
            EX_PRINT_CHAR('\r');
            break;
        case Command_CD_k:
            Command_CD(device, dir, fp, path);
            break;
        case Command_CAT_k:
            Command_CAT(device, dir, fp, path);
            EX_PRINT_CHAR('\n');
            EX_PRINT_CHAR('\r');
            break;
        case Command_TOUCH_k:
            Command_TOUCH(device, dir, fp, path, data, data_len);
            EX_PRINT_CHAR('\n');
            EX_PRINT_CHAR('\r');
            break;
        case Command_MKDIR_k:
            Command_MKDIR(device, dir, fp, path);
            break;
        case Command_RM_k:
            Command_RM(device, dir, fp, path);
            break;
        case Command_Unknown_k:
            EX_ERROR("Unknown command\n");
            break;
    }
}

void Exec_Command(FAT_Device *device, FAT_Directory *dir, FILE* fp, uint8_t *path, Command_k command){
    switch(command){
        case Command_TOUCH_k:
        case Command_Unknown_k:
            EX_ERROR("Unknown command\n");
            break;
        default:
            Exec_Command_Data(device, dir, fp, path, NULL, 0, command);
            break;
    }
}


// ls
void Command_LS(FAT_Device *device, FAT_Directory *dir, FILE* fp){
        FAT_Directory_collect(dir, device, fp);
        for(int i = 0; i < dir->size; i++){
            FAT_Dir_Entry entry = FAT_Device_get_dir(device, fp, dir->locations[i].sector, dir->locations[i].entry);
            if(entry.Base.DIR_Attr & 0x10){
                ANSI_String_Set(&ANSI_FOLDER);
                }else{
                ANSI_String_Set(&ANSI_FILE);
            }
            FAT_Device_print_dir_name(
                    device,
                    fp,
                    entry,
                    dir->locations[i].sector,
                    dir->locations[i].entry
            );
            ANSI_Reset();
            EX_PRINT_CHAR(' ');
        }
}

// cd [path]
void Command_CD(FAT_Device *device, FAT_Directory *dir, FILE* fp, uint8_t *path){
    FAT_Directory_collect(dir, device, fp);
    int32_t loc = FAT_Device_get_dir_location(device, fp, dir, path);
    if(loc == -1){
        EX_ERROR("Directory not found\n\r");
        return;
    }
    FAT_Dir_Entry entry = FAT_Device_get_dir(device, fp, dir->locations[loc].sector, dir->locations[loc].entry);
    if(!(entry.Base.DIR_Attr & 0x10)){
        EX_ERROR("Not a directory\n\r");
        return;
    }
    // get the cluster of the directory
    uint32_t cluster = entry.Base.DIR_FstClusHI << 16 | entry.Base.DIR_FstClusLO;
    dir->cluster = cluster;
    return;
}

// cat [path]
void Command_CAT(FAT_Device *device, FAT_Directory *dir, FILE* fp, uint8_t *path){
    FAT_Directory_collect(dir, device, fp);
    int32_t loc = FAT_Device_get_dir_location(device, fp, dir, path);
    if(loc == -1){
        EX_ERROR("File not found");
        return;
    }
    FAT_Dir_Entry entry = FAT_Device_get_dir(device, fp, dir->locations[loc].sector, dir->locations[loc].entry);
    if(entry.Base.DIR_Attr & 0x10){
        EX_ERROR("Not a file");
        return;
    }
    uint32_t cluster = entry.Base.DIR_FstClusHI << 16 | entry.Base.DIR_FstClusLO;
    uint32_t size = entry.Base.DIR_FileSize;
    FAT_Device_print_file_contents(device, fp, cluster, size);
}

// touch [path] [data]
void Command_TOUCH(FAT_Device *device, FAT_Directory *dir, FILE* fp, uint8_t *path, uint8_t *data, uint32_t data_len){
    FAT_Directory_collect(dir, device, fp);
//    int32_t loc = FAT_Device_get_dir_location(device, fp, dir, path);
//    if(loc != -1){
//        EX_ERROR("File already exists");
//        return;
//    }
    uint32_t sector;
    if(dir->cluster == 0){
        sector = device->BPB_RsvdSecCnt + (device->BPB_NumFATs * device->BPB_FATSz16);
    } else {
        sector = FAT_Device_get_cluster_sector_number(device, dir->cluster);
    }
    FAT_Device_write_file(device, fp, sector, path, data, data_len);
}

// mkdir [path]
void Command_MKDIR(FAT_Device *device, FAT_Directory *dir, FILE* fp, uint8_t *path){
    FAT_Directory_collect(dir, device, fp);
    int32_t loc = FAT_Device_get_dir_location(device, fp, dir, path);
    if(loc != -1){
        EX_ERROR("Directory already exists\n\r");
        return;
    }
    FAT_Device_create_directory(device, fp, path, dir->cluster);
}

// rm [path]
void Command_RM(FAT_Device *device, FAT_Directory *dir, FILE* fp, uint8_t *path){
    FAT_Directory_collect(dir, device, fp);
    int32_t loc = FAT_Device_get_dir_location(device, fp, dir, path);
    if(loc == -1){
        EX_ERROR("File not found\n\r");
        return;
    }
    FAT_Dir_Entry entry = FAT_Device_get_dir(device, fp, dir->locations[loc].sector, dir->locations[loc].entry);
    FAT_Device_remove_dir(device, fp, entry, dir->locations[loc].sector, dir->locations[loc].entry);
    if(entry.Base.DIR_Attr & 0x10){
        EX_ERROR("If dir was not empty, disk is now corrupted\n\r");
    }
}


int main(int argc, char *argv[])
{
	FILE *fptr;
	if(argc != 2)
	{
		fprintf(stderr, "No file specified, if the file does not exist one will be made.\nPlease specify like so: %s $filename$\n",argv[0]);
		exit(0);
	}

	fptr = fopen(argv[1],"rb+");

	if(!fptr)
	{
		printf("file does not exit!\n");
		return 0;
		printf("file does not exist!, do you want to make one? y/n: ");
		char c = getchar();
		putchar(10);
		if(c == 'n') return 0;
		fptr = fopen(argv[1],"wb");
		if(!fptr)
		{
			printf("error making file. try again\n");
		}
		printf("file %s created\n", argv[1]);
	}
	uint32_t keys[10] = {0};
	uint8_t values[10*1024] = {0};
	uint32_t key_rets[10] = {0};
	Cache_t cache = {
		.capacity = 10,
		.size = 0,
		.keys = &keys[0],
		.values = &values[0],
		.key_rets = &key_rets[0],
		.value_capacity = 1024,
	};
	
	FAT_Device d = FAT_Device_init(fptr, &cache);
    FAT_Dir_Entry_Location locations[100] = {0};
    FAT_Directory dir = {
            .cluster = 0,
            .locations = &locations[0],
            .size = 0,
            .capacity = 100,
    };
//    Exec_Command(&d, &dir, fptr, NULL, Command_LS_k);
////    Exec_Command(&d, &dir, fptr, "waluigi", Command_CD_k);
////    Exec_Command(&d, &dir, fptr, NULL, Command_LS_k);
////    Exec_Command(&d, &dir, fptr, "..", Command_CD_k);
////    Exec_Command(&d, &dir, fptr, NULL, Command_LS_k);
//    Exec_Command(&d, &dir, fptr, "test2.txt", Command_CAT_k);
//    Exec_Command_Data(&d, &dir, fptr, "test2.txt","dinosaurs are cool",18, Command_TOUCH_k);
//    Exec_Command(&d, &dir, fptr, "test2.txt", Command_CAT_k);
    char cmd[256];
    char path[256];
    uint8_t data[1024];
    while (1) {
        printf("> ");
        fgets(cmd, sizeof(cmd), stdin);

        // Parse command and execute
        if (strncmp(cmd, "exit", 4) == 0) {
            break;
        } else if (strncmp(cmd, "ls", 2) == 0) {
            Exec_Command(&d, &dir, fptr, NULL, Command_LS_k);
        } else if (sscanf(cmd, "cd %255s", path) == 1) {
            Exec_Command(&d, &dir, fptr, path, Command_CD_k);
        } else if (sscanf(cmd, "cat %255s", path) == 1) {
            Exec_Command(&d, &dir, fptr, path, Command_CAT_k);
        } else if (sscanf(cmd, "touch %255s", path) == 1) {
            printf("Enter data (max 1024 characters): ");
            fgets(data, sizeof(data), stdin);
            size_t len = strlen(data);
            if (data[len - 1] == '\n') data[len - 1] = '\0'; // Remove newline
            Exec_Command_Data(&d, &dir, fptr, path, data, strlen(data), Command_TOUCH_k);
        } else if (sscanf(cmd, "mkdir %255s", path) == 1) {
            Exec_Command(&d, &dir, fptr, path, Command_MKDIR_k);
        } else if (sscanf(cmd, "rm %255s", path) == 1) {
            Exec_Command(&d, &dir, fptr, path, Command_RM_k);
        } else {
            printf("Unknown command.\n");
        }
    }
//    Exec_Command(&d, &dir, fptr, "dir", Command_MKDIR_k);
//    Exec_Command(&d, &dir, fptr, NULL, Command_LS_k);

    free(fptr);
	return 0;
}
