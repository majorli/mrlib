/**
 * "mr_common.h"，通用类的工具函数、常量及宏函数，主要用于处理各类字符串操作
 *
 * 李斌，2016/1/28
 */
#ifndef MR_COMMON_H
#define MR_COMMON_H

/**
 * 字符串类型
 */
typedef char *String;

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
 * utf8编码字符串转为gbk编码字符串
 */
extern String u2g(const String str);					// 

/**
 * gbk编码字符串转为utf8编码字符串
 */
extern String g2u(const String str);					//

/**
 * 将字符串中的'\033', '\n', '\t'字符和空格符' '转换为控制字符串"\\E", "\\n", "\\t", "\\s"
 */
extern String c2s(const String str);					//

/**
 * 将字符串中的控制字符串"\\E", "\\n", "\\t", "\\s"转换为实际字符'\033', '\n', '\t'和空格符' '
 */
extern String s2c(const String str);					//

#endif
