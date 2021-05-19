#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>

#pragma pack(push, 1) //Отключаем выравнивание
typedef struct
{
	//BITMAPFILEHEADER
	uint16_t type; //Должно быть ASCII "BM" для .bmp
	uint32_t file_size; //Размер файла
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t pixel_data_offset; //Расстояние от начала файла до начала массива пикселей
	//DIB header (bitmap information header)
	uint32_t size; //размер хедера
	int32_t width;
	int32_t height;
	uint16_t planes; //Количество палитр (?); должно быть 1
	uint16_t bits_per_pixel; //Количество бит на пиксель
	uint32_t compression; //Сжатие; должно быть 0
	uint32_t image_size; //Размер сжатого изображения. 0, если без сжатия
	int32_t xpels_per_meter; //0 - без предпочтения 
	int32_t ypels_per_meter; //0 - без предпочтения
	uint32_t total_colors; //Количество цветов;если 0, то 2^bits_per_pixel
	uint32_t important_colors; //0
} BITMAPHEADER;
#pragma pack(pop)

//Функция считывает динамически выделяет память под header и data и считывает в них .bmp файл
//Возвращает 0, если успешно
int load_bitmap(char *filename, BITMAPHEADER **header, unsigned char **data)
{
	int image = open(filename, O_RDONLY);
	if(image == -1)
	{
		return -1;
	}

	*header = (BITMAPHEADER*) malloc(sizeof(BITMAPHEADER));
	if(header == NULL)
	{
		close(image);
		return -2;
	}

	int r = read(image, *header, sizeof(BITMAPHEADER));
	if(r != sizeof(BITMAPHEADER))
	{
		close(image);
		free(header);
		return -3;
	}
	
	*data = (unsigned char*) malloc((*header)->file_size - sizeof(BITMAPHEADER));
	if(data == NULL)
	{
		close(image);
		free(header);
		return -3;
	}

	r = read(image, *data, (*header)->file_size - sizeof(BITMAPHEADER));
	if(r != (*header)->file_size - sizeof(BITMAPHEADER))
	{
		close(image);
		free(header);
		free(data);
		return -5;
	}

	close(image);
	return 0;
}

int save_bitmap(char *filename, BITMAPHEADER *header, unsigned char *data)
{
	int image = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR| S_IWUSR);
	if (image == -1)
	{
		return -1;
	}

	int w = write(image, header, sizeof(BITMAPHEADER));
	if(w != sizeof(BITMAPHEADER))
	{
		close(image);
		remove(filename);
		return -2;
	}

	w = write(image, data, header->file_size - header->pixel_data_offset);
	if(w != header->file_size - header->pixel_data_offset)
	{
		close(image);
		remove(filename);
		return -3;
	}
	close(image);

	return 0;
}