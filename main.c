#include <stdio.h>


int main()
{
    FILE *fptr;
    fptr = fopen("test1.sd","rb+");

    if(!fptr)
    {
        printf("file does not exist!, do you want to make one? y/n\n");
        char c = getchar();
        putchar(c);
        if(c == 'n') return 0;
        fptr = fopen("test1.sd","wb");
        if(!fptr)
        {
            printf("error making file. try again\n");
        }
    }

    printf("file %s created\n", "test1.sd");


    return 0;
}
