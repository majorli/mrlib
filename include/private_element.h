/**
 * private_element.h 用于元素操作的内部函数
 */

#ifndef PRIVATE_ELEMENT_H
#define PRIVATE_ELEMENT_H

#include <stdlib.h>
#include <string.h>

#include "mr_common.h"

/**
 * 元素的内部结构
 */
typedef struct {
	void *value;
	ElementType type;
	size_t len;
} element_t, *element_p;

/**
 * 创建一个元素，元素值将被复制而非引用
 *
 * value
 *	元素值
 * type
 *	元素类型
 * len
 *	元素长度，此处应提供元素实际类型的真正长度，例如sizeof(int)
 *
 * return
 *	新创建的元素，创建失败返回NULL
 */
extern element_p __element_create(Element value, ElementType type, size_t len);

/**
 * 判断一个元素是否为NULL元素，value == NULL或len == 0为NULL元素
 *
 * element
 *	元素
 *
 * return
 *	是空元素返回1，否则返回0
 */
extern int __element_isnull(element_p element);

/**
 * 销毁一个元素
 *
 * element
 *	待销毁的元素，销毁元素时同时销毁其中的元素值
 */
extern void __element_destroy(element_p element);

/**
 * 通过复制的方式获取一个元素中的值
 *
 * element
 *	元素
 *
 * return
 *	元素的值的副本
 */
extern Element __element_clone_value(element_p element);

/**
 * 获取元素默认的比较函数
 *
 * type
 *	元素的类型
 *
 * return
 *	根据元素类型选择的默认比较函数
 */
extern CmpFunc __default_cmpfunc(ElementType type);

#endif
