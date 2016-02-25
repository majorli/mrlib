#include <iconv.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "mr_common.h"
#include "mr_string.h"
#include "mr_arraylist.h"

int utf2gb(char *src, size_t slen, char *dest, size_t dlen);
int gb2utf(char *src, size_t slen, char *dest, size_t dlen);
char *c2s(char *dest, const char *src);
char *s2c(char *dest, const char *src);
int scanChar(const char *str, const char *startp);
int scanbChar(const char *str, const char *startp);
size_t strlen_c(const char *s);
int strcmp_c(const char *s1, const char *s2);
int strncmp_c(const char *s1, const char *s2, size_t n);
char *substr_b(char *dest, const char *src, size_t start, size_t lob);
char *substr_c(char *dest, const char *src, size_t start, size_t loc);
char *left_b(char *dest, const char *src, size_t n);
char *left_c(char *dest, const char *src, size_t n);
char *right_b(char *dest, const char *src, size_t n);
char *right_c(char *dest, const char *src, size_t n);
char *first_nonspace(const char *str);
char *last_nonspace(const char *str);
char *ltrim(char *dest, const char *src);
char *rtrim(char *dest, const char *src);
char *trim(char *dest, const char *src);
int is_empty(const char *str);
int is_blank(const char *str);

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

/**
 * 获取UTF-8字符串的字数(LOC)，无效字节将被忽略不计
 * s:		目标字符串指针
 *
 * 返回:	字符串s中包含的有效字数，无效字节忽略不计。参数s为NULL时返回0
 */
size_t strlen_c(const char *s)
{
	size_t ret = 0;
	if (s != NULL && strlen(s) > 0) {
		int step = 0;
		const char *p = s;
		while ((step = scanChar(s, p)) != 0) {
			if (step < 0) {
				p -= step;
			} else {
				p += step;
				ret++;
			}
		}
	}
	return ret;
}

/**
 * 按中文拼音顺序比较字符串，传入NULL指针视为最小的字符串，比空字符串更小，NULL比任何非NULL字符串小1
 * s1:		待比较的字符串指针一
 * s2:		待比较的字符串指针二
 *
 * 返回:	s1 == s2时返回0，s1 > s2时返回一个正数，s1 < s2时返回一个负数
 */
int strcmp_c(const char *s1, const char *s2)
{
	int ret = 0;
	if (s1 != s2) {
		if (s1 == NULL) {
			ret = -1;
		} else {
			if (s2 == NULL) {
				ret = 1;
			} else {
				size_t slen1 = strlen(s1);
				size_t slen2 = strlen(s2);
				size_t dlen1 = UTF2GB_LEN(slen1);
				size_t dlen2 = UTF2GB_LEN(slen2);
				char *d1 = (char *)malloc(dlen1*sizeof(char));
				char *d2 = (char *)malloc(dlen2*sizeof(char));
				if (utf2gb((char *)s1, slen1, d1, dlen1) == 0 && utf2gb((char *)s2, slen2, d2, dlen2) ==0) {
					ret = strcmp(d1, d2);	// 转换为GB18030编码进行中文顺序比较
				} else {
					ret = 0;		// 有字符串转换为GB18030失败，无法比较中文顺序，视为相等
				}
				free(d1);
				free(d2);
			}
		}
	}
	return ret;
}

/**
 * 按中文拼音顺序比较字符串的前n个字，传入NULL指针视为最小的字符串，比空字符串更小，NULL比任何非NULL字符串小1
 * s1:		待比较的字符串指针一
 * s2:		待比较的字符串指针二
 * n:		用以比较的字数(LOC)
 *
 * 返回:	s1 == s2时返回0，s1 > s2时返回一个正数，s1 < s2时返回一个负数
 */
int strncmp_c(const char *s1, const char *s2, size_t n)
{
	int ret = 0;
	if (n > 0 && s1 != s2) {
		if (s1 == NULL) {
			ret = -1;
		} else {
			if (s2 == NULL) {
				ret = 1;
			} else {
				size_t lob = MAX_UTF8_LOB(n) + 1;
				char *sn1 = (char *)malloc(lob * sizeof(char));
				char *sn2 = (char *)malloc(lob * sizeof(char));
				sn1 = substr_c(sn1, s1, 0, n);
				sn2 = substr_c(sn2, s2, 0, n);
				size_t slen1 = strlen(sn1);
				size_t slen2 = strlen(sn2);
				size_t dlen1 = UTF2GB_LEN(slen1);
				size_t dlen2 = UTF2GB_LEN(slen2);
				char *d1 = (char *)malloc(dlen1*sizeof(char));
				char *d2 = (char *)malloc(dlen2*sizeof(char));
				if (utf2gb((char *)sn1, slen1, d1, dlen1) == 0 && utf2gb((char *)sn2, slen2, d2, dlen2) ==0) {
					ret = strcmp(d1, d2);	// 转换为GB18030编码进行中文顺序比较
				} else {
					ret = 0;		// 有字符串转换为GB18030失败，无法比较中文顺序，视为相等
				}
				free(d1);
				free(d2);
				free(sn1);
				free(sn2);
			}
		}
	}
	return ret;
}

