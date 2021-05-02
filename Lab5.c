//#define _REENTRANT

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>
#include <math.h>

void sobel_filter(char *imarr, char *resultarr, int pthread_cnt, int *right_border);

void sobel(char *imarr, char *resultarr, int pthread_number, int left_border, int right_border);

int main(int argc, char *argv[])
{
	int rpict;
	int wpict;
	int pthread_cnt;

	if ((argc <= 3) || (access(argv[1], F_OK) == -1))
	{
		printf("INCORRECT INPUT OF ARGUMENTS!\n");
		return -1;
	}

	pthread_cnt = atoi(argv[3]);
	if ((pthread_cnt < 1) || (pthread_cnt > 32))
	{
		printf("INCORRECT INPUT OF PTHREADS!\n");
		return -1;
	}

	rpict = open(argv[1], O_RDONLY);
	if (rpict == -1)
	{
		printf("COULDN\'T OPEN PICTURE FOR READING!\n");
		return -1;
	}
	wpict = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR| S_IWUSR);
	if (wpict == -1)
	{
		printf("COULDN\'T OPEN OR CREATE PICTURE FOR WRITING!\n");
		return -1;
	}

	struct stat file_stat;
	int e = stat(argv[1], &file_stat);
	if (e == -1)
	{
		printf("Some error has occured! %s not found!\n", argv[1]);
	}
	int fsize = file_stat.st_size;
	printf ("%d\n", fsize);

	char *imarr = malloc(fsize);
	char *resultarr = malloc(fsize);
	int *right_border = malloc(pthread_cnt*sizeof(int)); //хранит правую границу массива, который обрабатывает поток
	for (int i = 0; i < pthread_cnt; i++)
	{
		right_border[i] = floor((fsize*(i+1))/pthread_cnt);
	}
	right_border[pthread_cnt-1] -= 1;

	return 0;
}

void sobel_filter(char *imarr, char *resultarr, int pthread_cnt, int *right_border)
{

}

void sobel(char *imarr, char *resultarr, int pthread_number, int left_border, int right_border)
{
	float kernelx[3][3] = {{-1, 0, 1},
						   {-2, 0, 2},
						   {-1, 0, 1}}; 
	float kernely[3][3] = {{-1, 2, -1},
						   {0, 0, 0},
						   {1, 2, 1}}; 		

}