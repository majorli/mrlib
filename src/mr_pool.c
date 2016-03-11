#include <stdlib.h>
#include <pthread.h>

#include "mr_pool.h"

#define IS_VALID_POOL(X) (X && X->container && X->type == Pool)

typedef struct IdleNode {				// 空闲句柄链栈的节点结构
	size_t handler;					// 空闲句柄
	struct IdleNode *next;				// 链表指针
} idle_node_t, *idle_node_p;

typedef struct {					// 池结构
	Element *elements;				// 元素池
	size_t capacity;				// 当前池容量
	size_t init_capa;				// 初始池容量
	size_t size;					// 当前元素数量
	idle_node_p next_idle;				// 空闲句柄链栈头指针
	pthread_mutex_t mut;				// 共享锁
} pool_t, *pool_p;

static Container __pool_create(size_t capacity);		// 创建一个池并封装成Container
static int __pool_retrieve(pool_p pool, Element element);	// 托管一个新的元素
static Element __pool_release(pool_p pool, int handler);	// 释放一个池节点

Container pool_create(size_t capacity)
{
	return __pool_create(capacity > 9 ? capacity : 10);
}

int pool_destroy(Container pool)
{
	int ret = -1;
	if (IS_VALID_POOL(pool)) {
		pool_p p = (pool_p)pool->container;
		pthread_mutex_lock(&p->mut);
		while (p->next_idle) {
			idle_node_p next = p->next_idle->next;
			free(p->next_idle);
			p->next_idle = next;
		}
		p->capacity = 0;
		free(p->elements);
		pthread_mutex_unlock(&p->mut);
		pthread_mutex_destroy(&p->mut);
		free(p);
		free(pool);
		ret = 0;
	}
	return ret;
}

size_t pool_size(Container pool)
{
	return IS_VALID_POOL(pool) ? ((pool_p)pool->container)->size : 0;
}

int pool_isempty(Container pool)
{
	return IS_VALID_POOL(pool) ? ((pool_p)pool->container)->size == 0 : 1;
}

double pool_ratio(Container pool)
{
	return IS_VALID_POOL(pool) ? (double)((pool_p)pool->container)->size / (double)((pool_p)pool->container)->capacity * 100.0 : 0.0;
}

int pool_retrieve(Container pool, Element element)
{
	int handler = -1;
	if (IS_VALID_POOL(pool) && element)
		handler = __pool_retrieve((pool_p)pool->container, element);
	return handler;
}

Element pool_release(Container pool, int handler)
{
	Element element = NULL;
	if (IS_VALID_POOL(pool) &&
			handler >= 0 && handler < ((pool_p)pool->container)->capacity &&
			((pool_p)pool->container)->elements[handler])
		element = __pool_release((pool_p)pool->container, handler);
	return element;
}

Element pool_get(Container pool, int handler)
{
	Element element = NULL;
	if (IS_VALID_POOL(pool) && handler >= 0 && handler < ((pool_p)pool->container)->capacity)
		element = ((pool_p)pool->container)->elements[handler];
	return element;
}

int pool_expand(Container pool)
{
	int ret = -1;
	if (IS_VALID_POOL(pool)) {
		pool_p p = (pool_p)pool->container;
		pthread_mutex_lock(&p->mut);
		size_t nc = p->capacity + p->init_capa;
		Element *tmp = (Element *)realloc(p->elements, nc * sizeof(Element));
		if (tmp) {
			p->elements = tmp;
			for (size_t i = p->capacity; i < nc; i++)
				p->elements[i] = NULL;
			p->capacity = nc;
			ret = 0;
		}
		pthread_mutex_unlock(&p->mut);
	}
	return ret;
}

int pool_shrink(Container pool)
{
	int ret = -1;
	if (IS_VALID_POOL(pool)) {
		pool_p p = (pool_p)pool->container;
		pthread_mutex_lock(&p->mut);
		idle_node_p tail = p->next_idle;
		while (tail->next)
			tail = tail->next;
		size_t nc = tail->handler + 10;
		if (p->capacity > nc && nc >= p->init_capa) {
			Element *tmp = (Element *)realloc(p->elements, nc * sizeof(Element));
			if (tmp) {
				p->elements = tmp;
				p->capacity = nc;
				ret = 0;
			}
		}
		pthread_mutex_unlock(&p->mut);
	}
	return ret;
}

