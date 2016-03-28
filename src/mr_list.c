#include <stdlib.h>
#include <pthread.h>

#include "mr_list.h"
#include "private_element.h"

#define IS_VALID_LIST(X) (X && X->container && X->type == List)

#define AL_INIT_CAPA 10
#define AL_NEXT_CAPA(CC) ((CC) * 3 / 2 + 1)

/**
 * @brief 链表节点结构
 */
typedef struct LinkedListNode {
	element_p element;
	struct LinkedListNode *next;
	struct LinkedListNode *prev;
} ll_node_t, *ll_node_p;

/**
 * @brief 链表结构
 */
typedef struct {
	ll_node_p head;
	ll_node_p tail;
} linkedlist_t, *linkedlist_p;

/**
 * @brief 线性表结构
 */
typedef struct {
	size_t capacity;
	element_p *elements;
} arraylist_t, *arraylist_p;

/**
 * @brief 列表容器结构
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

/**
 * @brief 列表迭代器当前位置联合
 */
typedef union {
	int index;
	ll_node_p node;
} list_pos_t, *list_pos_p;

/**
 * @brief 列表迭代器
 */
typedef struct {
	list_p list;
	list_pos_t pos;
	int dir;
	int changes;
	int removable;
} list_it_t, *list_it_p;

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
static int __linkedlist_bisearch(element_p e, size_t size, int order, ll_node_p left, ll_node_p right, CmpFunc cmpfunc);	// 链表二分搜索
static int __arraylist_bisearch(element_p e, element_p *a, int order, int left, int right, CmpFunc cmpfunc);			// 线性表二分搜索
static void __linkedlist_quicksort(ll_node_p left, ll_node_p right, int order, CmpFunc cmpfunc);				// 链表快速排序
static void __arraylist_quicksort(element_p *a, size_t left, size_t right, int order, CmpFunc cmpfunc);				// 线性表快速排序
static void __linkedlist_insertsort(ll_node_p left, ll_node_p right, int order, CmpFunc cmpfunc);				// 链表插入排序
static void __arraylist_insertsort(element_p *a, size_t left, size_t right, int order, CmpFunc cmpfunc);			// 线性表插入排序

static void __linkedlist_reverse(ll_node_p head, ll_node_p tail);				// 链表元素反转排列
static void __arraylist_reverse(element_p *a, size_t size);					// 线性表元素反转排列

static list_it_p __list_iterator(list_p list, int dir);						// 创建一个列表迭代器
static Element __list_it_next(void *it);							// 迭代访问下一个元素
static size_t __list_it_remove(void *it);							// 删除上一次迭代访问的元素
static void __list_it_reset(void *it);								// 重置迭代器
static void __list_it_destroy(void *it);							// 销毁迭代器

static element_p __list_get_at(list_p list, list_pos_t pos);					// 获取当前位置的元素
static void __list_del_at(list_p list, list_pos_t pos);					// 删除当前位置的元素
static void __list_append(list_p list, element_p ele);						// 在最后添加元素

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
			free(((arraylist_p)l->list)->elements);
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

void list_set_cmpfunc(Container list, CmpFunc cmpfunc)
{
	if (IS_VALID_LIST(list)) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		l->cmpfunc = cmpfunc ? cmpfunc : __default_cmpfunc(l->etype);
		pthread_mutex_unlock(&l->mut);
	}
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
	int ret = -1;
	element_p e = NULL;
	if (IS_VALID_LIST(list) && ((list_p)list->container)->etype == type && ((list_p)list->container)->size > 0 && (e = __element_create(element, type, len))) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		int order;
		if (l->ltype == LinkedList) {
			linkedlist_p ll = (linkedlist_p)l->list;
			order = l->cmpfunc(ll->head->element->value, ll->tail->element->value, ll->head->element->len, ll->tail->element->len) > 0 ? Desc : Asc;
			ret = __linkedlist_bisearch(e, l->size, order, ll->head, ll->tail, l->cmpfunc);
		} else {
			arraylist_p al = (arraylist_p)l->list;
			order = l->cmpfunc(al->elements[0]->value, al->elements[l->size - 1]->value, al->elements[0]->len, al->elements[l->size -1]->len) > 0 ? Desc : Asc;
			ret = __arraylist_bisearch(e, al->elements, order, 0, l->size - 1, l->cmpfunc);
		}
		__element_destroy(e);
		pthread_mutex_unlock(&l->mut);
	}
	return ret;
}

