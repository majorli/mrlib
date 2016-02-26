/**
 * "mr_common.h"，通用工具函数、常量及宏函数
 *
 * Version 1.0.2, 李斌，2016/02/22
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
 * 判断一个变量是否在两个边界数字之内（不含边界）的宏函数
 */
#define BTW(x, min, max) (((x) > (min)) && ((x) < (max)))

/**
 * 判断一个变量是否在两个边界数字之外（不含边界）的宏函数
 */
#define OUT(x, min, max) (((x) < (min)) || ((x) > (max)))

/**
 * 判断一个变量是否包含在两个边界数字之间（含边界）的宏函数
 */
#define IN(x, min, max) (((x) >= (min)) && ((x) <= (max)))

/**
 * 判断一个变量是否处于两个边界数字之外（含边界）的宏函数
 */
#define EX(x, min, max) (((x) <= (min)) || ((x) >= (max)))

/**
 * 是否需要提供线程安全，默认为1(True)
 * 提供线程安全将不可避免地降低一些性能，如果客户程序确认只运行于单线程条件下则可以通过设置__MultiThreads__为0(False)来关闭线程安全能力
 */
extern int __MultiThreads__;

/**
 * 容器元素的基本类型
 */
typedef void *Element;

/**
 * 容器元素具体类型枚举
 */
typedef enum {
	Char,
	UChar,
	Short,
	UShort,
	Int,
	UInt,
	Long,
	ULong,
	LLong,
	ULLong,
	Float,
	Double,
	LDouble,
	String,
	Object
} ElementType;

/**
 * 元素比较函数的类型定义
 */
typedef int (*CmpFunc)(void *, void *);

/**
 * (unsigned) char,(unsigned) short,(unsigned) int,(unsigned) long,(unsigned) long long,float,double,long double,string的比较函数，传入数据的指针进行比较
 * NULL指针认为比非NULL指针小，两个NULL指针认为相等
 * d1,d2:	用于比较的数的指针
 *
 * 返回:	两数相等返回0，*d1>*d2返回1，*d1<*d2返回-1
 */
extern int charcmp(void *d1, void *d2);
extern int ucharcmp(void *d1, void *d2);
extern int shortcmp(void *d1, void *d2);
extern int ushortcmp(void *d1, void *d2);
extern int intcmp(void *d1, void *d2);
extern int uintcmp(void *d1, void *d2);
extern int longcmp(void *d1, void *d2);
extern int ulongcmp(void *d1, void *d2);
extern int llcmp(void *d1, void *d2);
extern int ullcmp(void *d1, void *d2);
extern int floatcmp(void *d1, void *d2);
extern int doublecmp(void *d1, void *d2);
extern int ldoublecmp(void *d1, void *d2);
extern int stringcmp(void *d1, void *d2);

/**
 * 在不知道d1,d2指针到底指向什么类型数据的情况下，将其视为通用的对象来进行比较，比较规则如下：
 * d1, d2指针值相同则认为两个对象相同，返回0
 * d1, d2有一个为NULL指针的，认为非NULL指针大于NULL指针，返回-1或1
 * d1, d2均不是NULL指针且不相同的，按指针值大小进行判断，返回-1或1
 */
extern int objcmp(void *d1, void *d2);

/**
 * 根据元素类型获取默认的比较函数
 */
extern CmpFunc default_cmpfunc(ElementType type);

/**
 * 容器类型定义，所有容器都采用一个整数(0或者正整数)作为句柄
 */
typedef int ArrayList;
typedef int LinkedList;
typedef int HashSet;
typedef int LinkedHashSet;
typedef int Stack;
typedef int Queue;
typedef int CircularQueue;
typedef int HashMap;
typedef int LinkedHashMap;
typedef int BTree;
typedef int Tree;

/**
 * 容器中元素的数据类型
 */
typedef void *Element;

#endif
