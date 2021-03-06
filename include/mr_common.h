/**
 * "mr_common.h"，通用工具函数、常量、宏函数和类型定义
 *
 * Version 2.0.1, 李斌，2016/03/23
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
	Catalogue,
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
 * 获取数值类型元素的真实值，例如: struct NODE s = VALUEOF(element, int);
 */
#define VALUEOF(element, type) (*(type *)(element))

/**
 * 获取指针类型元素的真实值，例如: String s = POINTOF(element, char);
 */
#define POINTOF(element, type) ((type *)(element))

/**
 * 元素比较函数的类型定义，四个参数依次为元素1的值, 元素2的值, 元素1的长度, 元素2的长度
 */
typedef int (*CmpFunc)(const Element, const Element, size_t, size_t);

/**
 * 迭代器
 */
typedef struct Iterator_t *Iterator;

/**
 * 迭代方向
 */
extern const int Forward;
extern const int Reverse;

/**
 * 排序方向
 */
extern const int Asc;
extern const int Desc;

/**
 * 迭代器函数
 */
extern Iterator it_create(void *iterator, Element (*next)(void *), size_t (*remove)(void *), void (*reset)(void *), void (*destroy)(void *));
extern Element it_next(Iterator it);
extern size_t it_remove(Iterator it);
extern void it_reset(Iterator it);
extern void it_destroy(Iterator it);

#endif
