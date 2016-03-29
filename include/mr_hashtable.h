/**
 * "mr_hashtable.h"，基于线性探测的三值哈希表
 * 哈希表实现的功能是元素的登记和查询，可以通过一个迭代器进行顺序的读取访问，可以在迭代过程中删除元素，但是不能修改元素
 * 哈希表不保证元素的顺序，也不能进行排序，不支持按索引随机访问
 * 哈希表不可以存入重复元素（元素值相等）或空元素（元素值为NULL或元素长度为0）
 * 哈希表不限制元素的数据类型，可以存放任何类型的元素
 *
 * 2.0.0-DEV, 李斌, 2016/03/29
 */
#ifndef MR_HASHTABLE_H
#define MR_HASHTABLE_H

#include "mr_common.h"

extern Container hash_create(void);

extern int hash_destroy(Container hash);

extern int hash_register(Container hash, Element ele, ElementType type, size_t len);

extern int hash_contains(Container hash, Element ele, ElementType type, size_t len);

extern int hash_remove(Container hash, Element ele, ElementType type, size_t len);

extern int hash_removeall(Container hash);

extern Iterator hash_iterator(Container hash);

#endif
