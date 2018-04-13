#include "user_file_buf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t read_file_cnt = 0;

//#define FIL_DEBUG

#ifdef FIL_DEBUG
#define fil_printf  printf
#else
#define fil_printf(...)  
#endif

FILE_BUF *fopen_buf(const uint8_t *buf_src, uint32_t buf_len)
{
	fil_printf("fopen_buf!\n");

	FILE_BUF *file_buf = (FILE_BUF *)malloc(sizeof(FILE_BUF));
	if(file_buf)
	{
		file_buf->base    = buf_src;
		file_buf->ptr     = buf_src;
		file_buf->ptr_idx = 0;
		file_buf->fSize   = buf_len;

		return file_buf;
	}
	else
	{
		return 0;
	}
}

uint32_t fread_buf(FILE_BUF *file_buf, uint8_t *dst, uint32_t count)
{
	uint32_t ret = 0;

	fil_printf("fread_buf! count = %4d ", count);

	if( (file_buf->ptr_idx + count) < file_buf->fSize )
	{
		memmove(dst, file_buf->ptr, count);
		file_buf->ptr    += count;
		file_buf->ptr_idx += count;

		fil_printf("0x%02x, 0x%02x ", *(file_buf->ptr), *(file_buf->ptr+1) );
		ret = count;
	}
	else
	{
		int32_t cnt = 0;
		cnt = file_buf->fSize - file_buf->ptr_idx;

		if(cnt > 0)
		{
			memmove(dst, file_buf->ptr, cnt);
			file_buf->ptr	 += cnt;
			file_buf->ptr_idx += cnt;
		}
		else if(cnt < 0)
		{
			printf("fread_buf err!\n");
			while(1);
		}

		ret = cnt;
	}

	read_file_cnt += ret;

	fil_printf("ret = %d\n", ret);
	return ret;
}

uint32_t fseek_buf(FILE_BUF *file_buf, uint32_t count)
{
	uint32_t ret = 0;

	fil_printf("fseek_buf! count = %4d ", count);

	if( (file_buf->ptr_idx + count) < file_buf->fSize )
	{
		file_buf->ptr 	  += count;
		file_buf->ptr_idx += count;

		fil_printf("0x%02x, 0x%02x\n", *(file_buf->ptr), *(file_buf->ptr+1) );
		ret = 0;
	}
	else
	{
		ret = 1;
	}

	return ret;
}

void fclose_buf(FILE_BUF *file_buf)
{
	fil_printf("fclose_buf!\n");
	fil_printf("read_cnt = %d\n", read_file_cnt);

	if(file_buf)
	{
		free(file_buf);
	}
}
