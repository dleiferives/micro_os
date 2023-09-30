#include <stdio.h>
#include <stdlib.h>
#include "fat_helper.h"

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
	
	// go to fat name
	for(int i=0; i<3;i++)
	{
		getc(fptr);
	}

	printf("The name of this fat device: ");
	for(int i =0; i < 8; i++)
	{
		putchar(getc(fptr));
	}
	putchar(10);





	return 0;
}
