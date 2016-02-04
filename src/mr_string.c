#include <iconv.h>
#include <string.h>

#include "mr_common.h"
#include "mr_string.h"

int utf2gb(char *src, size_t slen, char *dest, size_t dlen);
int gb2utf(char *src, size_t slen, char *dest, size_t dlen);
char *c2s(char *dest, const char *src);
char *s2c(char *dest, const char *src);
int scanChar(const char *str, const char *startp);
int scanbChar(const char *str, const char *startp);

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

/**
 * 向后扫描一个字，支持UTF-8字符集
 *
 * UTF-8字符集下，ASCII字符的长度为1字节，扩展字符的长度为2字节，汉字的长度为3字节，CJK扩展字符的长度为4字节，码段如下：
 * 	ASCII		扩展字符	汉字			CJK扩展字符
 *	00-7F		C0-DF|80-BF	E0-EF|(80-BF)*2		F0-F7|(80-BF)*3
 *
 * str:		被扫描的字符串
 * startp:	扫描的起点，str <= startp < str + strlen(str)
 * 
 * 返回:	如果读到有效的字，返回字的字节长度；如果读到的字无效，返回无效部分的字节长度的负值；如果已经抵达字符串末尾，返回0
 */
int scanChar(const char *str, const char *startp)
{
	int ret = 0;
	size_t len; 
	if (str != NULL && startp != NULL && (len = strlen(str)) > 0 && startp >= str && startp < str + len) {
		const char *p = startp;
		unsigned char c0 = *(p++);
		unsigned char c1 = *(p++);
		unsigned char c2 = *(p++);
		unsigned char c3 = *p;
		if (c0 < 0x80) {
			ret = 1;			// ASCII字符
		} else {
			if (IN(c0, 0xc0, 0xdf)) {
				if (IN(c1, 0x80, 0xbf)) {
					ret = 2;	// UTF-8双字节字符
				} else {
					ret = -2;	// 2字节无效字符
				}
			} else if (IN(c0, 0xe0, 0xef)) {
				if (IN(c1, 0x80, 0xbf) && IN(c2, 0x80, 0xbf)) {
					ret = 3;	// UTF-8汉字
				} else {
					ret = -3;	// 3字节无效字符
				}
			} else if (IN(c0, 0xf0, 0xf7)) {
				if (IN(c1, 0x80, 0xbf) && IN(c2, 0x80, 0xbf) && IN(c3, 0x80, 0xbf)) {
					ret = 4;	// UTF-8四字节CJK扩展字符
				} else {
					ret = -4;	// 4字节无效字符
				}
			} else {
				ret = -1;		// 1字节无效字符
			}
		}
	}
	return ret;
}

/**
 * 向前扫描一个字，支持UTF-8字符集
 *
 * UTF-8字符集下，ASCII字符的长度为1字节，扩展字符的长度为2字节，汉字的长度为3字节，CJK扩展字符的长度为4字节，码段如下：
 * 	ASCII		扩展字符	汉字			CJK扩展字符
 *	00-7F		C0-DF|80-BF	E0-EF|(80-BF)*2		F0-F7|(80-BF)*3
 *
 * str:		被扫描的字符串
 * startp:	扫描的起点，str < startp <= str + strlen(str)
 * 
 * 返回:	如果读到有效的字，返回字的字节长度；如果读到的字无效，返回无效部分的字节长度的负值；如果已经抵达字符串开头，返回0
 */
int scanbChar(const char *str, const char *startp)
{
	int ret = 0;
	size_t len;
	if (str != NULL && startp != NULL && (len = strlen(str)) > 0 && startp > str && startp <= str + len) {
		size_t left = startp - str;	// 起点之前的LOB
		const char *p = startp;
		unsigned char c0 = *(--p);
		if (c0 < 0x80) {				// 以下注释中：AS=ASCII字符，后=多字节字符后续字节，二/三/四=多字节字符起始字节，无=无效字节，|=字节分隔，...=省略，^=字符串头部
			ret = 1;				// 1) ...|AS
		} else if (c0 >= 0xc0) {
			ret = -1;				// 2) ...|二,三,四,无 
		} else {					// 3) ...|后
			if (left == 1) {
				ret = -1;				// 4) ^后 ==> ^无
			} else {
				unsigned char c1 = *(--p);
				if (c1 < 0x80) {
					ret = -1;			// 5) ...|AS|后
				} else if (c1 >= 0xe0) {
					ret = -2;			// 6) ...|三,四,无|后续字节
				} else if (c1 >= 0xc0) {
					ret = 2;			// 7) ...|二|后
				} else {				// 8) ...|后|后
					if (left == 2) {
						ret = -2;			// 9) ^后|后 ==> ^无|无
					} else {
						unsigned char c2 = *(--p);
						if (c2 < 0x80) {
							ret = -2;		// 10) ...|AS|后|后
						} else if (c2 >= 0xf0) {
							ret = -3;		// 11) ...|四,无|后|后
						} else if (c2 >= 0xe0) {
							ret = 3;		// 12) ...|三|后|后
						} else if (c2 >= 0xc0) {
							ret = -1;		// 13) ...|二|后|后
						} else {			// 14) ...|后|后|后
							if (left == 3) {
								ret = -3;			// 15) ^后|后|后 ==> ^无|无|无
							} else {
								unsigned char c3 = *(--p);
								if (c3 < 0x80) {
									ret = -3;		// 16) AS|后|后|后
								} else if (c3 >= 0xf8) {
									ret = -4;		// 17) 无|后|后|后
								} else if (c3 >= 0xf0) {
									ret = 4;		// 18) 四|后|后|后
								} else if (c3 >= 0xe0) {
									ret = -1;		// 19) 三|后|后|后
								} else if (c3 >= 0xc0) {
									ret = -2;		// 20) 二|后|后|后
								} else {
									ret = -1;		// 21) 后|后|后|后
								}
							}
						}
					}
				}
			}
		}
	}
	return ret;
}
