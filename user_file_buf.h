#ifndef USER_FILE_BUF_H
#define USER_FILE_BUF_H

#include <stdint.h>

#define USE_FILE_BUFFER

typedef struct
{
	uint32_t	   fSize;
	const uint8_t  *ptr;
	uint32_t       ptr_idx;
	const uint8_t  *base;

}FILE_BUF;

extern FILE_BUF *fopen_buf(const uint8_t *buf_src, uint32_t buf_len);
extern uint32_t fread_buf(FILE_BUF *file_buf, uint8_t *dst, uint32_t count);
extern uint32_t fseek_buf(FILE_BUF *file_buf, uint32_t count);
extern void fclose_buf(FILE_BUF *file_buf);

#endif

