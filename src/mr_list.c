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
static void __linkedlist_node_plugout(linkedlist_p ll, ll_node_p node);				// 从链表中抽离出一个节点
static void __linkedlist_removeall(linkedlist_p list);						// 清空链表
static void __arraylist_removeall(arraylist_p list, size_t size);				// 清空线性表
static size_t __linkedlist_remove_at(linkedlist_p ll, size_t size, size_t index);		// 删除链表节点
static size_t __arraylist_remove_at(arraylist_p al, size_t size, size_t index);			// 删除线性表元素
static size_t __linkedlist_remove(linkedlist_p ll, element_p ele, CmpFunc cmpfunc);		// 删除所有与ele相等的元素的链表节点
static size_t __arraylist_remove(arraylist_p al, size_t size, element_p ele, CmpFunc cmpfunc);	// 删除所有与ele相等的线性表元素

static int __arraylist_expand(arraylist_p list);						// 线性表扩容
static ll_node_p __linkedlist_node_create(element_p ele);					// 创建一个链表节点
static ll_node_p __linkedlist_goto(linkedlist_p ll, size_t size, size_t pos);			// 找到链表的第pos个节点
static int __linkedlist_ins(linkedlist_p ll, size_t size, size_t index, element_p ele);		// 在index位置插入一个新元素
static int __arraylist_ins(arraylist_p al, size_t size, size_t index, element_p ele);		// 在index位置插入一个新元素

static int __linkedlist_search(linkedlist_p ll, size_t from, int dir, size_t size, element_p ele, CmpFunc cmpfunc);		// 搜索链表
static int __arraylist_search(arraylist_p al, size_t from, int dir, size_t size, element_p ele, CmpFunc cmpfunc);		// 搜索线性表

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
			__arraylist_removeall((arraylist_p)l->list, l->size);
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
	return IS_VALID_LIST(list) ? ((list_p)list->container)->size == 0 : 1;
}

size_t list_size(Container list)
{
	return IS_VALID_LIST(list) ? ((list_p)list->container)->size : 0;
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
	Element ret = NULL;
	if (IS_VALID_LIST(list) && index < ((list_p)list->container)->size) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList)
			ret = __element_clone_value((__linkedlist_goto((linkedlist_p)l->list, l->size, index))->element);
		else
			ret = __element_clone_value(((arraylist_p)l->list)->elements[index]);
		pthread_mutex_unlock(&l->mut);
	}
	return ret;
}

size_t list_remove_at(Container list, size_t index)
{
	size_t count = 0;
	if (IS_VALID_LIST(list) && index < ((list_p)list->container)->size) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList)
			count = __linkedlist_remove_at((linkedlist_p)l->list, l->size, index);
		else
			count = __arraylist_remove_at((arraylist_p)l->list, l->size, index);
		if (count) {
			l->size -= count;
			l->changes++;
		}
		pthread_mutex_unlock(&l->mut);
	}
	return count;
}

size_t list_remove(Container list, Element element, ElementType type, size_t len)
{
	size_t count = 0;
	element_p ele = NULL;
	if (IS_VALID_LIST(list) && ((list_p)list->container)->etype == type && ((list_p)list->container)->size > 0 && (ele = __element_create(element, type, len))) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList)
			count = __linkedlist_remove((linkedlist_p)l->list, ele, l->cmpfunc);
		else
			count = __arraylist_remove((arraylist_p)l->list, l->size, ele, l->cmpfunc);
		if (count) {
			l->size -= count;
			l->changes++;
		}
		__element_destroy(ele);
		pthread_mutex_unlock(&l->mut);
	}
	return count;
}

void list_removeall(Container list)
{
	if (IS_VALID_LIST(list)) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList)
			__linkedlist_removeall((linkedlist_p)l->list);
		else
			__arraylist_removeall((arraylist_p)l->list, l->size);
		l->size = 0;
		l->changes++;
		pthread_mutex_unlock(&l->mut);
	}
}

