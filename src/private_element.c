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

static int int_cmp(const Element e1, const Element e2, size_t len1, size_t len2);
static int real_cmp(const Element e1, const Element e2, size_t len1, size_t len2);
static int str_cmp(const Element e1, const Element e2, size_t len1, size_t len2);
static int obj_cmp(const Element e1, const Element e2, size_t len1, size_t len2);

element_p __element_create(Element value, ElementType type, size_t len)
{
	element_p e = NULL;
	if ((e = (element_p)malloc(sizeof(element_t)))) {
		if (value && len) {
			e->type = type;
			switch (type) {
				case integer:
					e->value = malloc(sizeof(Integer));
					memset(e->value, 0, sizeof(Integer));
					memcpy(e->value, value, len);
					e->len = sizeof(Integer);
					break;
				case real:
					e->value = malloc(sizeof(Real));
					memset(e->value, 0, sizeof(Integer));
					memcpy(e->value, value, len);
					e->len = sizeof(Real);
					break;
				case string:
					e->value = malloc(len + 1);
					e->len = len + 1;
					strncpy(e->value, value, len);
					*(char *)(e->value + len) = '\0';
					break;
				case object:
					e->value = malloc(len);
					e->len = len;
					memcpy(e->value, value, len);
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

int __element_isnull(element_p element)
{
	return element->value && element->len;
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

CmpFunc __default_cmpfunc(ElementType type)
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

/**
 * 默认的元素比较函数，NULL认为比任何非NULL元素小，两个NULL元素相等
 *
 * d1,d2
 *	用以比较的两个元素
 *
 * return
 *	Integer: 转换为long long型整数比较大小，返回-1, 0, 或1
 *	Real: 转换为long double型浮点数比较大小，返回-1, 0, 或1
 *	String: 调用标准库函数strcmp()比较字符串大小
 *	Object: 首先比较两个元素的实际长度，长度相等时调用标准库函数memcmp()比较字节
 */
static int int_cmp(const Element e1, const Element e2, size_t len1, size_t len2)
{
	int ret = 0;
	if (e1 != e2) {
		if (e1 && e2)
			ret = *(Integer *)e1 == *(Integer *)e2 ? 0 : *(Integer *)e1 > *(Integer *)e2 ? 1 : -1;
		else
			ret = e1 ? 1 : -1;
	}
	return ret;
}

static int real_cmp(const Element e1, const Element e2, size_t len1, size_t len2)
{
	int ret = 0;
	if (e1 != e2) {
		if (e1 && e2)
			ret = *(Real *)e1 == *(Real *)e2 ? 0 : *(Real *)e1 > *(Real *)e2 ? 1 : -1;
		else
			ret = e1 ? 1 : -1;
	}
	return ret;
}

static int str_cmp(const Element e1, const Element e2, size_t len1, size_t len2)
{
	int ret = 0;
	if (e1 != e2) {
		if (e1 && e2)
			ret = strcmp((const char *)e1, (const char *)e2);
		else
			ret = e1 ? 1 : -1;
	}
	return ret;
}

static int obj_cmp(const Element e1, const Element e2, size_t len1, size_t len2)
{
	int ret = 0;
	if (e1 != e2) {
		if (e1 && e2)
			ret = len1 == len2 ? memcmp(e1, e2, len1) : len1 > len2 ? 1 : -1;
		else
			ret = e1 ? 1 : -1;
	}
	return ret;
}