void list_qsort(Container list, int order)
{
	if (IS_VALID_LIST(list) && ((list_p)list->container)->size > 1) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList)
			__linkedlist_quicksort(((linkedlist_p)l->list)->head, ((linkedlist_p)l->list)->tail, order, l->cmpfunc);
		else
			__arraylist_quicksort(((arraylist_p)l->list)->elements, 0, l->size - 1, order, l->cmpfunc);
		l->changes++;
		pthread_mutex_unlock(&l->mut);
	}
}

void list_isort(Container list, int order)
{
	if (IS_VALID_LIST(list) && ((list_p)list->container)->size > 1) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList)
			__linkedlist_insertsort(((linkedlist_p)l->list)->head, ((linkedlist_p)l->list)->tail, order, l->cmpfunc);
		else
			__arraylist_insertsort(((arraylist_p)l->list)->elements, 0, l->size - 1, order, l->cmpfunc);
		l->changes++;
		pthread_mutex_unlock(&l->mut);
	}
}

void list_reverse(Container list)
{
	if (IS_VALID_LIST(list) && ((list_p)list->container)->size > 1) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList)
			__linkedlist_reverse(((linkedlist_p)l->list)->head, ((linkedlist_p)l->list)->tail);
		else
			__arraylist_reverse(((arraylist_p)l->list)->elements, l->size);
		l->changes++;
		pthread_mutex_unlock(&l->mut);
	}
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
	list_it_p it = NULL;
	if (IS_VALID_LIST(list)) {
		list_p l = (list_p)list->container;
		pthread_mutex_lock(&l->mut);
		it = __list_iterator(l, dir);
		pthread_mutex_unlock(&l->mut);
	}
	return it ? it_create(it, __list_it_next, __list_it_remove, __list_it_reset, __list_it_destroy) : NULL;
}

void list_plus(Container list1, Container list2)
{
	list_p l1, l2;
	if (IS_VALID_LIST(list1) && IS_VALID_LIST(list2) &&
			(l1 = (list_p)list1->container)->etype == (l2 = (list_p)list2->container)->etype &&
			l2->size > 0) {
		pthread_mutex_lock(&l1->mut);
		pthread_mutex_lock(&l2->mut);
		list_pos_t pos;
		if (l2->ltype == LinkedList)
			pos.node = ((linkedlist_p)l2->list)->head;
		else
			pos.index = 0;
		element_p ele;
		while ((ele = __list_get_at(l2, pos))) {
			__list_append(l1, ele);
			if (l2->ltype == LinkedList)
				pos.node = pos.node->next;
			else
				pos.index++;
		}
		pthread_mutex_unlock(&l1->mut);
		pthread_mutex_unlock(&l2->mut);
	}
}

void list_minus(Container list1, Container list2)
{
	list_p l1, l2;
	if (IS_VALID_LIST(list1) && IS_VALID_LIST(list2) &&
			(l1 = (list_p)list1->container)->etype == (l2 = (list_p)list2->container)->etype &&
			l2->size > 0) {
		pthread_mutex_lock(&l1->mut);
		pthread_mutex_lock(&l2->mut);
		list_pos_t pos;
		if (l1->ltype == LinkedList)
			pos.node = ((linkedlist_p)l1->list)->head;
		else
			pos.index = 0;
		element_p ele;
		int ex;
		while ((ele = __list_get_at(l1, pos))) {
			if (l2->ltype == LinkedList)
				ex = __linkedlist_search((linkedlist_p)l2->list, 0, Forward, l2->size, ele, l2->cmpfunc);
			else
				ex = __arraylist_search((arraylist_p)l2->list, 0, Forward, l2->size, ele, l2->cmpfunc);
			if (ex != -1)
				__list_del_at(l1, pos);
			else {
				if (l1->ltype == LinkedList)
					pos.node = pos.node->next;
				else
					pos.index++;
			}
		}
		pthread_mutex_unlock(&l1->mut);
		pthread_mutex_unlock(&l2->mut);
	}
}

