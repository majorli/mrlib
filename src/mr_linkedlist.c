#include <stdlib.h>
#include <pthread.h>

#include "mr_containers.h"
#include "mr_linkedlist.h"

LinkedList ll_create(ElementType type, CmpFunc cmpfunc);
int ll_destroy(LinkedList ll);
int ll_isempty(LinkedList ll);
size_t ll_size(LinkedList ll);
LLNode ll_head(LinkedList ll);
LLNode ll_tail(LinkedList ll);
LLNode ll_next(LLNode pos);
LLNode ll_prev(LLNode pos);
Element ll_get(LLNode pos);
LLNode ll_insert_after(LLNode pos, Element ele);
LLNode ll_insert_before(LLNode pos, Element ele);
LLNode ll_append(LinkedList ll, Element ele);
LLNode ll_prepend(LinkedList ll, Element ele);
Element ll_remove(LLNode *pos);
void ll_removeall(LinkedList ll, onRemove onremove);
Element ll_replace(LLNode pos, Element ele);
LLNode ll_search(LinkedList ll, Element ele);
LLNode ll_rsearch(LinkedList ll, Element ele);
void ll_sort(LinkedList ll);
void ll_stsort(LinkedList ll);
void ll_comparator(LinkedList ll, CmpFunc cmpfunc);

/**
 * 链表节点结构
 */
typedef struct ll_node {
	Element element;		// 元素
	struct ll_node *prev;		// 前链接指针
	struct ll_node *next;		// 后链接指针
	LinkedList list;		// 所属链表的句柄，用于验证节点的归属
} ll_node_t, *ll_node_p;

/**
 * 链表结构
 */
typedef struct {
	ElementType type;		// 元素的数据类型
	ll_node_p head;			// 头指针
	ll_node_p tail;			// 尾指针
	CmpFunc cmpfunc;		// 元素比较函数
	pthread_mutex_t mut;		// 共享锁
} ll_t, *ll_p;

static void __ll_init(ll_p list, ll_node_p node);					// 空表第一次插入节点
static void __ll_insert_after(ll_p list, ll_node_p pos, ll_node_p node);		// 在指定位置之后插入节点
static void __ll_insert_before(ll_p list, ll_node_p pos, ll_node_p node);		// 在指定位置之前插入节点
static void __ll_remove(ll_p list, ll_node_p pos);					// 删除指定位置的节点
static void __ll_quicksort(ll_node_p left, ll_node_p right, CmpFunc cmpfunc);		// 快速排序
static void __ll_insertionsort(ll_node_p left, ll_node_p right, CmpFunc cmpfunc);	// 插入排序

/**
 * 创建一个LinkedList，返回句柄
 * type:	元素的类型
 * cmpfunc:	元素比较函数，传入NULL表示采用mr_common.h中定义的与type对应的默认比较函数
 *
 * 返回:	新创建的LinkedList的句柄(一个大于等于0的正整数)，创建失败返回-1
 */
LinkedList ll_create(ElementType type, CmpFunc cmpfunc)
{
	LinkedList ret = -1;
	ll_p list = (ll_p)malloc(sizeof(ll_t));
	list->type = type;
	list->head = NULL;
	list->tail = NULL;
	if (cmpfunc == NULL)
		list->cmpfunc = default_cmpfunc(type);
	else
		list->cmpfunc = cmpfunc;
	ret = container_retrieve(list, LinkedList_t);
	if (ret == -1) {
		free(list);
	} else {
		if (__MultiThreads__ == 1) {
			pthread_mutex_init(&(list->mut), NULL);
		}
	}
	return ret;
}

/**
 * 销毁一个LinkedList，释放列表的空间，但不会销毁其中的元素
 * ll:		LinkedList句柄
 *
 * 返回:	销毁完成返回0，销毁失败或无效LinkedList句柄返回-1
 */
