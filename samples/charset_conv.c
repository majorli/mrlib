#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mr_common.h"
#include "mr_string.h"

/**
 * 中文字符串的编码方式转换，UTF-8与GB18030字符集互转
 */
int main(void)
{
	char *utf = "你好啊！Hello!";			// 测试用字符串，含4个中文字符和6个ASCII字符，字符集UTF-8

	size_t lob_utf = strlen(utf);			// 开始转换为GB18030字符集，获取原字符串的字节长度（LOB）
	size_t lob_gb = UTF2GB_LEN(lob_utf);		// 用宏函数UTF2GB_LEN(slen)来获取目标字符串缓冲区的安全字节长度，该安全字节长度已经包含字符串结尾的'\0'字符
	printf("String in UTF-8 is \"%s\", LOB = %lu.\nBytes: ", utf, lob_utf);
	char *u = utf;
	while (*u) {
		printf("%02hhX ", *(u++));		// 逐个输出字节(UTF-8)
	}
	printf("\n");
	printf("Converting to GB18030...\n");
	char *gb = (char *)malloc(lob_gb * sizeof(char));
	if (utf2gb(utf, lob_utf, gb, lob_gb) != -1)	// 转换为GB18030编码，如果转换过程出错则返回-1，并填充错误代码
		printf("String in GB18030 is \"%s\".\nBytes: ", gb);
	else
		perror("Convert charset error: ");
	char *g = gb;
	while (*g) {
		printf("%02hhX ", *(g++));		// 逐个输出字节(GB18030)
	}
	printf("\n");

	size_t rlob_gb = strlen(gb);			// 转换后的GB18030字符串的实际字节长度(LOB)
	printf("Converting to UTF-8 again...\n");
	size_t lob_utfagain = GB2UTF_LEN(rlob_gb);	// 用宏函数GB2UTF_LEN(slen)来获取目标字符串缓冲区的安全字节长度
	char *utfagain = (char *)malloc(lob_utfagain * sizeof(char));
	if (gb2utf(gb, rlob_gb, utfagain, lob_utfagain) != -1)	// 再次转换回UTF-8编码
		printf("String in UTF-8 again: \"%s\".\nBytes: ", utfagain);
	else
		perror("Convert charset error: ");

	u = utfagain;
	while(*u) {
		printf("%02hhX ", *(u++));		// 逐个输出字节(UTF-8)
	}
	printf("\n");

	free(gb);
	free(utfagain);

	return 0;
}