int list_search(Container list, int from, int dir, Element element, ElementType type, size_t len)
{
	int ret = -1;
	element_p ele = NULL;
	if (IS_VALID_LIST(list) && ((list_p)list->container)->etype == type && ((list_p)list->container)->size > 0 && (ele = __element_create(element, type, len))) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		size_t start;
		if (from < 0)
			start = dir == Reverse ? l->size - 1 : 0;
		else if (from >= l->size)
			start = l->size - 1;
		else
			start = from;
		if (l->ltype == LinkedList)
			ret = __linkedlist_search((linkedlist_p)l->list, start, dir, l->size, ele, l->cmpfunc);
		else
			ret = __arraylist_search((arraylist_p)l->list, start, dir, l->size, ele, l->cmpfunc);
		__element_destroy(ele);
		pthread_mutex_unlock(&l->mut);
	}
	return ret;
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
	Element ret = NULL;
	if (IS_VALID_LIST(list) && ((list_p)list->container)->size > 0) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList) {
			linkedlist_p ll = (linkedlist_p)l->list;
			ll_node_p top = ll->tail;
			ret = __element_clone_value(top->element);
			__linkedlist_node_plugout(ll, top);
			__linkedlist_node_destroy(top);
		} else {
			arraylist_p al = (arraylist_p)l->list;
			ret = __element_clone_value(al->elements[l->size - 1]);
			__element_destroy(al->elements[l->size - 1]);
		}
		l->size--;
		l->changes++;
		pthread_mutex_unlock(&l->mut);
	}
	return ret;
}

Element list_stacktop(Container list)
{
	Element ret = NULL;
	if (IS_VALID_LIST(list) && ((list_p)list->container)->size > 0) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList)
			ret = __element_clone_value(((linkedlist_p)l->list)->tail->element);
		else
			ret = __element_clone_value(((arraylist_p)l->list)->elements[l->size - 1]);
		pthread_mutex_unlock(&l->mut);
	}
	return ret;
}

int list_enqueue(Container list, Element element, ElementType type, size_t len)
{
	return IS_VALID_LIST(list) ? list_insert(list, ((list_p)list->container)->size, element, type, len) : -1;
}

Element list_dequeue(Container list)
{
	Element ret = NULL;
	if (IS_VALID_LIST(list) && ((list_p)list->container)->size > 0) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList) {
			linkedlist_p ll = (linkedlist_p)l->list;
			ll_node_p head = ll->head;
			ret = __element_clone_value(head->element);
			__linkedlist_node_plugout(ll, head);
			__linkedlist_node_destroy(head);
		} else {
			arraylist_p al = (arraylist_p)l->list;
			ret = __element_clone_value(al->elements[0]);
			__element_destroy(al->elements[0]);
			for (size_t i = 1; i < l->size; i++)
				al->elements[i - 1] = al->elements[i];
		}
		l->size--;
		l->changes++;
		pthread_mutex_unlock(&l->mut);
	}
	return ret;
}

Element list_queuehead(Container list)
{
	Element ret = NULL;
	if (IS_VALID_LIST(list) && ((list_p)list->container)->size > 0) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList)
			ret = __element_clone_value(((linkedlist_p)l->list)->head->element);
		else
			ret = __element_clone_value(((arraylist_p)l->list)->elements[0]);
		pthread_mutex_unlock(&l->mut);
	}
	return ret;
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

static void __linkedlist_node_plugout(linkedlist_p ll, ll_node_p node)
{
	if (node->next)
		node->next->prev = node->prev;		// 非尾节点抽离，后继节点的前驱指针指向当前节点的前驱
	else
		ll->tail = node->prev;			// 尾节点抽离，尾指针指向当前节点的前驱
	if (node->prev)
		node->prev->next = node->next;		// 非头节点抽离，前驱节点的后继指针指向当前节点的后继
	else
		ll->head = node->next;			// 头节点抽离，头指针指向当前节点的后继
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
	for (size_t i = 0; i < size; i++)
		__element_destroy(list->elements[i]);
}

/**
 * @brief 删除链表中的一个节点
 *
 * @param list
 * 	链表
 * @param size
 * 	链表中元素的数量
 * @param index
 * 	删除节点的位置
 *
 * @return 
 * 	删除成功的节点数量
 */
static size_t __linkedlist_remove_at(linkedlist_p ll, size_t size, size_t index)
{
	ll_node_p node = __linkedlist_goto(ll, size, index);
	size_t ret = 0;
	if (node) {
		__linkedlist_node_plugout(ll, node);
		__linkedlist_node_destroy(node);
		ret = 1;
	}
	return ret;
}

/**
 * @brief 删除线性表中的一个元素
 *
 * @param list
 * 	线性表
 * @param size
 * 	线性表中元素的数量
 * @param index
 * 	删除元素的位置
 *
 * @return 
 * 	删除成功的元素数量
 */
