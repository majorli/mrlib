/**
 * "mr_string.h"，字符串工具函数库，参照Apache的Java语言字符串工具库StringUtil设计开发。
 *
 * mr_string库支持UTF-8和GB18030两种字符集，函数参数charset为1表示GB18030编码，为0或其他任何值表示UTF-8编码，也可以使用mr_common.h中的预定义宏。
 * 为区别C语言的char类型和实际文字的字符，引入“字”的概念，代码中用Char表示，但不设对应的数据类型。
 * C语言的字符串长度，即strlen()函数返回的长度，本库中成为字节长度，用LOB表示。另外引入“字长”的概念，用LOC表示，指字符串中实际字的个数。
 *
 * 为确保内存安全，所有函数均不会自行分配内存空间，也不会自行释放已分配的内存空间。
 * 因此在调用库函数时，所有char *dest参数或其他需要传入的用以填写字符串的指针参数需由客户端在调用前完成内存分配，使用完毕后由客户端负责释放内存。
 * 
 * 所有作为函数返回值的字符串指针，均与传入的参数相同。例如abbreviate()的返回值，与调用时传入的char *dest参数相同，这是为了实现客户端链式调用库函数。
 * 上述情况下，除非传入的参数即为NULL指针，否则库函数将确保不会返回NULL，而是返回空字符串，即长度为零的字符串来代替NULL指针。
 *
 * Version 0.0.1, 李斌，2016/02/01
 */
#ifndef MR_STRING_H
#define MR_STRING_H

/**
 * UTF-8/GB18030编码字符串字符数对应的安全字节长度，即字符数的4倍
 */
#define MAX_BYTES(chars) ((chars) + (chars) + (chars) + (chars))

/**
 * UTF-8编码下纯汉字字符串的字符数对应的字节长度
 */
#define UTF8_CHN_BYTES(chars) ((chars) + (chars) + (chars))

/**
 * GB18030编码下纯汉字字符串的字符数对应的字节长度
 */
#define GB18030_CHN_BYTES(chars) ((chars) + (chars))

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
 * 向后扫描一个字，支持UTF-8和GB18030两种字符集。扫描时按字符集码段规则判断是否有效的字，无效字包括不在码段规定范围内的字节或不完整的多字节字
 *
 * UTF-8字符集下，ASCII字符的长度为1字节，扩展字符的长度为2字节，汉字的长度为3字节，CJK扩展字符的长度为4字节，码段如下：
 * 	ASCII		扩展字符	汉字			CJK扩展字符
 *	00-7F		C0-DF|80-BF	E0-EF|(80-BF)*2		F0-F7|(80-BF)*3
 * GB18030字符集下，ASCII字符的长度为1字节，汉字的长度为2字节，少数民族语言字符的长度为4字节，码段如下：
 * 	ASCII		汉字(一)	汉字(二)	少数民族语言字符
 * 	00-7F		81-FE|40-7E	81-FE|80-FE	81-FE|30-39|81-FE|30-39
 *
 * str:		被扫描的字符串
 * startp:	扫描的起点
 * charset:	字符集，1表示GB18030，0或其他值为UTF-8
 * 
 * 返回:	如果读到有效的字，返回字的字节长度；如果读到的字无效，返回无效部分的字节长度的负值；如果已经抵达字符串末尾，返回0
 */
extern int scanChar(const char *str, const char *startp, int charset);

/**
 * 向前扫描一个字，支持UTF-8和GB18030两种字符集。扫描时按字符集码段规则判断是否有效的字，无效字包括不在码段规定范围内的字节或不完整的多字节字
 *
 * UTF-8字符集下，ASCII字符的长度为1字节，扩展字符的长度为2字节，汉字的长度为3字节，CJK扩展字符的长度为4字节，码段如下：
 * 	ASCII		扩展字符	汉字			CJK扩展字符
 *	00-7F		C0-DF|80-BF	E0-EF|(80-BF)*2		F0-F7|(80-BF)*3
 * GB18030字符集下，ASCII字符的长度为1字节，汉字的长度为2字节，少数民族语言字符的长度为4字节，码段如下：
 * 	ASCII		汉字(一)	汉字(二)	少数民族语言字符
 * 	00-7F		81-FE|40-7E	81-FE|80-FE	81-FE|30-39|81-FE|30-39
 *
 * str:		被扫描的字符串
 * startp:	扫描的起点
 * charset:	字符集，1表示GB18030，0或其他值为UTF-8
 * 
 * 返回:	如果读到有效的字，返回字的字节长度；如果读到的字无效，返回无效部分的字节长度的负值；如果已经抵达字符串末尾，返回0
 */
extern int scanbChar(const char *str, const char *startp, int charset);

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
