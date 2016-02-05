/**
 * "mr_string.h"，字符串工具函数库
 *
 * mr_string库默认使用UTF-8字符集，并提供字符串GB18030字符集与UTF-8字符集相互转换的函数，以便支持Windows平台下软件的需要。
 * 为区别C语言的char类型和多字节编码方式下实际文字的字符，引入“字”的概念，代码中用Char表示，但不设对应的数据类型。
 * C语言的字符串长度，即strlen()函数返回的长度，本库中成为字节长度，用LOB表示。另外引入“字长”的概念，用LOC表示，指字符串中实际字的个数。
 * 库函数中同一函数有针对两种字符计数方式的不同版本的，在函数名中用后缀"_b", "_c"来进行区分，如"substr_b()"和"substr_c()"。
 * 本库提供部分标准库函数的LOC计数方式版本，在标准库函数名后添加后缀"_b"表示，如"strlen_b()"。
 *
 * 为确保内存安全，所有函数均不会自行分配内存空间，也不会自行释放已分配的内存空间。
 * 调用库函数时，所有用于填写字符串并返回的char*参数均必须由调用者在调用前自行分配内存，并在使用完毕后自行释放。
 * 可以使用宏函数MAX_UTF8_LOB(loc)或MAX_CHN_LOB(loc)来获取UTF-8字符串LOC对应的安全LOB值。
 *
 * Version 0.0.2, 李斌，2016/02/04
 */
#ifndef MR_STRING_H
#define MR_STRING_H

/**
 * UTF-8编码字符串LOC对应的安全LOB，即LOC的4倍
 */
#define MAX_UTF8_LOB(chars) ((chars) + (chars) + (chars) + (chars))

/**
 * UTF-8编码纯汉字字符串LOC对应的安全LOB
 */
#define MAX_CHN_LOB(chars) ((chars) + (chars) + (chars))

/**
 * UTF-8字符串转GB18030时，目标字符串缓冲区的安全字节长度，含结尾的0字符
 */
#define UTF2GB_LEN(slen) ((slen) + 1)

/**
 * GB18030字符串转UTF-8时，目标字符串缓冲区的安全字节长度，含结尾的0字符
 */
#define GB2UTF_LEN(slen) ((slen) + (slen) + 1)

/**
 * UTF-8字符串转换为GB18030字符串
 * src:		原字符串
 * slen:	原字符串的字节长度，即strlen(src)
 * dest:	目标字符串起始地址，必须有足够的字节长度，一般取原字符串字节长度加1即可
 * dlen:	目标字符串字节长度，含结尾的0字符
 *
 * 返回:	转换失败返回-1，成功返回0
 */
extern int utf2gb(char *src, size_t slen, char *dest, size_t dlen);

/**
 * GB18030字符串转换为UTF-8字符串
 * src:		原字符串
 * slen:	原字符串的字节长度，即strlen(src)
 * dest:	目标字符串起始地址，必须有足够的字节长度，一般取原字符串字节长度的两倍加1即可
 * dlen:	目标字符串字节长度，含结尾的0字符
 *
 * 返回:	转换失败返回-1，成功返回0
 */
extern int gb2utf(char *src, size_t slen, char *dest, size_t dlen);

/**
 * 将字符串中的字符'\033', '\a', '\b', '\f', '\n', '\r', '\t', '\v'转换为字符串"\\E", "\\a", "\\b", "\\f", "\\n", "\\r", "\\t", "\\v"
 * dest:	目标字符串首地址，必须有足够的字节长度
 * src:		源字符串
 *
 * 返回:	目标字符串首地址
 */
extern char *c2s(char *dest, const char *src);

/**
 * 将字符串中的字符串"\\E", "\\a", "\\b", "\\f", "\\n", "\\r", "\\t", "\\v"转换为字符'\033', '\a', '\b', '\f', '\n', '\r', '\t', '\v'
 * dest:	目标字符串首地址，必须有足够的字节长度
 * src:		源字符串
 *
 * 返回:	目标字符串首地址
 */
extern char *s2c(char *dest, const char *src);

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
extern int scanChar(const char *str, const char *startp);

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
 * 返回:	如果读到有效的字，返回字的字节长度；如果读到的字无效，返回无效部分的字节长度的负值；如果已经抵达字符串末尾，返回0
 */