int ll_destroy(LinkedList ll)
{
	int ret = -1;
	ll_p list = (ll_p)container_release(ll, LinkedList_t);
	if (list != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ll_node_p p = list->head;
		while (p != NULL) {
			ll_node_p n = p->next;
			free(p);
			p = n;
		}
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
 * 判断一个LinkedList是否为空
 * ll:		LinkedList句柄
 *
 * 返回:	为空返回0，不为空返回1，无效句柄返回-1
 */
int ll_isempty(LinkedList ll)
{
	int ret = -1;
	ll_p list = (ll_p)container_get(ll, LinkedList_t);
	if (list != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = (list->head == NULL);
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return ret;
}

/**
 * 获取一个LinkedList中的元素数量
 * ll:		LinkedList句柄
 *
 * 返回:	元素数量，空列表或无效句柄返回0
 */
size_t ll_size(LinkedList ll)
{
	size_t ret = 0;
	ll_p list = (ll_p)container_get(ll, LinkedList_t);
	if (list != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ll_node_p p = list->head;
		while (p != NULL) {
			ret++;
			p = p->next;
		}
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return ret;
}

/**
 * 获取表头元素的位置
 * ll:		LinkedList句柄
 *
 * 返回:	表头的元素位置，列表句柄无效或空表时返回NULL
 */
LLNode ll_head(LinkedList ll)
{
	LLNode ret = NULL;
	ll_p list = (ll_p)container_get(ll, LinkedList_t);
	if (list != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = list->head;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return ret;
}

/**
 * 获取表尾元素的位置
 * ll:		LinkedList句柄
 *
 * 返回:	表尾的元素位置，列表句柄无效或空表时返回NULL
 */
LLNode ll_tail(LinkedList ll)
{
	LLNode ret = NULL;
	ll_p list = (ll_p)container_get(ll, LinkedList_t);
	if (list != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = list->tail;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return ret;
}

/**
 * 获取pos位置的下一个元素的位置
 * pos:		当前位置
 *
 * 返回:	下一个元素的位置，pos==NULL或已经到达表尾时返回NULL
 */
LLNode ll_next(LLNode pos)
{
	ll_node_p ret = NULL;
	ll_node_p p = (ll_node_p)pos;
	ll_p list;
	if (p != NULL && (list = (ll_p)container_get(p->list, LinkedList_t)) != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = p->next;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return (LLNode)ret;
}

/**
 * 获取pos位置的前一个元素的位置
 * pos:		当前位置
 *
 * 返回:	前一个元素的位置，pos==NULL或已经到达表头时返回NULL
 */
LLNode ll_prev(LLNode pos)
{
	ll_node_p ret = NULL;
	ll_node_p p = (ll_node_p)pos;
	ll_p list;
	if (p != NULL && (list = (ll_p)container_get(p->list, LinkedList_t)) != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = p->prev;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return (LLNode)ret;
}

/**
 * 获取指定位置处的元素
 * pos:		指定的元素位置
 *
 * 返回:	元素，如果位置无效或列表句柄无效则返回NULL
 */
Element ll_get(LLNode pos)
{
	Element ret = NULL;
	ll_node_p p = (ll_node_p)pos;
	ll_p list;
	if (p != NULL && (list = (ll_p)container_get(p->list, LinkedList_t)) != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = p->element;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return ret;
}

/**
 * 在列表指定位置之后插入一个元素
 * pos:		要插入元素的位置，不能为NULL
 * ele:		元素，不能为NULL
 *
 * 返回:	元素插入后所在位置，如果元素或位置为NULL或者ll句柄无效，或发生其他错误导致添加失败返回NULL
 */
LLNode ll_insert_after(LLNode pos, Element ele)
{
	ll_node_p ret = NULL;
	ll_node_p p = (ll_node_p)pos;
	ll_p list;
	if (p != NULL && ele != NULL && (list = (ll_p)container_get(p->list, LinkedList_t)) != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = (ll_node_p)malloc(sizeof(ll_node_t));
		ret->element = ele;
		ret->list = p->list;
		__ll_insert_after(list, p, ret);
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return (LLNode)ret;
}

/**
 * 在列表指定位置之前插入一个元素
 * pos:		要插入元素的位置，不能为NULL
 * ele:		元素，不能为NULL
 *
 * 返回:	元素插入后所在位置，如果元素或位置为NULL或者ll句柄无效，或发生其他错误导致添加失败返回NULL
 */
LLNode ll_insert_before(LLNode pos, Element ele)
{
	ll_node_p ret = NULL;
	ll_node_p p = (ll_node_p)pos;
	ll_p list;
	if (p != NULL && ele != NULL && (list = (ll_p)container_get(p->list, LinkedList_t)) != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = (ll_node_p)malloc(sizeof(ll_node_t));
		ret->element = ele;
		ret->list = p->list;
		__ll_insert_before(list, p, ret);
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return (LLNode)ret;
}

/**
 * 在列表最后添加一个元素
 * ll:		LinkedList句柄
 * ele:		元素，不能为NULL
 *
 * 返回:	元素添加后所在位置，如果元素为NULL或者ll句柄无效，或发生其他错误导致添加失败返回NULL
 */
LLNode ll_append(LinkedList ll, Element ele)
{
	ll_node_p ret = NULL;
	ll_p list = (ll_p)container_get(ll, LinkedList_t);
	if (ele != NULL && list != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = (ll_node_p)malloc(sizeof(ll_node_t));
		ret->element = ele;
		ret->list = ll;
		if (list->head == NULL)				// 空表，添加首个节点
			__ll_init(list, ret);
		else						// 非空表，在表尾之后添加节点
			__ll_insert_after(list, list->tail, ret);
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return (LLNode)ret;
}

/**
 * 在列表最前添加一个元素
 * ll:		LinkedList句柄
 * ele:		元素，不能为NULL
 *
 * 返回:	元素添加后所在位置，如果元素为NULL或者ll句柄无效，或发生其他错误导致添加失败返回NULL
 */
LLNode ll_prepend(LinkedList ll, Element ele)
{
	ll_node_p ret = NULL;
	ll_p list = (ll_p)container_get(ll, LinkedList_t);
	if (ele != NULL && list != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = (ll_node_p)malloc(sizeof(ll_node_t));
		ret->element = ele;
		ret->list = ll;
		if (list->head == NULL)				// 空表，添加首个节点
			__ll_init(list, ret);
		else						// 非空表，在表头之前添加节点
			__ll_insert_before(list, list->head, ret);
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return (LLNode)ret;
}

/**
 * 删除指定位置的元素，删除成功后pos所指的节点被释放，节点失效
 * pos:		要删除的节点，删除后*pos被置为NULL
 *
 * 返回:	删除的元素，ll无效或*pos无效时返回NULL
 */
Element ll_remove(LLNode *pos)
{
	Element ret = NULL;
	ll_node_p p = (ll_node_p)(*pos);
	ll_p list;
	if (p != NULL && (list = (ll_p)container_get(p->list, LinkedList_t)) != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = p->element;
		__ll_remove(list, p);
		*pos = NULL;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return ret;
}

/**
 * 删除LinkedList中所有的元素，被清除的元素用onremove函数进行后续处理
 * ll:		LinkedList句柄
 * onremove:	元素后续处理函数，NULL表示不做任何处理，典型的可以传入标准库函数free
 *
 */
void ll_removeall(LinkedList ll, onRemove onremove)
{
	ll_p list = (ll_p)container_get(ll, LinkedList_t);
	if (list != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ll_node_p p = list->head;
		while (p != NULL) {
			ll_node_p n = p->next;
			if (onremove != NULL)
				onremove(p->element);
			free(p);
			p = n;
		}
		list->head = NULL;
		list->tail = NULL;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
}

/**
 * 在指定位置存储一个元素，覆盖原有的元素，原元素将被返回，其所占的内存空间不会被释放
 * pos:		要存储元素的节点
 * ele:		元素，不能为NULL
 *
 * 返回:	原元素，如果元素为NULL或者节点无效时返回NULL
 */
Element ll_replace(LLNode pos, Element ele)
{
	Element ret = NULL;
	ll_node_p p = (ll_node_p)pos;
	ll_p list;
	if (p != NULL && ele != NULL && (list = (ll_p)container_get(p->list, LinkedList_t)) != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = p->element;
		p->element = ele;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return ret;
}

/**
 * 从列表中查找一个元素，元素的查找使用列表创建时提供的对象比较函数
 * ll:		LinkedList句柄
 * ele:		要查找的元素
 *
 * 返回:	查找到的时候返回节点，有多个相同元素时返回最前面的那个，ll句柄无效或ele为NULL或查找不到返回-1
 */
LLNode ll_search(LinkedList ll, Element ele)
{
	ll_node_p ret = NULL;
	ll_p list = (ll_p)container_get(ll, LinkedList_t);
	if (list != NULL && ele != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = list->head;
		while (ret != NULL) {
			if (list->cmpfunc(ret->element, ele) == 0)
				break;
			ret = ret->next;
		}
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return (LLNode)ret;
}

/**
 * 从列表中反向一个元素，元素的查找使用列表创建时提供的对象比较函数
 * ll:		LinkedList句柄
 * ele:		要查找的元素
 *
 * 返回:	查找到的时候返回节点，有多个相同元素时返回最后面的那个，ll句柄无效或ele为NULL或查找不到返回-1
 */
LLNode ll_rsearch(LinkedList ll, Element ele)
{
	ll_node_p ret = NULL;
	ll_p list = (ll_p)container_get(ll, LinkedList_t);
	if (list != NULL && ele != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(list->mut));
		ret = list->tail;
		while (ret != NULL) {
			if (list->cmpfunc(ret->element, ele) == 0)
				break;
			ret = ret->prev;
		}
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(list->mut));
	}
	return (LLNode)ret;
}

/**
 * 列表元素排序，使用列表创建时提供的对象比较函数进行比较，采用快速排序算法
 * ll:		LinkedList句柄
 *
 */
void ll_sort(LinkedList ll)
{
	ll_p list = (ll_p)container_get(ll, LinkedList_t);
	if (list != NULL && list->head != NULL && list->head != list->tail) {
		if (__MultiThreads__ == 1) {
			pthread_mutex_lock(&(list->mut));
		}
		__ll_quicksort(list->head, list->tail, list->cmpfunc);
		if (__MultiThreads__ == 1) {
			pthread_mutex_unlock(&(list->mut));
		}
	}
}

/**
 * 列表元素排序，使用列表创建时提供的对象比较函数进行比较，采用稳定的插入排序算法，适用于需要稳定排序或数据量较小的场合(建议数据量在5000以内)
 * ll:		LinkedList句柄
 *
 */
void ll_stsort(LinkedList ll)
{
	ll_p list = (ll_p)container_get(ll, LinkedList_t);
	if (list != NULL && list->head != NULL && list->head != list->tail) {
		if (__MultiThreads__ == 1) {
			pthread_mutex_lock(&(list->mut));
		}
		__ll_insertionsort(list->head, list->tail, list->cmpfunc);
		if (__MultiThreads__ == 1) {
			pthread_mutex_unlock(&(list->mut));
		}
	}
}

/**
 * 为列表设置或清除元素的数据比较函数
 * ll:		LinkedList句柄
 * cmpfunc:	比较函数，传入NULL表示清除原比较函数改为采用mr_common.h中定义的objcmp()函数
 *
 */
void ll_comparator(LinkedList ll, CmpFunc cmpfunc)
{
	ll_p list = (ll_p)container_get(ll, LinkedList_t);
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

static void __ll_init(ll_p list, ll_node_p node)				// 空表第一次插入节点
{
	node->next = NULL;
	node->prev = NULL;
	list->head = node;
	list->tail = node;
}

static void __ll_insert_after(ll_p list, ll_node_p pos, ll_node_p node)		// 在指定节点之后插入一个新节点
{
	node->next = pos->next;
	node->prev = pos;
	if (pos->next != NULL)
		pos->next->prev = node;
	else
		list->tail = node;
	pos->next = node;
}

static void __ll_insert_before(ll_p list, ll_node_p pos, ll_node_p node)	// 在指定节点之前插入一个新节点
{
	node->next = pos;
	node->prev = pos->prev;
	if (pos->prev != NULL)
		pos->prev->next = node;
	else
		list->head = node;
	pos->prev = node;
}

static void __ll_remove(ll_p list, ll_node_p pos)				// 删除指定位置的节点
{
	if (pos->prev != NULL)
		pos->prev->next = pos->next;
	else
		list->head = pos->next;
	if (pos->next != NULL)
		pos->next->prev = pos->prev;
	else
		list->tail = pos->prev;
	free(pos);
}

/**
 * 对一组元素进行快速排序
 * left:	左边界节点
 * right:	右边界节点
 * cmpfunc:	比较函数
 *
 */
static void __ll_quicksort(ll_node_p left, ll_node_p right, CmpFunc cmpfunc)
{
	if(left == right)
		return;
	ll_node_p i = left;
	ll_node_p j = right;
	Element key = left->element;
	while (i != j) {
		while (i != j && cmpfunc(key, j->element) <= 0)
			j = j->prev;
		if (i != j) {
			i->element = j->element;
			i = i->next;
		}
		while (i != j && cmpfunc(key, i->element) >= 0)
			i = i->next;
		if (i != j) {
			j->element = i->element;
			j = j->prev;
		}
	}
	i->element = key;
	if (left != i)
		__ll_quicksort(left, i->prev, cmpfunc);
	if (right != i)
		__ll_quicksort(i->next, right, cmpfunc);
}

/**
 * 对一组元素进行插入排序
 * left:	左边界节点
 * right:	右边界节点
 * cmpfunc:	比较函数
 *
 */
static void __ll_insertionsort(ll_node_p left, ll_node_p right, CmpFunc cmpfunc)
{
	ll_node_p i, j;
	i = left->next;
	while (i != right->next) {
		Element temp = i->element;
		j = i;
		while (j != left && cmpfunc(j->prev->element, temp) > 0) {
			j->element = j->prev->element;
			j = j->prev;
		}
		j->element = temp;
		i = i->next;
	}
}