/**
 * 以字节为单位获取字符串的子串
 * dest:	用于存放子串的目标字符串指针，为NULL时返回NULL，dest必须确保有足够的长度，否则可能发生任何意外的情况
 * src:		原字符串指针，为NULL时子串必然为空字符串
 * start:	子串起始字节位置(LOB)
 * lob:		子串字节长度(LOB)
 *
 * 返回:	子串的起始地址，与传入的参数dest相同。如传入的参数dest为NULL，则返回NULL，否则确保不会返回NULL
 */
char *substr_b(char *dest, const char *src, size_t start, size_t lob)
{
	char *ret = dest;
	if (dest != NULL) {					// 目标地址有效，开始进行复制
		memset(dest, 0, lob + 1);			// 标准库的strncpy()函数不保证添加null字符，所以先在目标地址中填充null字符
		size_t slen;					// slen: 原字符串LOB
		if (lob > 0 && src != NULL && start < (slen = strlen(src))) {
			// 子串长度大于0，原字符串指针有效并且起点在原字符串内，开始子串提取，否则不做任何操作，返回空字符串
			size_t len = lob;			// len:  子字符串LOB
			const char *sp = src + start;		// sp:   复制的起点
			if (slen < start + lob) {
				len = slen - start;		// 起始点加上子串长度超过原字符串的总长度，修正为提取到原字符串结尾
			}
			strncpy(dest, sp, len);			// 开始提取子串
		}
	}
	return ret;
}

/**
 * 以字为单位获取字符串的子串，无效字节将被跳过
 * dest:	用于存放子串的目标字符串指针，为NULL时返回NULL，dest必须确保有足够的长度，否则可能发生任何意外的情况
 * src:		原字符串指针，为NULL时子串必然为空字符串
 * start:	子串起始字位置(LOC)
 * loc:		子串字长度(LOC)
 *
 * 返回:	子串的起始地址，与传入的参数dest相同。如传入的参数dest为NULL，则返回NULL，否则确保不会返回NULL
 */
char *substr_c(char *dest, const char *src, size_t start, size_t loc)
{
	char *ret = dest;
	if (dest != NULL) {					// 目标地址有效，开始进行复制
		char *dp = dest;
		const char *sp = src;
		size_t slen;
		if (loc > 0 && src != NULL && (slen = strlen(src)) > 0) {	// 子串长度大于0，原字符串指针有效并且长度大于0，可以进行复制
			size_t cc = 0;
			int step;
			while (cc < start && (step = scanChar(src, sp)) != 0) {	// 跳过start之前的有效字，抵达start或抵达字符串结尾
				if (step < 0) {
					sp -= step;
				} else {
					sp += step;
					cc++;
				}
			}
			if (sp < src + slen) {			// start小于字符串的LOC，可以复制子串
				cc = 0;
				while (cc < loc && (step = scanChar(src, sp)) != 0) {	// 开始读取从start开始的loc个字，或者到字符串结尾
					if (step < 0) {
						sp -= step;
					} else {
						while (step-- > 0) {
							*(dp++) = *(sp++);
						}
						cc++;
					}
				}
			}
		}
		*dp = EOS;					// 添加最后的NULL字符
	}
	return ret;
}

/**
 * 以字节为单位，获取字符串前n个字节的子串
 * dest:	用于存放子串的目标字符串指针，为NULL时返回NULL，dest必须确保有足够的长度，否则可能发生任何意外的情况
 * src:		原字符串，为NULL时子串必然为空字符串
 * n:		子串字节长度(LOB)
 *
 * 返回:	子串的起始地址，与传入的参数dest相同。如传入的参数dest为NULL，则返回NULL，否则确保不会返回NULL
 */
char *left_b(char *dest, const char *src, size_t n)
{
	return substr_b(dest, src, 0, n);
}

/**
 * 以字为单位，获取字符串前n个字的子串
 * dest:	用于存放子串的目标字符串指针，为NULL时返回NULL，dest必须确保有足够的长度，否则可能发生任何意外的情况
 * src:		原字符串，为NULL时子串必然为空字符串
 * n:		子串字长度(LOC)
 *
 * 返回:	子串的起始地址，与传入的参数dest相同。如传入的参数dest为NULL，则返回NULL，否则确保不会返回NULL
 */
char *left_c(char *dest, const char *src, size_t n)
{
	return substr_c(dest, src, 0, n);
}

/**
 * 以字节为单位，获取字符串尾部n个字节的子串
 * dest:	用于存放子串的目标字符串指针，为NULL时返回NULL，dest必须确保有足够的长度，否则可能发生任何意外的情况
 * src:		原字符串，为NULL时子串必然为空字符串
 * n:		子串字节长度(LOB)
 *
 * 返回:	子串的起始地址，与传入的参数dest相同。如传入的参数dest为NULL，则返回NULL，否则确保不会返回NULL
 */
