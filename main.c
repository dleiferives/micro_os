#include <stdio.h>
#include <stdlib.h>
#include "fat_helper.h"
#include "lru_cache.h"

//TODO: verify BPB_Bytes_Per_Sector * BPB_Sectors_Per_Cluster <= 1024 * 32;
//TODO: verify sector[510] and sector[511] are equal to 0xAA
	
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

	return 0;
}
