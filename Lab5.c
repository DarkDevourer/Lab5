//#define _REENTRANT

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <malloc.h>
#include <math.h>
#include "bitmap.h"

void to_bw(int data_size, unsigned char *imarr, int pthread_cnt, int *right_border);

void sobel_filter(unsigned char *imarr, unsigned char *resultarr, int pthread_cnt, int *right_border);

void sobel(unsigned char *imarr, unsigned char *resultarr, int pthread_number, int left_border, int right_border, int width, int height);

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


	BITMAPHEADER *header;
	unsigned char *data;
	load_bitmap(argv[1], &header, &data);
	/*int *right_border = malloc(pthread_cnt*sizeof(int)); //хранит правую границу массива, который обрабатывает поток
	for (int i = 0; i < pthread_cnt; i++)
	{
		right_border[i] = floor((header->file_size*(i+1))/pthread_cnt);
	}
	right_border[pthread_cnt-1] -= 1;*/
	int data_size = header->file_size - header->pixel_data_offset;
	unsigned char *resultarr = (unsigned char*)malloc(data_size);
	printf("%d\n",header->file_size);
	printf("%d\n",header->bits_per_pixel);
	printf("%d\n",data_size);
	printf("%d\n",header->image_size);
	printf("%d\n",header->pixel_data_offset);
	printf("%d\n",header->width);
	printf("%d\n",header->height);
	printf("%d\n",header->size);
	to_bw(data_size, data, pthread_cnt, NULL);
	sobel(data, resultarr,pthread_cnt,0,0,header->width,header->height);

	save_bitmap(argv[2], header, resultarr);

	return 0;
}

void to_bw(int data_size, unsigned char *imarr, int pthread_cnt, int *right_border)
{
	int pixel_cnt = data_size/3;
	for (int i = 0; i < pixel_cnt; i++)
	{
		int a=imarr[i*3];
		int b=imarr[i*3+1];
		int c=imarr[i*3+2];
		unsigned char av = (a+b+c)/3;
		imarr[i*3] = av;
		imarr[i*3+1] = av;
		imarr[i*3+2] = av;
	}
}

void sobel_filter(unsigned char *imarr, unsigned char *resultarr, int pthread_cnt, int *right_border)
{

}

void sobel(unsigned char *imarr, unsigned char *resultarr, int pthread_number, int left_border,int right_border, int width, int height)
{
	char sobel_x[3][3] = {{-1, 0, 1},
						   {-2, 0, 2},
						   {-1, 0, 1}}; 
	char sobel_y[3][3] = {{-1, -2, -1},
						   {0, 0, 0},
						   {1, 2, 1}}; 

	for (int x = 1; x < width-1; x++)
		for (int y = 1; y < height-1; y++)
		{
			unsigned int pixel_x = (sobel_x[0][0] * imarr[3*(x-1+(y-1)*width)]) + (sobel_x[0][1] * imarr[3*(x+(y-1)*width)]) + (sobel_x[0][2] * imarr[3*(x+1+(y-1)*width)]) +
			(sobel_x[1][0] * imarr[3*(x-1+(y)*width)]) + (sobel_x[1][1] * imarr[3*(x+(y)*width)]) + (sobel_x[1][2] * imarr[3*(x+1+(y)*width)]) +
			(sobel_x[2][0] * imarr[3*(x-1+(y+1)*width)]) + (sobel_x[2][1] * imarr[3*(x+(y+1)*width)]) + (sobel_x[2][2] * imarr[3*(x+1+(y+1)*width)]);

			unsigned int pixel_y = (sobel_y[0][0] * imarr[3*(x-1+(y-1)*width)]) + (sobel_y[0][1] * imarr[3*(x+(y-1)*width)]) + (sobel_y[0][2] * imarr[3*(x+1+(y-1)*width)]) +
			(sobel_y[1][0] * imarr[3*(x-1+(y)*width)]) + (sobel_y[1][1] * imarr[3*(x+(y)*width)]) + (sobel_y[1][2] * imarr[3*(x+1+(y)*width)]) +
			(sobel_y[2][0] * imarr[3*(x-1+(y+1)*width)]) + (sobel_y[2][1] * imarr[3*(x+(y+1)*width)]) + (sobel_y[2][2] * imarr[3*(x+1+(y+1)*width)]);
			unsigned int mag = sqrt((pixel_x*pixel_x)+(pixel_y*pixel_y));
			resultarr[3*(x+y*width)] = mag;
			resultarr[3*(x+y*width)+1] = mag;
			resultarr[3*(x+y*width)+2] = mag;
		}

}