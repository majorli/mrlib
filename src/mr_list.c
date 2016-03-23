#include <stdlib.h>
#include <pthread.h>

#include "mr_list.h"
#include "private_element.h"

#define IS_VALID_LIST(X) (X && X->container && X->type == List)

#define AL_INIT_CAPA 10
#define AL_NEXT_CAPA(CC) ((CC) * 3 / 2 + 1)

/**
 * 链表节点结构
 */
typedef struct LinkedListNode {
	element_p element;
	struct LinkedListNode *next;
	struct LinkedListNode *prev;
} ll_node_t, *ll_node_p;

/**
 * 链表结构
 */
typedef struct {
	ll_node_p head;
	ll_node_p tail;
} linkedlist_t, *linkedlist_p;

/**
 * 线性表结构
 */
typedef struct {
	size_t capacity;
	element_p *elements;
} arraylist_t, *arraylist_p;

/**
 * 列表容器结构
 */
typedef struct {
	void *list;
	ListType ltype;
	ElementType etype;
	CmpFunc cmpfunc;
	size_t size;
	pthread_mutex_t mut;
	unsigned int changes;
} list_t, *list_p;

static void __linkedlist_node_destroy(ll_node_p node);						// 销毁链表节点
static void __linkedlist_removeall(linkedlist_p list);						// 清空链表
static void __arraylist_removeall(arraylist_p list, size_t size);				// 清空线性表

static int __arraylist_expand(arraylist_p list);						// 线性表扩容
static ll_node_p __linkedlist_node_create(element_p ele);					// 创建一个链表节点
static ll_node_p __linkedlist_goto(linkedlist_p ll, size_t size, size_t pos);			// 找到链表的第pos个节点
static int __linkedlist_ins(linkedlist_p ll, size_t size, size_t pos, element_p ele);		// 在pos位置插入一个新元素
static int __arraylist_ins(arraylist_p al, size_t size, size_t pos, element_p ele);		// 在pos位置插入一个新元素

Container list_create(ElementType etype, ListType ltype, CmpFunc cmpfunc)
{
	Container cont = NULL;
	list_p list = NULL;
	void *l = NULL;
	element_p *elements = NULL;
	cont = (Container)malloc(sizeof(Container_t));
	if (!cont)
		return NULL;
	list = (list_p)malloc(sizeof(list_t));
	if (!list) {
		free(cont);
		return NULL;
	} else {
		cont->container = list;
		cont->type = List;
	}
	l = ltype == LinkedList ? malloc(sizeof(linkedlist_t)) : malloc(sizeof(arraylist_t));
	if (!l) {
		free(list);
		free(cont);
		return NULL;
	} else {
		if (ltype == LinkedList) {
			((linkedlist_p)l)->head = NULL;
			((linkedlist_p)l)->tail = NULL;
		} else {
			elements = (element_p *)malloc(AL_INIT_CAPA * sizeof(element_p));
			if (!elements) {
				free(l);
				free(list);
				free(cont);
				return NULL;
			} else {
				((arraylist_p)l)->capacity = AL_INIT_CAPA;
				((arraylist_p)l)->elements = elements;
			}
		}
		list->list = l;
		list->ltype = ltype;
		list->etype = etype;
		list->cmpfunc = cmpfunc ? cmpfunc : __default_cmpfunc(etype);
		list->size = 0;
		list->changes = 0;
		pthread_mutex_init(&list->mut, NULL);
	}
	return cont;
}

int list_destroy(Container list)
{
	int ret = -1;
	if (IS_VALID_LIST(list)) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList) {
			__linkedlist_removeall((linkedlist_p)l->list);
		} else {
			__arraylist_removeall((arraylist_p)l->list);
			free(((arraylist_p)l)->elements);
		}
		free(l->list);
		l->size = 0;
		l->changes++;
		pthread_mutex_unlock(&l->mut);
		pthread_mutex_destroy(&l->mut);
		free(list);
		ret = 0;
	}
	return ret;
}