extern int scanbChar(const char *str, const char *startp);

/**
 * 获取UTF-8字符串的字数(LOC)，无效字节将被忽略不计
 * s:		目标字符串指针
 *
 * 返回:	字符串s中包含的有效字数，无效字节忽略不计。参数s为NULL时返回0
 */
extern size_t strlen_c(const char *s);

/**
 * 按中文拼音顺序比较字符串，传入NULL指针视为空字符串
 * s1:		待比较的字符串指针一
 * s2:		待比较的字符串指针二
 *
 * 返回:	s1 == s2时返回0，s1 > s2时返回一个正数，s1 < s2时返回一个负数
 */
extern int strcmp_c(const char *s1, const char *s2);

/**
 * 按中文拼音顺序比较字符串的前n个字，传入NULL指针视为空字符串
 * s1:		待比较的字符串指针一
 * s2:		待比较的字符串指针二
 * n:		用以比较的字数(LOC)
 *
 * 返回:	s1 == s2时返回0，s1 > s2时返回一个正数，s1 < s2时返回一个负数
 */
extern int strncmp_c(const char *s1, const char *s2, size_t n);

/**
 * 以字节为单位获取字符串的子串
 * src:		原字符串指针，为NULL时子串必然为空字符串
 * dest:	用于存放子串的目标字符串指针，为NULL时返回NULL
 * start:	子串起始字节位置(LOB)
 * lob:		子串字节长度(LOB)
 *
 * 返回:	子串的起始地址，与传入的参数dest相同。如传入的参数dest为NULL，则返回NULL，否则确保不会返回NULL
 */
extern char *substr_b(const char *src, char *dest, size_t start, size_t lob);

/**
 * 以字为单位获取字符串的子串，无效字节将被跳过
 * src:		原字符串指针，为NULL时子串必然为空字符串
 * dest:	用于存放子串的目标字符串指针，为NULL时返回NULL
 * start:	子串起始字位置(LOC)
 * lob:		子串字长度(LOC)
 *
 * 返回:	子串的起始地址，与传入的参数dest相同。如传入的参数dest为NULL，则返回NULL，否则确保不会返回NULL
 */
extern char *substr_c(const char *src, char *dest, size_t start, size_t loc);

/**
 * 以字节为单位，获取字符串前n个字节的子串
 * src:		原字符串，为NULL时子串必然为空字符串
 * dest:	用于存放子串的目标字符串指针，为NULL时返回NULL
 * n:		子串字节长度(LOB)
 *
 * 返回:	子串的起始地址，与传入的参数dest相同。如传入的参数dest为NULL，则返回NULL，否则确保不会返回NULL
 */
extern char *left_b(const char *src, char *dest, size_t n);

/**
 * 以字为单位，获取字符串前n个字的子串
 * src:		原字符串，为NULL时子串必然为空字符串
 * dest:	用于存放子串的目标字符串指针，为NULL时返回NULL
 * n:		子串字长度(LOC)
 *
 * 返回:	子串的起始地址，与传入的参数dest相同。如传入的参数dest为NULL，则返回NULL，否则确保不会返回NULL
 */
extern char *left_c(const char *src, char *dest, size_t n);

/**
 * 以字节为单位，获取字符串尾部n个字节的子串
 * src:		原字符串，为NULL时子串必然为空字符串
 * dest:	用于存放子串的目标字符串指针，为NULL时返回NULL
 * n:		子串字节长度(LOB)
 *
 * 返回:	子串的起始地址，与传入的参数dest相同。如传入的参数dest为NULL，则返回NULL，否则确保不会返回NULL
 */
extern char *right_b(const char *src, char *dest, size_t n);

/**
 * 以字为单位，获取字符串前n个字的子串
 * src:		原字符串，为NULL时子串必然为空字符串
 * dest:	用于存放子串的目标字符串指针，为NULL时返回NULL
 * n:		子串字长度(LOC)
 *
 * 返回:	子串的起始地址，与传入的参数dest相同。如传入的参数dest为NULL，则返回NULL，否则确保不会返回NULL
 */
extern char *right_c(const char *src, char *dest, size_t n);

#endif
