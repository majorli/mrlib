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
	List,
	PriorityQueue,
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

typedef long long Integer;		// 整数类型
typedef long double Real;		// 实数类型
typedef char *String;			// 字符串类型
typedef void *Object;			// 对象类型

/**
 * 元素类型的枚举
 */
typedef enum {
	integer,
	real,
	string,
	object
} ElementType;

/**
 * 容器元素的基本类型
 */
typedef void *Element;

/**
 * 获取元素的真实值，例如: struct NODE s = VALUEOF(element, struct NODE);
 */
#define VALUEOF(element, type) (*(type *)(element))

/**
 * 元素比较函数的类型定义
 */
typedef int (*CmpFunc)(const void *, const void *);

/**
 * 迭代器
 */
typedef struct Iterator_t *Iterator;

/**
 * 迭代方向
 */
const int Forward = 1;
const int Reverse = 0;

/**
 * 迭代器函数
 */
extern Iterator it_create(void *iterator, Element (*next)(void *), void (*reset)(void *) void (*destroy)(void *));
extern Element it_next(Iterator it);
extern void it_reset(Iterator it);
extern void it_destroy(Iterator it);

#endif