char *right_b(char *dest, const char *src, size_t n)
{
	char *ret = dest;
	if (dest != NULL) {
		size_t slen;
		if (src != NULL && n > 0 && (slen = strlen(src)) > 0) {
			size_t start = n < slen ? slen - n : 0;
			ret = substr_b(dest, src, start, n);
		}
	}
	return ret;
}

/**
 * 以字为单位，获取字符串前n个字的子串
 * dest:	用于存放子串的目标字符串指针，为NULL时返回NULL，dest必须确保有足够的长度，否则可能发生任何意外的情况
 * src:		原字符串，为NULL时子串必然为空字符串
 * n:		子串字长度(LOC)
 *
 * 返回:	子串的起始地址，与传入的参数dest相同。如传入的参数dest为NULL，则返回NULL，否则确保不会返回NULL
 */
char *right_c(char *dest, const char *src, size_t n)
{
	char *ret = dest;
	if (dest != NULL) {
		size_t sloc;
		if (src != NULL && n > 0 && (sloc = strlen_c(src)) > 0) {
			size_t start = n < sloc ? sloc - n : 0;
			ret = substr_c(dest, src, start, n);
		}
	}
	return ret;
}

/**
 * 找到原字符串中第一个非空白符的位置
 * str:		原字符串
 *
 * 返回:	第一个非空白符的指针，找不到或str==NULL时返回NULL
 */
char *first_nonspace(const char *str)
{
	const char *ret = str;
	if (ret != NULL) {
		while (*ret != EOS && isspace(*ret)) {
			ret++;
		}
		if (*ret == EOS) {
			ret = NULL;
		}
	}
	return (char *)ret;
}

/**
 * 找到原字符串中最后一个非空白符的位置
 * str:		原字符串
 *
 * 返回:	最后一个非空白符的指针，找不到或str==NULL时返回NULL
 */
char *last_nonspace(const char *str)
{
	const char *ret = str;
	if (ret != NULL) {
		ret += strlen(str) - 1;
		while (ret >= str && isspace(*ret)) {
			ret--;
		}
		if (ret < str) {
			ret = NULL;
		}
	}
	return (char *)ret;
}

/**
 * 移除字符串头部的空白字符，原字符串为NULL时返回空字符串
 * dest:	目标字符串的指针，目标缓冲区必须保证有足够的长度
 * src:		原字符符
 *
 * 返回:	移除头部空白字符后的字符串起始地址，与dest相同，如果dest==NULL则返回仍然为NULL
 */
char *ltrim(char *dest, const char *src)
{
	if (dest != NULL) {
		char *p = first_nonspace(src);
		if (p == NULL) {
			*dest = EOS;
		} else {
			strcpy(dest, p);
		}
	}
	return dest;
}

/**
 * 移除字符串尾部的空白字符
 * dest:	目标字符串的指针，目标缓冲区必须保证有足够的长度
 * src:		原字符符
 *
 * 返回:	移除头部空白字符后的字符串起始地址，与dest相同，如果dest==NULL则返回仍然为NULL
 */
char *rtrim(char *dest, const char *src)
{
	if (src != NULL) {
		char *p = last_nonspace(src);
		if (p == NULL) {
			*dest = EOS;
		} else {
			size_t n = p - src + 1;
			strncpy(dest, src, n);
			*(dest + n) = EOS;
		}
	}
	return dest;
}

/**
 * 移除字符串头尾的空白字符
 * dest:	目标字符串的指针，目标缓冲区必须保证有足够的长度
 * src:		原字符符
 *
 * 返回:	移除头部空白字符后的字符串起始地址，与dest相同，如果dest==NULL则返回仍然为NULL
 */
char *trim(char *dest, const char *src)
{
	if (dest != NULL) {
		char *sp = first_nonspace(src);
		if (sp == NULL) {
			*dest = EOS;
		} else {
			char *ep = last_nonspace(src);
			size_t n = ep - sp + 1;
			strncpy(dest, sp, n);
			*(dest + n) = EOS;
		}
	}
	return dest;
}

/**
 * 判断一个字符串是否为空，即指针为NULL或长度为0
 * str:		待判断的字符串
 *
 * 返回:	如果str == NULL或strlen(str) == 0则返回1，否则返回0
 */
int is_empty(const char *str)
{
	return str == NULL || strlen(str) == 0;
}

/**
 * 判断一个字符串是否为空白，即指针为NULL或长度为0或全部由空白符组成
 * str:		待判断的字符串
 *
 * 返回:	如果str == NULL或strlen(str) == 0或全部由空白符组成则返回1，否则返回0
 */
int is_blank(const char *str)
{
	int ret = 1;
	if (str != NULL) {
		const char *p = str;
		while (*p) {
			if (isspace(*p)) {
				p++;
			} else {
				ret = 0;
				break;
			}
		}
	}
	return ret;
}