void list_retain(Container list1, Container list2)
{
	list_p l1, l2;
	if (IS_VALID_LIST(list1) && IS_VALID_LIST(list2) &&
			(l1 = (list_p)list1->container)->etype == (l2 = (list_p)list2->container)->etype &&
			l2->size > 0) {
		pthread_mutex_lock(&l1->mut);
		pthread_mutex_lock(&l2->mut);
		list_pos_t pos;
		if (l1->ltype == LinkedList)
			pos.node = ((linkedlist_p)l1->list)->head;
		else
			pos.index = 0;
		element_p ele;
		int ex;
		while ((ele = __list_get_at(l1, pos))) {
			if (l2->ltype == LinkedList)
				ex = __linkedlist_search((linkedlist_p)l2->list, 0, Forward, l2->size, ele, l2->cmpfunc);
			else
				ex = __arraylist_search((arraylist_p)l2->list, 0, Forward, l2->size, ele, l2->cmpfunc);
			if (ex == -1)
				__list_del_at(l1, pos);
			else {
				if (l1->ltype == LinkedList)
					pos.node = pos.node->next;
				else
					pos.index++;
			}
		}
		pthread_mutex_unlock(&l1->mut);
		pthread_mutex_unlock(&l2->mut);
	}
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
 * @brief 把一个节点从链表中抽离出来
 *
 * @param ll
 * 	链表
 * @param node
 * 	待抽离的节点
 */
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
		for (i = size; i > index; i--)
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

/**
 * @brief 对链表进行快速排序
 *
 * @param left
 * 	左节点
 * @param right
 * 	右节点
 * @param order
 * 	排序顺序
 * @param cmpfunc
 * 	比较函数
 */
static void __linkedlist_quicksort(ll_node_p left, ll_node_p right, int order, CmpFunc cmpfunc)
{
	if(left == right)
		return;
	ll_node_p i = left;
	ll_node_p j = right;
	element_p key = left->element;
	while (i != j) {
		while (i != j && (order * cmpfunc(key->value, j->element->value, key->len, j->element->len)) <= 0)
			j = j->prev;
		if (i != j) {
			i->element = j->element;
			i = i->next;
		}
		while (i != j && (order * cmpfunc(key->value, i->element->value, key->len, i->element->len)) >= 0)
			i = i->next;
		if (i != j) {
			j->element = i->element;
			j = j->prev;
		}
	}
	i->element = key;
	if (left != i)
		__linkedlist_quicksort(left, i->prev, order, cmpfunc);
	if (right != i)
		__linkedlist_quicksort(i->next, right, order, cmpfunc);
}

/**
 * @brief 对线性表进行快速排序
 *
 * @param a
 * 	元素数组
 * @param left
 * 	左位置
 * @param right
 * 	右位置
 * @param order
 * 	排序顺序
 * @param cmpfunc
 * 	比较函数
 */
static void __arraylist_quicksort(element_p *a, size_t left, size_t right, int order, CmpFunc cmpfunc)
{
	if(left >= right)
		return;
	int i = left;
	int j = right;
	element_p key = a[left];
	while (i < j) {
		while (i < j && (order * cmpfunc(key->value, a[j]->value, key->len, a[j]->len)) <= 0)
			j--;
		if (i < j)
			a[i++] = a[j];
		while (i < j && (order * cmpfunc(key->value, a[i]->value, key->len, a[i]->len)) >= 0)
			i++;
		if (i < j)
			a[j--] = a[i];
	}
	a[i] = key;
	__arraylist_quicksort(a, left, i - 1, order, cmpfunc);
	__arraylist_quicksort(a, i + 1, right, order, cmpfunc);
}

/**
 * @brief 对链表进行插入排序
 *
 * @param left
 * 	左节点
 * @param right
 * 	右节点
 * @param order
 * 	排序顺序
 * @param cmpfunc
 * 	比较函数
 */
static void __linkedlist_insertsort(ll_node_p left, ll_node_p right, int order, CmpFunc cmpfunc)
{
	ll_node_p i, j;
	i = left->next;
	while (i != right->next) {
		element_p temp = i->element;
		j = i;
		while (j != left && (order * cmpfunc(j->prev->element->value, temp->value, j->prev->element->len, temp->len)) > 0) {
			j->element = j->prev->element;
			j = j->prev;
		}
		j->element = temp;
		i = i->next;
	}
}

/**
 * @brief 对线性表进行插入排序
 *
 * @param a
 * 	元素数组
 * @param left
 * 	左位置
 * @param right
 * 	右位置
 * @param order
 * 	排序顺序
 * @param cmpfunc
 * 	比较函数
 */
static void __arraylist_insertsort(element_p *a, size_t left, size_t right, int order, CmpFunc cmpfunc)
{
	int i, j;
	for (i = left + 1; i <= right; i++) {
		element_p temp = a[i];
		j = i;
		while (j > left && (order * cmpfunc(a[j - 1]->value, temp->value, a[j - 1]->len, temp->len)) > 0) {
			a[j] = a[j - 1];
			j--;
		}
		a[j] = temp;
	}
}

/**
 * @brief 对链表进行二分搜索
 *
 * @param e
 * 	搜索的元素
 * @param size
 * 	链表中的节点数量
 * @param order
 * 	表中元素的排列顺序
 * @param left
 * 	左节点
 * @param right
 * 	右节点
 * @param cmpfunc
 * 	比较函数
 *
 * @return 
 * 	找到的元素的位置，找不到返回-1
 */
static int __linkedlist_bisearch(element_p e, size_t size, int order, ll_node_p left, ll_node_p right, CmpFunc cmpfunc)
{
	int s = size;					// 每一轮搜索范围的节点数量
	ll_node_p m = left;				// 每一轮搜索范围的比较节点
	ll_node_p l = left;				// 每一轮搜索范围的左节点
	ll_node_p r = right;				// 每一轮搜索范围的右节点
	int pos = 0;					// 比较节点的位置
	int step, cmp, i;				// 跳数，比较结果，循环变量
	do {						// 每一轮循环都在一定的范围内找到中间节点并进行比较，循环开始时比较节点置于左端，循环中进行跳转
		step = (s - 1) / 2;			// 计算到中间节点的跳跃数
		for (i = 0; i < step; i++)
			m = m->next;			// 比较节点跳跃到中间节点
		pos += step;				// 比较节点节点的位置调整为中间节点的位置
		cmp = order * cmpfunc(e->value, m->element->value, e->len, m->element->len);
		if (cmp == 0)				// 中间节点就是要搜索的节点，直接返回中间节点的位置
			return pos;
		else if (cmp > 0) {			// 要搜索的元素大于中间节点的元素，在右半表(m->next, r)中继续循环
			l = m->next;			// 右半表的左节点是当前节点的后继
			m = l;				// 当前节点修正为右半表的左节点
			pos++;				// 修正当前节点的位置
			s = s - step - 1;		// 右半表的元素数量
		} else {				// 要搜索的元素小于中间节点的元素，在左半表(l, m->prev)中继续循环
			r = m->prev;			// 左半表的右节点等于当前节点的前驱
			m = l;				// 当前节点回归到左节点
			pos -= step;			// 修正当前节点的位置
			s = step;			// 左半表的元素数量
		}
	} while (s > 0);				// 下一轮循环已经没有元素的时候循环结束，元素没有找到
	return -1;
}

/**
 * @brief 对线性表进行二分搜索
 *
 * @param e
 * 	搜索的元素
 * @param a
 * 	元素数组
 * @param order
 * 	表中元素的排列顺序
 * @param left
 * 	左位置
 * @param right
 * 	右位置
 * @param cmpfunc
 * 	比较函数
 *
 * @return 
 * 	找到的元素的位置，找不到返回-1
 */
static int __arraylist_bisearch(element_p e, element_p *a, int order, int left, int right, CmpFunc cmpfunc)
{
	if (left > right)
		return -1;
	int mid = (left + right) / 2;
	int cmp = order * cmpfunc(e->value, a[mid]->value, e->len, a[mid]->len);
	if (cmp == 0)
		return mid;
	else if (cmp > 0)
		return __arraylist_bisearch(e, a, order, mid + 1, right, cmpfunc);
	else
		return __arraylist_bisearch(e, a, order, left, mid - 1, cmpfunc);
}

/**
 * @brief 反向排列链表
 *
 * @param head
 * 	头节点
 * @param tail
 * 	尾节点
 */
static void __linkedlist_reverse(ll_node_p head, ll_node_p tail)
{
	ll_node_p left = head;
	ll_node_p right = tail;
	element_p temp = NULL;
	while (left != right && left->prev != right) {
		temp = left->element;
		left->element = right->element;
		right->element = temp;
		left = left->next;
		right = right->prev;
	}
}

/**
 * @brief 反向排列线性表
 *
 * @param a
 * 	线性表
 * @param size
 * 	表内元素数量
 */
static void __arraylist_reverse(element_p *a, size_t size)
{
	int left = 0;
	int right = size - 1;
	element_p temp = NULL;
	while (left < right) {
		temp = a[left];
		a[left] = a[right];
		a[right] = temp;
		left++;
		right--;
	}
}

/**
 * @brief 创建一个列表的迭代器
 *
 * @param list
 * 	列表容器
 * @param dir
 * 	迭代方向
 *
 * @return 
 * 	列表迭代器
 */
static list_it_p __list_iterator(list_p list, int dir)
{
	list_it_p it = (list_it_p)malloc(sizeof(list_it_t));
	if (it) {
		it->list = list;
		if (list->ltype == LinkedList)
			it->pos.node = dir == Reverse ? ((linkedlist_p)list->list)->tail : ((linkedlist_p)list->list)->head;
		else
			it->pos.index = dir == Reverse ? list->size - 1 : 0;
		it->dir = dir;
		it->removable = 0;
		it->changes = list->changes;
	}
	return it;
}

/**
 * @brief 迭代访问下一个元素
 *
 * @param it
 * 	迭代器
 *
 * @return 
 * 	下一个元素，迭代结束后返回NULL直到迭代器被重置，迭代时发现Fast-Fail时将迭代器置为迭代结束状态并返回NULL
 */
static Element __list_it_next(void *it)
{
	Element ret = NULL;
	if (it && ((list_it_p)it)->list) {
		list_it_p i = (list_it_p)it;
		list_p l = i->list;
		pthread_mutex_lock(&l->mut);
		if (i->changes != l->changes) {
			if (l->ltype == LinkedList)
				i->pos.node = NULL;
			else
				i->pos.index = i->dir == Reverse ? -1 : l->size;
		}
		if (l->ltype == LinkedList && i->pos.node) {
			ret = __element_clone_value(i->pos.node->element);
			i->pos.node = i->dir == Reverse ? i->pos.node->prev : i->pos.node->next;
			i->removable = 1;
		} else if (l->ltype == ArrayList && i->pos.index >= 0 && i->pos.index < l->size) {
			ret = __element_clone_value(((arraylist_p)l->list)->elements[i->pos.index]);
			i->pos.index = i->dir == Reverse ? i->pos.index - 1 : i->pos.index + 1;
			i->removable = 1;
		}
		pthread_mutex_unlock(&l->mut);
	}
	return ret;
}

/**
 * @brief 删除上一次迭代得到的元素，没有迭代过的初始迭代器或连续调用都不会删除任何元素
 *
 * @param it
 * 	迭代器
 *
 * @return 
 * 	删除的元素数量，0或1
 */
static size_t __list_it_remove(void *it)
{
	size_t count = 0;
	if (it && ((list_it_p)it)->list && ((list_it_p)it)->removable) {
		list_it_p i = (list_it_p)it;
		list_p l = i->list;
		pthread_mutex_lock(&l->mut);
		if (l->ltype == LinkedList) {
			linkedlist_p ll = (linkedlist_p)l->list;
			ll_node_p n = i->dir == Reverse ? (i->pos.node ? i->pos.node->next : ll->head) : (i->pos.node ? i->pos.node->prev : ll->tail);
			__linkedlist_node_plugout(ll, n);
			__linkedlist_node_destroy(n);
		} else {
			element_p *a = ((arraylist_p)l->list)->elements;
			int dpos = i->dir == Reverse ? i->pos.index + 1 : i->pos.index - 1;
			__element_destroy(a[dpos]);
			for (int p = dpos + 1; p < l->size; p++)
				a[p - 1] = a[p];
			if (i->dir != Reverse)
				i->pos.index--;
		}
		i->removable = 0;
		l->size--;
		l->changes++;
		i->changes++;
		count++;
		pthread_mutex_unlock(&l->mut);
	}
	return count;
}

/**
 * @brief 重置迭代器
 *
 * @param it
 * 	迭代器
 */
static void __list_it_reset(void *it)
{
	if (it && ((list_it_p)it)->list) {
		list_it_p i = (list_it_p)it;
		pthread_mutex_lock(&i->list->mut);
		if (i->list->ltype == LinkedList)
			i->pos.node = i->dir == Reverse ? ((linkedlist_p)i->list->list)->tail : ((linkedlist_p)i->list->list)->head;
		else
			i->pos.index = i->dir == Reverse ? i->list->size - 1 : 0;
		i->changes = i->list->changes;
		i->removable = 0;
		pthread_mutex_unlock(&i->list->mut);
	}
}

/**
 * @brief 销毁迭代器
 *
 * @param it
 * 	迭代器
 */
static void __list_it_destroy(void *it)
{
	free(it);
}

/**
 * @brief 获取位置pos上的元素
 *
 * @param list
 * 	列表
 * @param pos
 * 	位置
 *
 * @return 
 * 	位置pos上的元素或NULL
 */
static element_p __list_get_at(list_p list, list_pos_t pos)
{
	if (list->ltype == LinkedList)
		return pos.node ? pos.node->element : NULL;
	else
		return (pos.index >= 0 && pos.index < list->size) ? ((arraylist_p)list->list)->elements[pos.index] : NULL;
}

/**
 * @brief 删除位置pos上的元素
 *
 * @param list
 * 	列表
 * @param pos
 * 	位置
 */
static void __list_del_at(list_p list, list_pos_t pos)
{
	if (list->ltype == LinkedList) {
		ll_node_p n = pos.node;
		pos.node = pos.node->next;
		__linkedlist_node_plugout((linkedlist_p)list->list, n);
		__linkedlist_node_destroy(n);
	} else {
		element_p *a = ((arraylist_p)list->list)->elements;
		__element_destroy(a[pos.index]);
		for (int i = pos.index + 1; i < list->size; i++)
			a[i - 1] = a[i];
	}
	list->size--;
	list->changes++;
}

/**
 * @brief 在列表最后添加一个元素，添加时复制元素而非引用元素
 *
 * @param list
 * 	列表
 * @param ele
 * 	元素
 */
static void __list_append(list_p list, element_p ele)
{
	element_p e = __element_create(ele->value, ele->type, ele->len);
	if (!e)
		return;
	if (list->ltype == LinkedList) {
		ll_node_p n = __linkedlist_node_create(e);
		if (!n)
			return;
		((linkedlist_p)list->list)->tail->next = n;
		n->prev = ((linkedlist_p)list->list)->tail;
		((linkedlist_p)list->list)->tail = n;
	} else {
		arraylist_p al = (arraylist_p)list->list;
		if (list->size <  al->capacity || __arraylist_expand(al) == 0)
			al->elements[list->size] = e;
	}
	list->size++;
	list->changes++;
}

