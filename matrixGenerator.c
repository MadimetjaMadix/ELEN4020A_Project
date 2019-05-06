#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

void createMarix(int N, char* filename)
{	
	int number_of_elements = N*N;
	FILE * file;
	file = fopen(filename,"w");
	fprintf(file, "%d",N);
	
	for(int i=0; i<number_of_elements; ++i)
	{
		fprintf(file, " %d",rand()%99);
	}
	
	fclose(file);
}

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

int main()
{
	int n[5]= {8, 16, 32, 64, 128};
	char nsize[10];
	
	
	for(int i = 0; i<5;i++)
	{
		char* filename = "matrixFile_";
		int N = n[i];
		
		sprintf(nsize,"%d",N);
		
		filename = concat(filename,nsize);
		
		createMarix(N,filename);
	
	}
	return 0;
}
