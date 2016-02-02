#include <iconv.h>
#include <string.h>

#include "mr_common.h"
#include "mr_string.h"

int utf2gb(char *src, size_t slen, char *dest, size_t dlen);
int gb2utf(char *src, size_t slen, char *dest, size_t dlen);
char *c2s(char *dest, const char *src);
char *s2c(char *dest, const char *src);

/**
 * UTF-8字符串转换为GB18030字符串
 * src:		原字符串
 * slen:	原字符串的字节长度，即strlen(src)
 * dest:	目标字符串起始地址，必须有足够的字节长度，一般取原字符串字节长度加1即可
 * dlen:	目标字符串字节长度，含结尾的0字符
 *
 * 转换失败返回-1，成功返回0
 */
int utf2gb(char *src, size_t slen, char *dest, size_t dlen)
{
	iconv_t cd;
	int rc;
	char **pin = &src;
	char **pout = &dest;

	cd = iconv_open("GB18030", "UTF-8");
	if (cd == (iconv_t)(-1))
		return -1;
	memset(dest, 0, dlen);
	if (iconv(cd, pin, &slen, pout, &dlen) == -1)
		return -1;
	iconv_close(cd);
	return 0;
}

/**
 * GB18030字符串转换为UTF-8字符串
 * src:		原字符串
 * slen:	原字符串的字节长度，即strlen(src)
 * dest:	目标字符串起始地址，必须有足够的字节长度，一般取原字符串字节长度的两倍加1即可
 * dlen:	目标字符串字节长度，含结尾的0字符
 *
 * 转换失败返回-1，成功返回0
 */
int gb2utf(char *src, size_t slen, char *dest, size_t dlen)
{
	iconv_t cd;
	int rc;
	char **pin = &src;
	char **pout = &dest;

	cd = iconv_open("UTF-8", "GB18030");
	if (cd == (iconv_t)(-1))
		return -1;
	memset(dest, 0, dlen);
	if (iconv(cd, pin, &slen, pout, &dlen) == -1)
		return -1;
	iconv_close(cd);
	return 0;
}

/**
 * 将字符串中的字符'\033', '\a', '\b', '\f', '\n', '\r', '\t', '\v'转换为字符串"\\E", "\\a", "\\b", "\\f", "\\n", "\\r", "\\t", "\\v"
 * dest:	目标字符串首地址，必须有足够的字节长度
 * src:		源字符串
 *
 * 返回:	目标字符串首地址
 */
char *c2s(char *dest, const char *src)
{
	const char *p = src;
	char *dp = dest;
	int ch;
	while ((ch = *p) != EOS) {
		switch (ch) {
			case '\033':
				*(dp++) = '\\';
				*(dp++) = 'E';
				break;
			case '\a':
				*(dp++) = '\\';
				*(dp++) = 'a';
				break;
			case '\b':
				*(dp++) = '\\';
				*(dp++) = 'b';
				break;
			case '\f':
				*(dp++) = '\\';
				*(dp++) = 'f';
				break;
			case '\n':
				*(dp++) = '\\';
				*(dp++) = 'n';
				break;
			case '\r':
				*(dp++) = '\\';
				*(dp++) = 'r';
				break;
			case '\t':
				*(dp++) = '\\';
				*(dp++) = 't';
				break;
			case '\v':
				*(dp++) = '\\';
				*(dp++) = 'v';
				break;
			default:
				*(dp++) = *p;
		}
		p++;
	}
	*dp = EOS;
	return dest;
}

/**
 * 将字符串中的字符串"\\E", "\\a", "\\b", "\\f", "\\n", "\\r", "\\t", "\\v"转换为字符'\033', '\a', '\b', '\f', '\n', '\r', '\t', '\v'
 * dest:	目标字符串首地址，必须有足够的字节长度
 * src:		源字符串
 *
 * 返回:	目标字符串首地址
 */
char *s2c(char *dest, const char *src)
{
	//int len = strlen(str);
	//char *ret = (char*)malloc((len+1)*sizeof(char));
	//memset(ret, 0, len+1);
	const char *p = src;
	char *dp = dest;
	while(*p) {
		if((*p) != '\\')
			*(dp++) = *(p++);
		else {
			int n = *(++p);
			switch(n) {
				case 'a':
					*(dp++) = '\a';
					break;
				case 'b':
					*(dp++) = '\b';
					break;
				case 'f':
					*(dp++) = '\f';
					break;
				case 'n':
					*(dp++) = '\n';
					break;
				case 't':
					*(dp++) = '\t';
					break;
				case 'r':
					*(dp++) = '\r';
					break;
				case 'v':
					*(dp++) = '\v';
					break;
				case 'E':
					*(dp++) = '\033';
					break;
				default:
					*(dp++) = '\\';
					*(dp++) = n;
			}
			p++;
		}
	}
	*dp = EOS;
	return dest;
}

