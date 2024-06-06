#include <stdio.h>
#include <stdlib.h>
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
    printf("%c",c);
}

void EX_ERROR(uint8_t *str){
    ANSI_String_Set(&ANSI_ERROR);
    EX_PRINT_STR(str);
    ANSI_Reset();
}
typedef enum{
    Command_LS_k,
    Command_Unknown_k,
}Command_k;
void Command_LS(FAT_Device *device, FAT_Directory *dir, FILE* fp);

void Exec_Command(FAT_Device *device, FAT_Directory *dir, FILE* fp, Command_k command){
    switch(command){
        case Command_LS_k:
            Command_LS(device, dir, fp);
            break;
        case Command_Unknown_k:
            EX_ERROR("Unknown command\n");
            break;
    }
    EX_PRINT_CHAR('\n');
    EX_PRINT_CHAR('\r');
}

void Command_LS(FAT_Device *device, FAT_Directory *dir, FILE* fp){
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

    FAT_Directory_collect(&dir, &d, fptr);
    Exec_Command(&d, &dir, fptr, Command_LS_k);

	return 0;
}
