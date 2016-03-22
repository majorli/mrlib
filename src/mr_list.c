#include <stdlib.h>
#include <pthread.h>

#include "mr_set.h"
#include "private_element.h"

#define IS_VALID_LIST(X) (X && X->container && X->type == List)

#define AL_INIT_CAPA 10

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
		list->cmpfunc = cmpfunc ? cmpfunc : default_cmpfunc(etype);
		list->size = 0;
		list->changes = 0;
		pthread_mutex_init(&list->mut, NULL);
	}
	return cont;
}

int list_destroy(Container list)
{
	return -1;
}

int list_isempty(Container list)
{
	return -1;
}

int list_size(Container list)
{
	return -1;
}

int list_append(Container list, Element element, ElementType type, size_t len)
{
	return -1;
}

int list_insert(Container list, size_t index, Element element, ElementType type, size_t len)
{
	return -1;
}

Element list_get(Container list, size_t index)
{
	return NULL;
}

Element list_remove(Container list, size_t index)
{
	return NULL;
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

void list_push(Container list, Element element, ElementType type, size_t len)
{
	return;
}

Element list_pop(Container list)
{
	return NULL;
}

Element list_peak(Container list)
{
	return NULL;
}

void list_enqueue(Container list, Element element, ElementType type, size_t len)
{
	return;
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