int list_isempty(Container list)
{
	return IS_VALIE_LIST(list) ? ((list_p)list->container)->size == 0 : 1;
}

size_t list_size(Container list)
{
	return IS_VALIE_LIST(list) ? ((list_p)list->container)->size : 0;
}

int list_append(Container list, Element element, ElementType type, size_t len)
{
	return IS_VALID_LIST(list) ? list_insert(list, ((list_p)list->container)->size, element, type, len) : -1;
}

int list_insert(Container list, size_t index, Element element, ElementType type, size_t len)
{
	int ret = -1;
	element_p ele = NULL;
	list_p l = NULL;
	if (IS_VALID_LIST(list) && (l = (list_p)list->container)->etype == type && (ele = __element_create(element, type, len))) {
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList)
			ret = __linkedlist_ins((linkedlist_p)l->list, l->size, index, ele);
		else
			ret = __arraylist_ins((arraylist_p)l->list, l->size, index, ele);
		if (ret == 0) {
			l->size++;
			l->changes++;
		} else {
			__element_destroy(ele);
		}
		pthread_mutex_unlock(&l->mut);
	}
	return ret;
}

Element list_get(Container list, size_t index)
{
	return NULL;
}

size_t list_remove_at(Container list, size_t index)
{
	return 0;
}

size_t list_remove(Container list, Element element, ElementType type, size_t len)
{
	return 0;
}

void list_removeall(Container list)
{
	return;
}

int list_search(Container list, Element element, ElementType type, size_t len)
{
	return -1;
}

int list_bi_search(Container list, Element element, ElementType type, size_t len)
{
	return -1;
}

void list_qsort(Container list, CmpFunc cmpfunc)
{
	return;
}

void list_isort(Container list, CmpFunc cmpfunc)
{
	return;
}

void list_reverse(Container list)
{
	return;
}

void list_plus(Container l1, Container l2)
{
	return;
}

void list_minus(Container l1, Container l2)
{
	return;
}

void list_retain(Container l1, Container l2)
{
	return;
}

int list_push(Container list, Element element, ElementType type, size_t len)
{
	return IS_VALID_LIST(list) ? list_insert(list, ((list_p)list->container)->size, element, type, len) : -1;
}

Element list_pop(Container list)
{
	return NULL;
}

Element list_peak(Container list)
{
	return NULL;
}

int list_enqueue(Container list, Element element, ElementType type, size_t len)
{
	return IS_VALID_LIST(list) ? list_insert(list, ((list_p)list->container)->size, element, type, len) : -1;
}

Element list_dequeue(Container list)
{
	return NULL;
}

Element list_queuehead(Container list)
{
	return NULL;
}

Iterator list_iterator(Container list, int dir)
{
	return NULL;
}

/**
 * @brief 销毁一个链表节点，包括其中的元素
 *
 * @param node
 * 	待销毁的节点
 */
static void __linkedlist_node_destroy(ll_node_p node)
{
	if (!node)
		return;
	__element_destroy(node->element);
	free(node);
}

/**
 * @brief 清空链表，销毁其中所有节点，回到初始状态
 *
 * @param list
 * 	待清空的链表
 */
static void __linkedlist_removeall(linkedlist_p list)
{
	ll_node_p node = list->head;
	while (node) {
		ll_node_p current = node;
		node = node->next;
		__linkedlist_node_destroy(current);
	}
	list->head = NULL;
	list->tail = NULL;
}

/**
 * @brief 清空线性表，销毁其中所有元素，回到初始状态
 *
 * @param list
 * 	待清空的线性表
 * @param size
 * 	表中元素的数量
 */
static void __arraylist_removeall(arraylist_p list, size_t size)
{
	size_t i;
	for (i = 0; i < size; i++) {
		__element_destroy(list->elements[i]);
		list->elements[i] = NULL;
	}
}

/**
 * @brief 线性表底层数组扩容，宏函数AL_NEXT_CAPA用于计算扩容后的容量
 *
 * @param list
 * 	现行表
 *
 * @return 
 * 	扩容成功返回0，失败返回-1
 */
