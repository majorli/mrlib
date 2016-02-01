#include <iconv.h>
#include <string.h>

#include "mr_common.h"

int charset_conv(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen);
char *c2s(char *dest, const char *src);
char *s2c(char *dest, const char *src);

/**
 * 转换字符串的字符集编码，暂不提供其他文件调用
 * from_charset: 原字符集
 * to_charset: 目标字符集
 * inbuf: 原字符串
 * inlen: 原字符串要转换字符集的长度
 * outbuf: 目标字符串，必须有足够的长度
 * outlen: 目标字符串最大长度
 * 转换失败返回-1，成功返回0
 */
int charset_conv(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	iconv_t cd;
	int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if (cd == (iconv_t)(-1))
		return -1;
	memset(outbuf, 0, outlen);
	if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
		return -1;
	iconv_close(cd);
	return 0;
}

/**
 * 将字符串中的字符'\033', '\a', '\b', '\f', '\n', '\r', '\t', '\v'转换为字符串"\\E", "\\a", "\\b", "\\f", "\\n", "\\r", "\\t", "\\v"
 * dest字符串必须具有足够的空间
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
 * dest字符串必须具有足够的空间
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

