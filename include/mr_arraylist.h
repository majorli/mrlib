/**
 * "mr_arraylist.h"，线性表数据结构处理函数
 *
 * mr_arraylist库提供一种基于数组构造的线性表数据结构，用于连续、依次和有序地存放数据项。
 * arraylist中存放的数据项连续存储，并保持固定的存放顺序，删除中间的数据项时会自动移动后续数据项以使存储空间始终保持连续。
 * 当数据项超过当前表的容量时，arraylist会自动扩展容量，但删除数据项时不会自动释放空余的表空间
 *
 * Version 0.0.1, 李斌，2016/02/22
 */
#ifndef MR_ARRAYLIST_H
#define MR_ARRAYLIST_H

#define ARRAYLIST_SECTION_LEN 20

typedef struct {
	void **elements;
	size_t capacity;
	size_t size;
} arraylist_t, *arraylist_p;

extern arraylist_p arraylist_create(void);

extern int arraylist_isempty(arraylist_p alp);

extern size_t arraylist_size(arraylist_p alp);

extern void *arraylist_get(arraylist_p alp, size_t index);

extern int arraylist_append(arraylist_p alp, void *element);

extern int arraylist_add(arraylist_p alp, void *element, size_t index);

extern void *arraylist_remove(arraylist_p alp, size_t index);

extern int arraylist_clear(arraylist_p alp);

extern int arraylist_contains(arraylist_p alp, void *element);

extern int arraylist_search(arraylist_p alp, void *element, Comparator comp);

extern void arraylist_sort(arraylist_p alp, Comparator comp);

#endif