static int __arraylist_expand(arraylist_p list)
{
	int ret = -1;
	size_t oc = list->capacity;
	size_t nc = AL_NEXT_CAPA(oc);
	element_p *nl = (element_p *)realloc(list->elements, nc * sizeof(element_p));
	if (nl) {
		list->elements = nl;
		list->capacity = nc;
		ret = 0;
	}
	return ret;
}

/**
 * @brief 创建一个链表节点
 *
 * @param ele
 * 	节点中的元素
 *
 * @return 
 * 	创建的新节点，创建失败返回NULL
 */
static ll_node_p __linkedlist_node_create(element_p ele)
{
	ll_node_p node = (ll_node_p)malloc(sizeof(ll_node_t));
	if (node) {
		node->element = ele;
		node->next = NULL;
		node->prev = NULL;
	}
	return node;
}

/**
 * @brief 找到链表中的第pos个节点
 *
 * @param ll
 * 	链表
 * @param size
 * 	链表中元素数量
 * @param pos
 * 	寻找的位置
 *
 * @return 
 * 	第pos个节点，pos超出范围返回NULL
 */
static ll_node_p __linkedlist_goto(linkedlist_p ll, size_t size, size_t pos)
{
	if (size == 0)
		return ll->tail;	// ll->tail === NULL

	if (pos == 0)
		return ll->head;
	else if (pos == size -1)
		return ll->tail;
	else if (pos >= size)
		return NULL;
	else if (pos < size / 2) {
		ll_node_p node = ll->head;
		for (size_t i = 0; i < pos; i++)
			node = node->next;
		return node;
	} else {			// size / 2 <= pos < size -1
		ll_node_p node = ll->tail;
		for (size_t i = size - 1; i > pos; i--)
			node = node->prev;
		return node;
	}
}

/**
 * @brief 在链表的第pos个位置插入新元素ele
 *
 * @param ll
 * 	链表
 * @para size
 * 	插入前链表中元素数量
 * @param pos
 * 	插入位置，超过表尾的添加在尾部
 * @param ele
 * 	新的元素
 *
 * @return 
 * 	插入成功返回0，失败返回-1
 */
static int __linkedlist_ins(linkedlist_p ll, size_t size, size_t pos, element_p ele)
{
	ll_node_p node = __linkedlist_node_create(ele);
	if (node) {
		ll_node_p pos = __linkedlist_goto(ll, size, pos);
		if (pos) {
			// 在位置pos处插入node
			node->next = pos;
			node->prev = pos->prev;
			pos->prev = node;
			if (node->prev)
				node->prev->next = node;
			if (ll->head == pos)
				ll->head = node;
		} else {
			// 位置pos处无元素，表示当前链表为空或pos在链尾后方，此时在尾部添加
			if (ll->tail) {
				// 链表不空，此时tail和head不为NULL，将node链接到tail后方即可
				node->prev = ll->tail;
				ll->tail->next = node;
				ll->tail = node;
			} else {
				// 链表为空，此时tail和head为NULL，将tail和head同时指向node即可
				ll->tail = node;
				ll->head = node;
			}
		}
	}
	return node ? 0 : -1;
}

/**
 * @brief 在线性表的第pos个位置插入新元素ele
 *
 * @param ll
 * 	线性表
 * @para size
 * 	插入前线性表中元素数量
 * @param pos
 * 	插入位置，超过表尾的添加在尾部
 * @param ele
 * 	新的元素
 *
 * @return 
 * 	插入成功返回0，失败返回-1
 */
static int __arraylist_ins(arraylist_p al, size_t size, size_t pos, element_p ele)
{
	int ret = -1;
	if (size <  capacity || __arraylist_expand(al) == 0) {
		size_t i;
		for (size_t i = size; i > pos; i--)
			al->elements[i] = al->elements[i - 1];
		al->elements[i] = ele;
		ret = 0;
	}
	return ret;
}