static size_t __arraylist_remove_at(arraylist_p al, size_t size, size_t index)
{
	element_p ele = al->elements[index];
	for (size_t i = index; i < size - 1; i++)
		al->elements[i] = al->elements[i + 1];
	__element_destroy(ele);
	return 1;
}

/**
 * @brief 在链表中删除所有元素等于ele的节点
 *
 * @param ll
 * 	链表
 * @param ele
 * 	元素
 * @param cmpfunc
 * 	比较函数
 *
 * @return 
 * 	删除的节点数量
 */
static size_t __linkedlist_remove(linkedlist_p ll, element_p ele, CmpFunc cmpfunc)
{
	size_t count = 0;
	ll_node_p node = ll->head;
	while (node) {
		if (cmpfunc(node->element->value, ele->value, node->element->len, ele->len) == 0) {
			__linkedlist_node_plugout(ll, node);
			ll_node_p n = node;
			node = node->next;
			__linkedlist_node_destroy(n);
			count++;
		} else {
			node = node->next;
		}
	}
	return count;
}

/**
 * @brief 在线性表中删除所有等于ele的元素
 *
 * @param al
 * 	线性表
 * @param size
 * 	表中元素数量
 * @param ele
 * 	元素
 * @param cmpfunc
 * 	比较函数
 *
 * @return 
 * 	删除的元素数量
 */
static size_t __arraylist_remove(arraylist_p al, size_t size, element_p ele, CmpFunc cmpfunc)
{
	size_t count = 0;
	size_t pos = 0;
	while (pos < size) {
		if (cmpfunc(al->elements[pos]->value, ele->value, al->elements[pos]->len, ele->len) == 0) {
			__element_destroy(al->elements[pos]);
			count++;
		} else if (count) {
			al->elements[pos - count] = al->elements[pos];
		}
		pos++;
	}
	return count;
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
 * @param index
 * 	插入位置，超过表尾的添加在尾部
 * @param ele
 * 	新的元素
 *
 * @return 
 * 	插入成功返回0，失败返回-1
 */
static int __linkedlist_ins(linkedlist_p ll, size_t size, size_t index, element_p ele)
{
	ll_node_p node = __linkedlist_node_create(ele);
	if (node) {
		ll_node_p pos = __linkedlist_goto(ll, size, index);
		if (pos) {
			// 在位置pos处插入node
			node->next = pos;
			node->prev = pos->prev;
			pos->prev = node;
			if (node->prev)
				node->prev->next = node;
			else
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
static int __arraylist_ins(arraylist_p al, size_t size, size_t index, element_p ele)
{
	int ret = -1;
	if (size <  al->capacity || __arraylist_expand(al) == 0) {
		size_t i;
		for (size_t i = size; i > index; i--)
			al->elements[i] = al->elements[i - 1];
		al->elements[i] = ele;
		ret = 0;
	}
	return ret;
}

/**
 * @brief 链表搜索
 *
 * @param ll
 * 	链表
 * @param from
 * 	搜索开始位置
 * @param dir
 * 	搜索的方向
 * @param size
 * 	链表中元素数量
 * @param ele
 * 	搜索的元素
 * @param cmpfunc
 * 	元素比较函数
 *
 * @return 
 * 	搜索到的位置，搜索不到返回-1
 */
static int __linkedlist_search(linkedlist_p ll, size_t from, int dir, size_t size, element_p ele, CmpFunc cmpfunc)
{
	int pos = from;
	ll_node_p node = __linkedlist_goto(ll, size, from);
	while (node && cmpfunc(node->element->value, ele->value, node->element->len, ele->len))
		if (dir == Reverse) {
			node = node->prev;
			pos--;
		} else {
			node = node->next;
			pos++;
		}
	return node ? pos : -1;
}

/**
 * @brief 线性表搜索
 *
 * @param al
 * 	线性表
 * @param from
 * 	搜索开始位置
 * @param dir
 * 	搜索的方向
 * @param size
 * 	线性表中元素数量
 * @param ele
 * 	搜索的元素
 * @param cmpfunc
 * 	元素比较函数
 *
 * @return 
 * 	搜索到的位置，搜索不到返回-1
 */
static int __arraylist_search(arraylist_p al, size_t from, int dir, size_t size, element_p ele, CmpFunc cmpfunc)
{
	int pos = from;
	while (cmpfunc(al->elements[pos]->value, ele->value, al->elements[pos]->len, ele->len) && pos >= 0 && pos < size)
		if (dir == Reverse)
			pos--;
		else
			pos++;
	return IN(pos, 0, size - 1) ? pos : -1;
}

