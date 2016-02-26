#include <stdlib.h>
#include <pthread.h>

#include "mr_containers.h"

typedef struct {							// ArrayList结构
	ElementType type;						// 元素的数据类型
	Element *elements;						// 元素存储区域
	size_t capacity;						// 当前列表总容量
	size_t size;							// 当前列表中元素数量
	CmpFunc cmpfunc;						// 元素比较函数
	pthread_mutex_t mut;						// 共享锁
} al_t, *al_p;

static const size_t SECTION_SIZE = 50;					// 列表每一节的长度

ArrayList al_create(ElementType type, CmpFunc cmpfunc);
int al_destroy(ArrayList al);
int al_isempty(ArrayList al);
size_t al_size(ArrayList al);
Element al_get(ArrayList al, size_t index);
int al_append(ArrayList al, Element ele);
int al_add(ArrayList al, Element ele, size_t index);
void *al_remove(ArrayList al, size_t index);
int al_search(ArrayList al, Element ele);
int al_clear(ArrayList al);
void al_sort(ArrayList al);
void al_comparator(CmpFunc cmpfunc);

/**
 * 创建一个ArrayList，返回句柄
 * type:	元素的类型
 * cmpfunc:	元素比较函数，传入NULL表示采用mr_common.h中定义的与type对应的默认比较函数
 *
 * 返回:	新创建的ArrayList的句柄(一个大于等于0的正整数)，创建失败返回-1
 */
ArrayList al_create(ElementType type, CmpFunc cmpfunc)
{
	ArrayList ret = -1;
	al_p list = (al_p)malloc(sizeof(al_t));
	list->type = type;
	list->elements = (Element *)malloc(SECTION_SIZE * sizeof(Element));
	list->capacity = SECTION_SIZE;
	list->size = 0;
	if (cmpfunc == NULL) {
		list->cmpfunc = default_cmpfunc(type);
	} else {
		list->cmpfunc = cmpfunc;
	}
	ret = container_retrieve(list, ArrayList_t);
	if (ret == -1) {
		free(list->elements);
	} else {
		if (__MultiThreads__ == 1) {
			pthread_mutex_init(&(list->mut), NULL);
		}
	}
	return ret;
}

/**
 * 销毁一个ArrayList，释放列表的空间，但不会销毁其中的元素
 * al:		ArrayList句柄
 *
 * 返回:	销毁完成返回0，销毁失败或无效ArrayList句柄返回-1
 */
int al_destroy(ArrayList al)
{
	int ret = -1;
	al_p list = (al_p)container_release(al, ArrayList_t);
	if (list != NULL) {
		if (__MultiThreads__) {
			pthread_mutex_destroy(&(list->mut));
		}
		free(list);
		ret = 0;
	}
	return ret;
}

/**
 * 判断一个ArrayList是否为空
 * al:		ArrayList句柄
 *
 * 返回:	为空返回0，不为空返回1，无效句柄返回-1
 */
int al_isempty(ArrayList al)
{
	int ret = -1;
	al_p list = (al_p)container_get(al, ArrayList_t);
	if (list != NULL) {
		if (list->size > 0) {
			ret = 1;
		} else {
			ret = 0;
		}
	}
	return ret;
}

/**
 * 获取一个ArrayList中的元素数量
 * al:		ArrayList句柄
 *
 * 返回:	元素数量，空列表或无效句柄返回0
 */
size_t al_size(ArrayList al)
{
	size_t ret = 0;
	al_p list = (al_p)container_get(al, ArrayList_t);
	if (list != NULL) {
		ret = list->size;
	}
	return ret;
}

/**
 * 获取指定位置的元素
 * al:		ArrayList句柄
 * index:	元素位置，从0开始计数到size - 1
 *
 * 返回:	index位置的元素指针，超出0 <= index < size的范围时返回NULL，无效ArrayList句柄返回NULL
 */
Element al_get(ArrayList al, size_t index)
{
	Element ret = NULL;
	al_p list = (al_p)container_get(al, ArrayList_t);
	if (list != NULL && index < list->size) {
		ret = list->elements[index];
	}
	return ret;
}

/**
 * 在列表最后添加一个元素
 * al:		ArrayList句柄
 * ele:		元素，不能为NULL
 *
 * 返回:	元素添加后所在位置，如果元素为NULL或者al句柄无效，或发生其他错误导致添加失败返回-1
 */
int al_append(ArrayList al, Element ele)
{
	int ret = -1;

	return ret;
}

/**
 * 在列表指定位置添加一个元素
 * al:		ArrayList句柄
 * ele:		元素，不能为NULL
 * index:	要插入元素的位置，原本该位置开始直至最后的元素将被向后移动一个位置
 *
 * 返回:	元素插入后所在位置，如果元素为NULL或者al句柄无效，或发生其他错误导致添加失败返回-1
 */
int al_add(ArrayList al, Element ele, size_t index)
{
	int ret = -1;

	return ret;
}

/**
 * 删除指定位置的元素
 * al:		ArrayList句柄
 * index:	要删除的元素位置
 *
 * 返回:	删除的元素，al句柄无效或index超范围时返回NULL
 */
Element al_remove(ArrayList al, size_t index)
{
	Element ret = NULL;

	return ret;
}

/**
 * 从列表中查找一个元素，元素的查找使用列表创建时提供的对象比较函数
 * al:		ArrayList句柄
 * ele:		要查找的元素
 *
 * 返回:	查找到的时候返回元素位置，有多个相同元素时返回最前面的那个，al句柄无效或ele为NULL或查找不到返回-1
 */
int al_search(ArrayList al, Element ele)
{
	int ret = -1;

	return ret;
}

/**
 * 清空列表，清空列表并不会释放列表所用的内存空间
 * al:		ArrayList句柄
 *
 * 返回:	清空成功返回被清空的元素个数，al句柄无效或清空不成功返回-1
 */
int al_clear(ArrayList al)
{
	int ret = -1;

	return ret;
}

/**
 * 列表元素排序，使用列表创建时提供的对象比较函数进行比较，采用快速排序算法
 * al:		ArrayList句柄
 *
 */
void al_sort(ArrayList al)
{
	return;
}

/**
 * 为列表设置或清除元素的数据比较函数
 * cmpfunc:	比较函数，传入NULL表示清除原比较函数改为采用mr_common.h中定义的objcmp()函数
 *
 */
void al_comparator(CmpFunc cmpfunc)
{
	return;
}
