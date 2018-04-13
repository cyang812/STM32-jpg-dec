#include "test_jpg.h"
#include "user_display.h"
#include "user_ram.h"
#include "user_file_buf.h"
#include "example.h"
#include "ok_jpg.h"
#include <stdio.h>

#define JPG_PATH "0:/testjpg.jpg"

FIL pic_file; //fatfs 文件

static uint8_t test_raw_data[4096];

/*
 * 由于ok_jpg 不支持 RGB888, 而alpha 通道又保持为 0xff,
 * 为了节省空间，在 ok_jpg.h 中定义了 JUST_USE_RGB 宏
 * 开启后，输出的数据将不包含 alpha 通道
 */ 

#define COLOR_RGBA8888  1
#define COLOR_RGB888    2

/*
 * 为解码出来的 raw_data 添加 BMP 文件头
 * bmp_header:  指向 BMP 在内存中的地址，该空间前 54 字节为 BMP 头，剩余字节为 raw_data
 * color_type:  颜色模式会影响每个像素点需要用几位来表示，如 RGBA8888 则为32位
 * xsize/ysize: 图片大小
 */
void encodeToBMP(uint8_t *bmp_header, uint8_t color_type, uint32_t xsize, uint32_t ysize)
{
	uint8_t header[54] =
	{
    	0x42, 0x4d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

#ifdef JUST_USE_RGB
    uint32_t file_size = (uint32_t)xsize * (uint32_t)ysize * 3 + 54;
#else
	uint32_t file_size = (uint32_t)xsize * (uint32_t)ysize * 4 + 54;
#endif

    header[2] = (unsigned char)(file_size &0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    uint32_t width = xsize;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) & 0x000000ff;
    header[20] = (width >> 16) & 0x000000ff;
    header[21] = (width >> 24) & 0x000000ff;

    uint32_t height = ysize;
    header[22] = height & 0x000000ff;
    header[23] = (height >> 8) & 0x000000ff;
    header[24] = (height >> 16) & 0x000000ff;
    header[25] = (height >> 24) & 0x000000ff;

	uint8_t biCount = (color_type == COLOR_RGBA8888) ? 0x20 : 0x18;
	header[28] = biCount;

	memmove(bmp_header, header, sizeof(header));
}

/*
 * 使用 fatfs 将解码后的 BMP 数据写成文件
 * data:     待写入文件的 BMP 图片数据
 * data_len: 数据长度
 * item_id:  文件序号
 */
uint8_t writeToBMP(const uint8_t *data, uint32_t data_len, uint32_t item_id)
{
    uint8_t filename_buf[100];
    sprintf((char *)filename_buf, "0:/out%04d.bmp", item_id);

    uint8_t res = f_open(&pic_file, (char const *)filename_buf, FA_WRITE|FA_CREATE_ALWAYS);
    if(res)
    {
        printf("unable creat file!\n");
        return 0;
    }
    else
    {
    	//bmp
        f_write(&pic_file, data, data_len, NULL);
    }

    printf("write file succ, file name = %s, len = %d\n", filename_buf, pic_file.fsize);
    f_close(&pic_file);
}

/*
 * 解码文件中的 jpg 图片，使用 fatfs 管理文件
 * note# 在使用文件是，需要取消 user_file_buf.h 中的 USE_FILE_BUFFER 宏定义
 * jpg_path: jpg图片的路径
 * bmp_buf:  解码输出的 raw_data 数据存放地址，前54字节保留给 bmp 头
 */
uint8_t dec_jpg(char *jpg_path, uint8_t *bmp_buf)
{
	uint8_t ret = f_open(&In_file, (const char *)jpg_path, FA_READ);
	if(ret)
	{
		printf("file name: %s\n", jpg_path);
		printf("open file err! ret = %d\n", ret);
	}

    ok_jpg *image = ok_jpg_read_to_buffer(&In_file, &bmp_buf[54], 0, ok_jpg_decode_flags(OK_JPG_COLOR_FORMAT_BGRA | OK_JPG_FLIP_Y));
    f_close(&In_file);

    if(image->width && image->height)	
    {
    	printf("Got image %s", jpg_path);
        printf(" Size: %li x %li\n", (uint32_t)image->width, (uint32_t)image->height);

		uint32_t xsize = image->width;
		uint32_t ysize = image->height;

#ifdef JUST_USE_RGB		
		uint32_t length = image->width * image->height * 3;
		encodeToBMP(bmp_buf, COLOR_RGB888, xsize, ysize);
#else
		uint32_t length = image->width * image->height * 4;
		encodeToBMP(bmp_buf, COLOR_RGBA8888, xsize, ysize);
#endif
		static uint8_t idx = 0;
		writeToBMP(bmp_buf, length + 54, idx++);
    }
    else
    {
		printf("err msg = %s\n", image->error_message);
    }

    ok_jpg_free(image);
    return 0;
}

/*
 * 解码数据流中的 jpg 图片，使用 user_file_buf.c 中的 api 管理数据
 * note# 在使用数据流时，需要定义 user_file_buf.h 中的 USE_FILE_BUFFER 宏
 * jpg_src: jpg 数据的地址
 * jpg_len: jpg 数据的长度
 * bmp_buf: 解码输出的 raw_data 数据存放地址，前54字节保留给 bmp 头
 */
void dec_jpg_buf(uint8_t *jpg_src, uint32_t jpg_len, uint8_t *bmp_buf)
{
	FILE_BUF *file = fopen_buf(jpg_src, jpg_len);
	if(!file)
	{
		printf("open file err!\n");
	}
	else
	{
		printf("fsize = %d\n", file->fSize);
	}
	
	ok_jpg *image = ok_jpg_read_to_buffer(file, &bmp_buf[54], 0, ok_jpg_decode_flags(OK_JPG_COLOR_FORMAT_BGRA | OK_JPG_FLIP_Y));
    fclose_buf(file);
	
	if(image->width && image->height)
    {
    	printf("Got image ");
        printf(" Size: %li x %li\n", (uint32_t)image->width, (uint32_t)image->height);

		uint32_t xsize = image->width;
		uint32_t ysize = image->height;
		
#ifdef JUST_USE_RGB		
		uint32_t length = image->width * image->height * 3;
		encodeToBMP(bmp_buf, COLOR_RGB888, xsize, ysize);
#else
		uint32_t length = image->width * image->height * 4;
		encodeToBMP(bmp_buf, COLOR_RGBA8888, xsize, ysize);
#endif

		encodeToBMP(bmp_buf, COLOR_RGB888, xsize, ysize);

		static uint8_t idx = 0;
		writeToBMP(bmp_buf, length + 54, idx++);
    }
    else
    {
    	printf("err msg add = 0x%08x\n", image->error_message );
		printf("err msg = %s\n", image->error_message);
    }

    ok_jpg_free(image);

}

/*
 * 解码并显示图片
 * jpg_src: jpg 数据的地址
 * jpg_len: jpg 数据的长度
 * bmp:     bmp 数据的地址
 * x:       显示在屏幕中的 x 坐标
 * y:       显示在屏幕中的 y 坐标
 */
void show_jpg(uint8_t *jpg_src, uint32_t jpg_len, uint8_t *bmp, uint32_t x, uint32_t y)
{
	dec_jpg_buf(jpg_src, jpg_len, bmp);
	LCD_DisplayBmp(x, y, bmp);
}

/*
 * 三个简单的测试
 * example 1: 解码 test_jpg 数组中的 jpg 图像, 解码后的数据存放在 test_raw_data 中, 显示在(50,50)的位置
 * example 2: 解码 t1~t10.jpg 文件中的 jpg 图像, 解码后的数据写回文件
 * example 3: 依次解码并显示 test_jpg1~4, 解码后的数据比较大，因此存放在外置 sdram
 */
void dec_image_test()
{
    //example 1
	show_jpg((uint8_t *)test_jpg, sizeof(test_jpg), test_raw_data, 50, 50);

    //example 2
	char filename[20];
	for(uint8_t i=1; i<=10; i++)
	{
		sprintf((char *)filename, "0:/t%d.jpg", i);
		dec_jpg(filename, sdram_buffer_2);
		printf("\n");
	}

    //example 3
	show_jpg((uint8_t *)test_jpg_1, sizeof(test_jpg_1), sdram_buffer_2, 50, 50);
	HAL_Delay(5000);
	show_jpg((uint8_t *)test_jpg_2, sizeof(test_jpg_2), sdram_buffer_2, 50, 50);
	HAL_Delay(5000);
	show_jpg((uint8_t *)test_jpg_3, sizeof(test_jpg_3), sdram_buffer_2, 50, 50);
	HAL_Delay(5000);
	show_jpg((uint8_t *)test_jpg_4, sizeof(test_jpg_4), sdram_buffer_2, 50, 50);
	HAL_Delay(5000);
}
