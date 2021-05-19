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
#include <time.h>
#include "bitmap.h"

void to_bw(int width, int height, unsigned char *imarr);

void sobel_filter(unsigned char *imarr, unsigned char *resultarr, int pthread_cnt, int width, int height);

void *sobel(void *arg);

typedef struct  //структура аргументов функции потока
{
	 unsigned char *imarr;
	 unsigned char *resultarr;
	 int pthread_number;
	 int begin_line;
	 int end_line;
	 int width;
	 int height;
} thread_args;

int main(int argc, char *argv[])
{
	clock_t start = clock();
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
	int data_size = header->file_size - header->pixel_data_offset;
	unsigned char *resultarr = (unsigned char*)malloc(data_size);
	/*printf("%d\n",header->file_size);
	printf("%d\n",header->bits_per_pixel);
	printf("%d\n",data_size);
	printf("%d\n",header->image_size);
	printf("%d\n",header->pixel_data_offset);
	printf("%d\n",header->width);
	printf("%d\n",header->height);
	printf("%d\n",header->size);*/
	to_bw(header->width, header->height, data);
	clock_t filt = clock();
	sobel_filter(data, resultarr, pthread_cnt,header->width,header->height);

	save_bitmap(argv[2], header, resultarr);

	printf("Время работы программы = %lu миллисекунд\n Время обработки фильтром собела с %d потоками = %lu миллисекунд\n",(clock()-start)*1000/CLOCKS_PER_SEC, pthread_cnt, (clock()-filt)*1000/CLOCKS_PER_SEC);
	free(header);
	free(data);
	free(resultarr);
	return 0;
}

void to_bw(int width, int height, unsigned char *imarr)
{
	for (int x = 0; x < width-1; x++)
		for (int y = 0; y < height-1; y++)
		{
			int a = imarr[3*(x+y*width)];
			int b = imarr[3*(x+y*width)+1];
			int c = imarr[3*(x+y*width)+2];
			unsigned char av = (a+b+c)/3;
			imarr[3*(x+y*width)] = av;
			imarr[3*(x+y*width)+1] = av;
			imarr[3*(x+y*width)+2] = av;
		}
}

void sobel_filter(unsigned char *imarr, unsigned char *resultarr, int pthread_cnt, int width, int height)
{	int begin_line;
	int end_line;
	pthread_t a_thread[pthread_cnt];
	int res;
	void *thread_result;
	thread_args argstr[pthread_cnt];

	for (int i = 0; i < pthread_cnt; i++)
	{
		if (i == 0)
			{
				begin_line = 1;
			}
		else
			{
				begin_line = i*floor(height/pthread_cnt);
			}

		if (i == pthread_cnt - 1)
			{
				end_line = height-2;
			}
		else
			{
				end_line = (i+1)*floor(height/pthread_cnt);
			}

		argstr[i].imarr = imarr;
		argstr[i].resultarr = resultarr;
		argstr[i].pthread_number = i;
		argstr[i].begin_line = begin_line;
		argstr[i].end_line = end_line;
		argstr[i].width = width;
		argstr[i].height = height;
		res = pthread_create(&(a_thread[i]), NULL, sobel, (void*)&argstr[i]);
		if(res !=0)
		{
			perror("Thread creation failed!");
			exit(EXIT_FAILURE);
		}
		
	}

	for(int i = pthread_cnt-1; i >= 0; i--)
	{
		res = pthread_join(a_thread[i],&thread_result);
		if (res != 0)
		{
			perror("Pthread_join failed");
			exit(EXIT_FAILURE);
		}
	}
}

void *sobel(void *arg)
{
	thread_args argt = *(thread_args*)arg;
	char sobel_x[3][3] = {{-1, 0, 1},
						   {-2, 0, 2},
						   {-1, 0, 1}}; 
	char sobel_y[3][3] = {{-1, -2, -1},
						   {0, 0, 0},
						   {1, 2, 1}}; 


	for (int x = 1; x < argt.width-1; x++)
		for (int y = argt.begin_line; y < argt.end_line; y++)
		{
			unsigned int pixel_x = (sobel_x[0][0] * argt.imarr[3*(x-1+(y-1)*argt.width)]) + (sobel_x[0][1] * argt.imarr[3*(x+(y-1)*argt.width)]) + (sobel_x[0][2] * argt.imarr[3*(x+1+(y-1)*argt.width)]) +
			(sobel_x[1][0] * argt.imarr[3*(x-1+(y)*argt.width)]) + (sobel_x[1][1] * argt.imarr[3*(x+(y)*argt.width)]) + (sobel_x[1][2] * argt.imarr[3*(x+1+(y)*argt.width)]) +
			(sobel_x[2][0] * argt.imarr[3*(x-1+(y+1)*argt.width)]) + (sobel_x[2][1] * argt.imarr[3*(x+(y+1)*argt.width)]) + (sobel_x[2][2] * argt.imarr[3*(x+1+(y+1)*argt.width)]);

			unsigned int pixel_y = (sobel_y[0][0] * argt.imarr[3*(x-1+(y-1)*argt.width)]) + (sobel_y[0][1] * argt.imarr[3*(x+(y-1)*argt.width)]) + (sobel_y[0][2] * argt.imarr[3*(x+1+(y-1)*argt.width)]) +
			(sobel_y[1][0] * argt.imarr[3*(x-1+(y)*argt.width)]) + (sobel_y[1][1] * argt.imarr[3*(x+(y)*argt.width)]) + (sobel_y[1][2] * argt.imarr[3*(x+1+(y)*argt.width)]) +
			(sobel_y[2][0] * argt.imarr[3*(x-1+(y+1)*argt.width)]) + (sobel_y[2][1] * argt.imarr[3*(x+(y+1)*argt.width)]) + (sobel_y[2][2] * argt.imarr[3*(x+1+(y+1)*argt.width)]);
			unsigned int mag = sqrt((pixel_x*pixel_x)+(pixel_y*pixel_y));
			argt.resultarr[3*(x+y*argt.width)] = mag;
			argt.resultarr[3*(x+y*argt.width)+1] = mag;
			argt.resultarr[3*(x+y*argt.width)+2] = mag;
		}
	pthread_exit(NULL);
}