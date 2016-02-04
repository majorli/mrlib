/**
 * "mr_string.h"，字符串工具函数库，参照Apache的Java语言字符串工具库StringUtil设计开发。
 *
 * mr_string库默认使用UTF-8字符集，并提供字符串GB18030字符集与UTF-8字符集相互转换的函数，以便支持Windows平台下软件的需要。
 * 为区别C语言的char类型和实际文字的字符，引入“字”的概念，代码中用Char表示，但不设对应的数据类型。
 * C语言的字符串长度，即strlen()函数返回的长度，本库中成为字节长度，用LOB表示。另外引入“字长”的概念，用LOC表示，指字符串中实际字的个数。
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
 * 缩写字符串到最大长度范围内，超出长度部分用英文省略号(...)省略
 * dest:	存放缩写后的目标字符串的首地址，该地址必须有足够的空间，即max_len+1个字符的位置
 * src:		源字符串
 * max_len:	缩写后字符串的最大长度
 *
 * 如果src为NULL，则设置dest为空字符串并返回
 * 如果src的长度小于max_len，则不做任何改变地复制到dest中并返回
 * 如果src的长度大于max_len，则缩写为(substring(str, 0, max-3) + "...")的形式，复制到dest中并返回
 * 如果max_len < 4，且src的长度大于max_len，则dest改写为空字符串并返回
 *
 * 除非dest参数为NULL，否则无论何种情况，本函数确保不会返回NULL指针，也不会修改dest值
extern char *abbreviate(char *dest, const char *src, size_t max_len);
 */

#endif