int pool_removeall(Container pool, OnRemove onremove)
{
	int ret = -1;
	if (IS_VALID_POOL(pool)) {
		pool_p p = (pool_p)pool->container;
		pthread_mutex_lock(&p->mut);
		for (size_t i = 0; i < p->capacity; i++) {
			if (p->elements[i]) {
				onremove(p->elements[i]);
				p->elements[i] = NULL;
			}
		}
		p->size = 0;
		idle_node_p head;
		while (p->next_idle->next) {
			head = p->next_idle->next;
			free(p->next_idle);
			p->next_idle = head;
		}
		p->next_idle->handler = 0;
		ret = 0;
		pthread_mutex_unlock(&p->mut);
	}
	return ret;
}

/**
 * 池创建完成后的数据结构初始状态
 * 1. 池空间(pool->elements)
 * +---+---+---+---+
 * | 0 | 1 |...|C-1|		// C = pool->capacity
 * +---+---+---+---+
 * |nil|nil|nil|nil|
 * +---+---+---+---+
 * 2. 空闲链栈(pool->next_idle)
 *               +---+
 * next_idle --> | 0 | --> nil
 *               +---+
 */
static Container __pool_create(size_t capacity)
{
	Container cont = NULL;
	pool_p pool = NULL;
	Element *elements = NULL;
	idle_node_p next_idle = NULL;
	if ((cont = (Container)malloc(sizeof(Container_t))) &&
			(pool = (pool_p)malloc(sizeof(pool_t))) &&
			(elements = (Element *)malloc(capacity * sizeof(Element))) &&
			(next_idle = (idle_node_p)malloc(sizeof(idle_node_t)))) {
		for (size_t i = 0; i < capacity; i++)
			elements[i] = NULL;
		pool->elements = elements;
		pool->capacity = capacity;
		pool->init_capa = capacity;
		pool->size = 0;
		next_idle->handler = 0;
		next_idle->next = NULL;
		pool->next_idle = next_idle;
		pthread_mutex_init(&(pool->mut), NULL);
		cont->container = pool;
		cont->type = Pool;
	} else {
		free(next_idle);
		free(elements);
		free(pool);
		free(cont);
		cont = NULL;
	}
	return cont;
}

/**
 * 托管一个新的元素到池中，返回句柄
 * 如果池已满，返回-1
 * 托管一个新元素的算法
 * POOL-RETRIEVE(P, x)
 * 1	if size[P] == capacity[P]
 * 2		then ERR_CONTAINER_FULL
 * 3		     return -1
 * 4	h := handler[next_idle[P]]
 * 5	if next[next_idle[P]] == NULL
 * 6		then handler[next_idle[P]] := h + 1
 * 7		else pop next_idle[P]
 * 8	elements[P][h] := x
 * 9	size[P]++
 *10	return h
 */
static int __pool_retrieve(pool_p pool, Element element)
{
	int h = -1;
	pthread_mutex_lock(&pool->mut);
	if (pool->size < pool->capacity) {
		h = pool->next_idle->handler++;
		if (pool->next_idle->next) {
			idle_node_p head = pool->next_idle;
			pool->next_idle = head->next;
			free(head);
		}
		pool->elements[h] = element;
		pool->size++;
	}
	pthread_mutex_unlock(&pool->mut);
	return h;
}

/**
 * 释放一个池节点，返回其中的元素，调用前须确保池和句柄有效
 * 释放池节点的算法描述
 * POOL-RELEASE(P, h)
 * 1	e := elements[P][h]
 * 2	elements[P][h] := NULL
 * 3	size[P]--
 * 4	if size[P] == 0
 * 5		then init next_idle[P]
 * 6		else push h into next_idle[P]
 * 7	return e
 */
static Element __pool_release(pool_p pool, int handler)
{
	pthread_mutex_lock(&pool->mut);
	Element e = pool->elements[handler];
	if (pool->size == 1) {
		idle_node_p head;
		while (pool->next_idle->next) {
			head = pool->next_idle;
			pool->next_idle = head->next;
			free(head);
		}
		pool->next_idle->handler = 0;
	} else {
		idle_node_p idle = (idle_node_p)malloc(sizeof(idle_node_t));
		if (idle) {
			idle->handler = handler;
			idle->next = pool->next_idle;
			pool->next_idle = idle;
		} else {
			e = NULL;
		}
	}
	if (e) {
		pool->elements[handler] = NULL;
		pool->size--;
	}
	pthread_mutex_unlock(&pool->mut);
	return e;
}
