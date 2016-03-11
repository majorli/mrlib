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

static Container __pool_create(size_t capacity);	// 创建一个池并封装成Container
static int __pool_retrieve(pool_p pool, Element element);	// 托管一个新的元素
static Element __pool_release(pool_p pool, int handler);	// 释放一个池节点

/**
 * 创建一个容量为capacity个节点的池
 *
 * 参数:	capacity	池容量
 *
 * 返回:	创建成功返回一个封装了池的容器，创建失败返回NULL
 */
Container pool_create(size_t capacity)
{
	return __pool_create(capacity > 9 ? capacity : 10);
}

/**
 * 销毁池，但不销毁其中的元素
 *
 * 参数:	pool		待销毁的池容器
 *
 * 返回:	销毁成功返回0，销毁失败返回-1
 */
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

/**
 * 获取池中元素数量
 *
 * 参数:	pool		池容器
 *
 * 返回:	池中的元素数量，池为空或容器无效或容器不是池时返回0
 */
size_t pool_size(Container pool)
{
	return IS_VALID_POOL(pool) ? ((pool_p)pool->container)->size : 0;
}

/**
 * 判断当前池是否为空
 *
 * 参数:	pool		池容器
 *
 * 返回:	池中有元素返回0，池为空或容器无效或容器不是池时返回1
 */
int pool_isempty(Container pool)
{
	return IS_VALID_POOL(pool) ? ((pool_p)pool->container)->size == 0 : 1;
}

/**
 * 获取池的使用率，使用百分率数值
 *
 * 参数:	pool		池容器
 *
 * 返回:	池的使用率，即(元素数量/池容量)×100.0，容器无效或容器不是池时返回0.0
 */
double pool_ratio(Container pool)
{
	return IS_VALID_POOL(pool) ? (double)((pool_p)pool->container)->size / (double)((pool_p)pool->container)->capacity * 100.0 : 0.0;
}

/**
 * 托管一个元素到池中
 *
 * 参数:	pool		池容器
 *		element		要托管到池中的元素
 *
 * 返回:	托管成功返回一个非负整数的句柄，托管失败返回-1
 */
int pool_retrieve(Container pool, Element element)
{
	int handler = -1;
	if (IS_VALID_POOL(pool) && element)
		handler = __pool_retrieve((pool_p)pool->container, element);
	return handler;
}

/**
 * 从池中释放一个元素
 *
 * 参数:	pool		池容器
 * 		handler		要释放的元素的句柄
 *
 * 返回:	释放成功返回被释放的元素，释放失败返回NULL
 */
Element pool_release(Container pool, int handler)
{
	Element element = NULL;
	if (IS_VALID_POOL(pool) &&
			handler >= 0 && handler < ((pool_p)pool->container)->capacity &&
			((pool_p)pool->container)->elements[handler])
		element = __pool_release((pool_p)pool->container, handler);
	return element;
}

/**
 * 从池中获取一个元素
 *
 * 参数:	pool		池容器
 * 		handler		要获取的容器的句柄
 * 
 * 返回:	获取成功返回句柄对应的元素，获取失败返回NULL
 */
Element pool_get(Container pool, int handler)
{
	Element element = NULL;
	if (IS_VALID_POOL(pool) && handler >= 0 && handler < ((pool_p)pool->container)->capacity)
		element = ((pool_p)pool->container)->elements[handler];
	return element;
}

/**
 * 扩展池的容量，扩展的容量为池创建时的初始容量
 *
 * 参数:	pool		池容器
 *
 * 返回:	扩展成功返回0，扩展失败返回-1
 */
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

/**
 * 缩小池的容量，缩小到当前最后一个非空节点后剩余10个空闲节点
 * 只有当前容量 > 计算得出的缩小后容量 >= 初始容量的时候，缩小才会得到执行
 * 这样是为了确保池确实得到了缩小，缩小后不会过于拥挤，且不会缩小到比初始容量更小
 *
 * 参数:	pool		池容器
 *
 * 返回:	缩小成功返回0，缩小失败返回-1
 */
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

/**
 * 清空池中所有元素，使用指定的方式对元素进行处置
 *
 * 参数:	pool		要清空的池
 *		onremove	用于处置池中元素的函数指针，NULL表示不对元素进行后续处置
 *
 * 返回:	清空成功返回被清空的元素数量，清空失败返回-1
 */
int pool_removeall(Container pool, onRemove onremove)
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
 * | 0 | 1 |...|C-1|		// c = capacity
 * +---+---+---+---+
 * |nil|nil|nil|nil|
 * +---+---+---+---+
 * 2. 空闲链栈(pool->next_idle)
 *               +---+
 * next_idle --> | 0 | --> nil
 *               +---+
 */
static Container __pool_create(size_t capacity)		// 创建一个池并封装成Container
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
static int __pool_retrieve(pool_p pool, Element element)	// 托管一个新的元素
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
static Element __pool_release(pool_p pool, int handler)		// 释放一个池节点
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
