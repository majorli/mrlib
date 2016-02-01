/**
 * "mr_common.h"，通用类的工具函数、常量及宏函数，主要用于处理各类字符串操作
 *
 * Version 1.0.0, 李斌，2016/1/28
 */
#ifndef MR_COMMON_H
#define MR_COMMON_H

/**
 * 字符串结尾符
 */
#define EOS '\0'

/**
 * 返回任意类型数组的长度
 */
#define ARRAY_LEN(arr) (sizeof(arr)/sizeof(arr[0]))

/**
 * 判断字符是否为引用符号，包括单引号、双引号、反引号、大中小括号、尖括号
 */
#define IS_QUOTES(ch) ((ch)=='"'||(ch)=='\''||(ch)=='`'||(ch)=='('||(ch)==')'||(ch)=='['||(ch)==']'||(ch)=='{'||(ch)=='}'||(ch)=='<'||(ch)=='>')

/**
 * 判断字符是否为句子分隔符，包括英文分号、句号、问号、感叹号和回车换行
 */
#define IS_STBRKS(ch) ((ch)=='.'||(ch)==';'||(ch)=='\n'||(ch)=='?'||(ch)=='!')

/**
 * 获取字符编码由gbk转换为utf8时目标字符串的可能长度，包括结尾的'\0'字符
 */
#define LEN_TO_UTF8(len) ((len)+((len)>>1)+1)

/**
 * 获取字符编码由utf8转换为gbk时目标字符串的可能长度，包括结尾的'\0'字符
 */
#define LEN_TO_GBK(len) ((len)+1)

/**
 * 常用中文字符集编码名称字符串的宏
 */
#define GBK "GBK"
#define GB18030 "GB18030"
#define BIG5 "BIG5"
#define UTF8 "UTF-8"
#define UTF16 "UTF-16"

/**
 * 转换字符串的字符集编码
 * from_charset: 原字符集
 * to_charset: 目标字符集
 * inbuf: 原字符串
 * inlen: 原字符串要转换字符集的长度
 * outbuf: 目标字符串，必须有足够的长度
 * outlen: 目标字符串最大长度
 * 转换失败返回-1，成功返回0
 */
extern int charset_conv(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen);

/**
 * 将字符串中的字符'\033', '\a', '\b', '\f', '\n', '\r', '\t', '\v'转换为字符串"\\E", "\\a", "\\b", "\\f", "\\n", "\\r", "\\t", "\\v"
 */
extern char *c2s(char *dest, const char *src);

/**
 * 将字符串中的字符串"\\E", "\\a", "\\b", "\\f", "\\n", "\\r", "\\t", "\\v"转换为字符'\033', '\a', '\b', '\f', '\n', '\r', '\t', '\v'
 */
extern char *s2c(char *dest, const char *src);

#endif
