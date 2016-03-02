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
Element al_remove(ArrayList al, size_t index);
void al_removeall(ArrayList al, onRemove onremove);
Element al_replace(ArrayList al, Element ele, size_t index);
int al_search(ArrayList al, Element ele);
int al_rsearch(ArrayList al, Element ele);
void al_sort(ArrayList al);
void al_stsort(ArrayList al);
void al_comparator(ArrayList al, CmpFunc cmpfunc);

static size_t __al_expand(al_p list);
static void __al_quicksort(Element *a, int left, int right, CmpFunc cmpfunc);
static void __al_insertionsort(Element *a, int left, int right, CmpFunc cmpfunc);

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
	if (cmpfunc == NULL)
		list->cmpfunc = default_cmpfunc(type);
	else
		list->cmpfunc = cmpfunc;
	ret = container_retrieve(list, ArrayList_t);
	if (ret == -1) {
		free(list->elements);
		free(list);
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
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		free(list->elements);
		if (__MultiThreads__ == 1) {
			pthread_mutex_unlock(&(list->mut));
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
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = (list->size == 0);
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
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
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = list->size;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
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
	if (list != NULL) {
		if (__MultiThreads__ == 1) {
			pthread_mutex_lock(&(list->mut));
		}
		if (index < list->size) {
			ret = list->elements[index];
		}
		if (__MultiThreads__ == 1) {
			pthread_mutex_unlock(&(list->mut));
		}
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
	al_p list = (al_p)container_get(al, ArrayList_t);
	if (list != NULL) {
		if (__MultiThreads__ == 1) {
			pthread_mutex_lock(&(list->mut));
		}
		if (list->size == list->capacity) {
			__al_expand(list);
		}
		list->elements[list->size] = ele;
		ret = list->size;
		list->size++;
		if (__MultiThreads__ == 1) {
			pthread_mutex_unlock(&(list->mut));
		}
	}
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
	al_p list = (al_p)container_get(al, ArrayList_t);
	if (list != NULL) {
		if (__MultiThreads__ == 1) {
			pthread_mutex_lock(&(list->mut));
		}
		if (list->size == list->capacity) {
			__al_expand(list);
		}
		if (index < list->size) {	// 指定的位置在当前列表中间，顺序后移指定位置开始的元素，留出空格存放当前元素
			ret = index;
			size_t p = list->size;
			while (p > ret) {
				list->elements[p] = list->elements[p - 1];
				p--;
			}
		} else {			// 指定的位置在当前列表最后元素之后，直接放入最后一个元素即可
			ret = list->size;
		}
		list->elements[ret] = ele;
		list->size++;
		if (__MultiThreads__ == 1) {
			pthread_mutex_unlock(&(list->mut));
		}
	}
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
	al_p list = (al_p)container_get(al, ArrayList_t);
	if (list != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		if (index < list->size) {
			ret = list->elements[index];		// 待删除的元素将作为返回值返回
			size_t p = index + 1;
			while (p < list->size) {
				list->elements[p - 1] = list->elements[p];
				p++;
			}
			list->size--;
		}
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return ret;
}

/**
 * 删除ArrayList中所有的元素，被清除的元素用onremove函数进行后续处理
 * al:		ArrayList句柄
 * onremove:	元素后续处理函数，NULL表示不做任何处理，典型的可以传入标准库函数free
 *
 */
void al_removeall(ArrayList al, onRemove onremove)
{
	al_p list = (al_p)container_get(al, ArrayList_t);
	if (list != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		if (onremove != NULL)
			for (size_t p = 0; p < list->size; p++)
				onremove(list->elements[p]);
		list->size = 0;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
}

/**
 * 在列表指定位置存储一个元素，覆盖原有的元素，原元素将被返回，其所占的内存空间不会被释放
 * al:		ArrayList句柄
 * ele:		元素，不能为NULL
 * index:	要存储元素的位置，必须在0 <= index < size的范围内
 *
 * 返回:	原元素，如果元素为NULL或者al句柄无效，或index超出范围时返回NULL
 */
Element al_replace(ArrayList al, Element ele, size_t index)
{
	Element ret = NULL;
	al_p list = (al_p)container_get(al, ArrayList_t);
	if (ele != NULL && list != NULL) {
		if (__MultiThreads__ == 1) {
			pthread_mutex_lock(&(list->mut));
		}
		if (index < list->size) {
			ret = list->elements[index];
			list->elements[index] = ele;
		}
		if (__MultiThreads__ == 1) {
			pthread_mutex_unlock(&(list->mut));
		}
	}
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
	al_p list = (al_p)container_get(al, ArrayList_t);
	if (list != NULL && ele != NULL) {
		if (__MultiThreads__ == 1) {
			pthread_mutex_lock(&(list->mut));
		}
		if (list->size > 0) {
			int p;
			for (p = 0; p < list->size; p++) {
				if (list->cmpfunc(list->elements[p], ele) == 0) {
					ret = p;
					break;
				}
			}
		}
		if (__MultiThreads__ == 1) {
			pthread_mutex_unlock(&(list->mut));
		}
	}
	return ret;
}

/**
 * 从列表中逆向查找一个元素，元素的查找使用列表创建时提供的对象比较函数
 * al:		ArrayList句柄
 * ele:		要查找的元素
 *
 * 返回:	查找到的时候返回元素位置，有多个相同元素时返回最后面的那个，al句柄无效或ele为NULL或查找不到返回-1
 */
int al_rsearch(ArrayList al, Element ele)
{
	int ret = -1;
	al_p list = (al_p)container_get(al, ArrayList_t);
	if (list != NULL && ele != NULL) {
		if (__MultiThreads__ == 1) {
			pthread_mutex_lock(&(list->mut));
		}
		if (list->size > 0) {
			int p;
			for (p = list->size - 1; p >= 0; p--) {
				if (list->cmpfunc(list->elements[p], ele) == 0) {
					ret = p;
					break;
				}
			}
		}
		if (__MultiThreads__ == 1) {
			pthread_mutex_unlock(&(list->mut));
		}
	}
	return ret;
}

/**
 * 列表元素排序，使用列表创建时提供的对象比较函数进行比较，采用快速排序算法
 * al:		ArrayList句柄
 *
 */
void al_sort(ArrayList al)
{
	al_p list = (al_p)container_get(al, ArrayList_t);
	if (list != NULL && list->size > 1) {
		if (__MultiThreads__ == 1) {
			pthread_mutex_lock(&(list->mut));
		}
		__al_quicksort(list->elements, 0, list->size - 1, list->cmpfunc);
		if (__MultiThreads__ == 1) {
			pthread_mutex_unlock(&(list->mut));
		}
	}
}

/**
 * 列表元素排序，使用列表创建时提供的对象比较函数进行比较，采用稳定的插入排序算法，适用于需要稳定排序或数据量较小的场合(建议数据量在5000以内)
 * al:		ArrayList句柄
 *
 */
void al_stsort(ArrayList al)
{
	al_p list = (al_p)container_get(al, ArrayList_t);
	if (list != NULL && list->size > 1) {
		if (__MultiThreads__ == 1) {
			pthread_mutex_lock(&(list->mut));
		}
		__al_insertionsort(list->elements, 0, list->size - 1, list->cmpfunc);
		if (__MultiThreads__ == 1) {
			pthread_mutex_unlock(&(list->mut));
		}
	}
}

/**
 * 为列表设置或清除元素的数据比较函数
 * al:		ArrayList句柄
 * cmpfunc:	比较函数，传入NULL表示清除原比较函数改为采用mr_common.h中定义的objcmp()函数
 *
 */
void al_comparator(ArrayList al, CmpFunc cmpfunc)
{
	al_p list = (al_p)container_get(al, ArrayList_t);
	if (list != NULL) {
		if (__MultiThreads__ == 1) {
			pthread_mutex_lock(&(list->mut));
		}
		if (cmpfunc == NULL) {
			list->cmpfunc = default_cmpfunc(list->type);
		} else {
			list->cmpfunc = cmpfunc;
		}
		if (__MultiThreads__ == 1) {
			pthread_mutex_unlock(&(list->mut));
		}
	}
}

/**
 * 扩展列表容量，单位为一个SECTION_SIZE，返回扩展后的容量值
 */
static size_t __al_expand(al_p list)
{
	size_t nc = list->capacity + SECTION_SIZE;
	list->elements = (Element *)realloc(list->elements, nc * sizeof(Element));
	list->capacity = nc;
	return nc;
}

/**
 * 对一组元素进行快速排序
 * a:		待排序元素数组
 * left:	左边界坐标
 * right:	右边界坐标
 * cmpfunc:	比较函数
 *
 */
static void __al_quicksort(Element *a, int left, int right, CmpFunc cmpfunc)
{
	if(left >= right)
		return;
	int i = left;
	int j = right;
	Element key = a[left];
	while (i < j) {
		while (i < j && cmpfunc(key, a[j]) <= 0)
			j--;
		if (i < j)
			a[i++] = a[j];
		while (i < j && cmpfunc(key, a[i]) >= 0)
			i++;
		if (i < j)
			a[j--] = a[i];
	}
	a[i] = key;
	__al_quicksort(a, left, i - 1, cmpfunc);
	__al_quicksort(a, i + 1, right, cmpfunc);
	return;
}

/**
 * 对一组元素进行插入排序
 * a:		待排序元素数组
 * left:	左边界坐标
 * right:	右边界坐标
 * cmpfunc:	比较函数
 *
 */
static void __al_insertionsort(Element *a, int left, int right, CmpFunc cmpfunc)
{
	int i, j;
	for (i = left + 1; i <= right; i++) {
		Element temp = a[i];
		j = i;
		while (j > left && cmpfunc(a[j - 1], temp) > 0) {
			a[j] = a[j - 1];
			j--;
		}
		a[j] = temp;
	}
}
