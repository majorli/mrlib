/**
 * mr_element.c，用于处理元素封装、比较、取值的私有函数
 */

#ifndef MR_ELEMENT_C
#define MR_ELEMENT_C

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

element_p __element_create(Element value, ElementType type, size_t len)
{
	element_p e = NULL;
	if ((e = (element_p)malloc(sizeof(element_t)))) {
		if (value) {
			e->type = type;
			switch (type) {
				case integer:
					e->value = malloc(sizeof(Integer));
					memcpy(e->value, value, sizeof(Integer));
					e->len = sizeof(Integer);
					break;
				case real:
					e->value = malloc(sizeof(Real));
					memcpy(e->value, value, sizeof(Real));
					e->len = sizeof(Real);
					break;
				case string:
					e->value = malloc(len + 1);
					e->len = len + 1;
					strncpy(e->value, value, len);
					*(char *)(e->value + len) = '\0';
					break;
				case object:
					if (e->len > 0) {
						e->value = malloc(len);
						e->len = len;
						memcpy(e->value, value, len);
					} else {
						e->value = NULL;
						e->len = 0;
					}
					break;
			}
		} else {
			e->type = object;
			e->value = NULL;
			e->len = 0;
		}
	}
	return e;
}

void __element_destroy(element_p element)
{
	if (!element)
		return;
	free(element->value);
	free(element);
}

Element __element_clone_value(element_p element)
{
	Element ret = NULL;
	if (element && element->value && (ret = malloc(element->len)))
		memcpy(ret, element->value, element->len);
	return ret;
}

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
int int_cmp(const void *e1, const void *e2)
{
	int ret = 0;
	if (e1 != e2) {
		element_p ele1 = (element_p)e1;
		element_p ele2 = (element_p)e2;
		if (ele1 && ele2)
			ret = *(Integer *)ele1->value == *(Integer *)ele2->value ? 0 : *(Integer *)ele1->value > *(Integer *)ele2->value ? 1 : -1;
		else
			ret = ele1 ? 1 : -1;
	}
	return ret;
}

int real_cmp(const void *e1, const void *e2)
{
	int ret = 0;
	if (e1 != e2) {
		element_p ele1 = (element_p)e1;
		element_p ele2 = (element_p)e2;
		if (ele1 && ele2)
			ret = *(Real *)ele1->value == *(Real *)ele2->value ? 0 : *(Real *)ele1->value > *(Real *)ele2->value ? 1 : -1;
		else
			ret = ele1 ? 1 : -1;
	}
	return ret;
}

int str_cmp(const void *e1, const void *e2)
{
	int ret = 0;
	if (e1 != e2) {
		element_p ele1 = (element_p)e1;
		element_p ele2 = (element_p)e2;
		if (ele1 && ele2)
			ret = strcmp((const char *)ele1->value, (const char *)ele2->value);
		else
			ret = ele1 ? 1 : -1;
	}
	return ret;
}

int obj_cmp(const void *e1, const void *e2)
{
	int ret = 0;
	if (e1 != e2) {
		element_p ele1 = (element_p)e1;
		element_p ele2 = (element_p)e2;
		if (ele1 && ele2)
			ret = ele1->len == ele2->len ? memcmp(ele1->value, ele2->value, ele1->len) : ele1->len > ele2->len ? 1 : -1;
		else
			ret = ele1 ? 1 : -1;
	}
	return ret;
}

CmpFunc default_cmpfunc(ElementType type)
{
	CmpFunc ret = NULL;
	switch (type) {
		case integer:
			ret = int_cmp;
			break;
		case real:
			ret = real_cmp;
			break;
		case string:
			ret = str_cmp;
			break;
		default:
			ret = obj_cmp;
	}
	return ret;
}

#endif
