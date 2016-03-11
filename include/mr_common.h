/**
 * "mr_common.h"，通用工具函数、常量、宏函数和类型定义
 *
 * Version 2.0.0, 李斌，2016/03/11
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
 * 计算以2为底的对数，n >= 1，n等于0时返回0，其他数值的对数值向下取整
 */
extern unsigned int lg2(unsigned int n);

/**
 * 容器类型的枚举
 */
typedef enum {
	Set,
	Pool,
	HashTable,
	Content,
	Report,
	Network
} ContainerType;

/**
 * 容器结构
 */
typedef struct {
	void *container;		// 容器指针
	ContainerType type;		// 容器类型
} Container_t, *Container;

typedef char *string;			// 字符串类型
typedef void *object;			// 对象类型

/**
 * 元素类型的枚举
 */
typedef enum {
	Integer,
	Real,
	String,
	Object
} ElementType;

/**
 * 容器元素的基本类型
 */
typedef void *Element;

/**
 * 基础数据类型数据的装箱函数，用于将基础类型的数据包装为容器的元素
 * 装箱函数使用malloc()来分配一块内存，所以装箱后获得的元素不再使用后必须free，内存分配失败返回NULL
 */
extern Element integer_inbox(long long value);
extern Element real_inbox(long double value);

/**
 * 基础数据类型元素解包函数，从元素中获取实际的值，元素为NULL时返回0
 */
extern long long integer_outbox(Element element);
extern long double real_outbox(Element element);

/**
 * 元素比较函数的类型定义
 */
typedef int (*CmpFunc)(const Element, const Element);

/**
 * 容器清空时用于处理被清除的节点中元素的处理
 */
typedef void (*OnRemove)(Element);

/**
 * 元素的默认比较函数，NULL指针认为比非NULL指针小，两个NULL指针认为相等
 *
 * d1,d2
 *	用于比较的元素
 *
 * return
 *	Integer: 比较实际的数值大小，返回-1, 0, 或1
 *	Real: 比较实际的数值大小，返回-1, 0, 或1
 *	String: 调用标准库函数strcmp()进行比较并返回其返回值
 *	Object: 比较两个元素的地址，地址相同认为相等并返回0，否则认为不等，根据两者地址位置的先后返回-1或1
 */
extern int integer_cmp(const Element e1, const Element e2);
extern int real_cmp(const Element e1, const Element e2);
extern int string_cmp(const Element e1, const Element e2);
extern int obj_cmp(const Element e1, const Element e2);

/**
 * 根据元素类型获取默认的比较函数
 */
extern CmpFunc default_cmpfunc(ElementType type);

#endif
