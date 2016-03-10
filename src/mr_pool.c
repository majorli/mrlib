#include <stdlib.h>
#include <pthread.h>

#include "mr_pool.h"

typedef struct IdleNode {				// 空闲句柄链栈的节点结构
	int handler;					// 空闲句柄
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

/**
 * 创建一个容量为capacity个节点的池
 *
 * 参数:	capacity	池容量
 *
 * 返回:	创建成功返回一个封装了池的容器，创建失败返回NULL
 *
 * 错误:	1. 参数capacity == 0，错误码ERR_INVALID_PARAMETER
 *		2. 内存不足，错误码ERR_OUT_OF_MEMORY
 */
Container pool_create(size_t capacity)
{
	Container ret = NULL;
	if (capacity == 0)
		set_ecode(ERR_INVALID_PARAMETER);
	else if ((ret = __pool_create(capacity)) == NULL)
		set_ecode(ERR_OUT_OF_MEMORY);
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
 * 获取一个空闲池节点的算法
 * NEXT-IDLE(P)
 * 1	h = handler[P->next_idle]
 * 2	if next[P->next_idle] == NULL
 * 3		then handler[P->next_idle] := h + 1
 * 4		else do t := next[P->next_idle]
 * 5			free P->next_idle
 * 6			P->next_idle := t
 */
static int __pool_retrieve_next_idle(idle_node_p next_idle)
{
	// TODO
	int idle = -1;
	return idle;
}

// ======================= LEGACY PRIVATE FUNCTIONS ==================================================
/**
 * 释放池空间，释放空隙句柄堆栈
 //
static void pool_destroy(void)
{
	free(containers_pool);
	containers_pool = NULL;
	containers_capacity = 0;
	containers_elements = 0;
	free(slots);
	slots = NULL;
	slots_capacity = 0;
	slots_top = 0;
	return;
}

**
 * 扩容容器池，扩容的容量为一个SECTION_SIZE
 //
static int pool_expand(void)
{
	int ret = -1;
	size_t nc = containers_capacity + SECTION_SIZE;
	node_p *tmp = (node_p *)realloc(containers_pool, nc * sizeof(node_p));
	if (tmp) {
		containers_pool = tmp;
		for (size_t i = containers_capacity; i < nc; i++)
			containers_pool[i] = NULL;
		containers_capacity = nc;
		ret = 0;
	}
	return ret;
}

**
 * 弹出空隙句柄堆栈的栈顶句柄，如果堆栈为空则返回-1
 //
static int slots_pop(void)
{
	return slots_top == 0 ? -1 : slots[--slots_top];
}

**
 * 获取空隙句柄堆栈的栈顶句柄，如果堆栈为空则返回-1
 //
static int slots_peak(void)
{
	return slots_top == 0 ? -1 : slots[slots_top];
}

**
 * 压入一个空隙句柄到堆栈中，如果堆栈已满则以一个SECTION_SIZE为单位扩容堆栈
 * slot:	空隙句柄
 //
static void slots_push(int slot)
{
	if (slots_top == slots_capacity) {	// 堆栈已满，先扩容堆栈
		slots_capacity += SECTION_SIZE;
		slots = (int *)realloc(slots, slots_capacity * sizeof(int));
	}
	slots[slots_top++] = slot;
	return;
}
*/

// ============ LEGACY PUBLIC FUNCTIONS ==========================================================
/**
 * 在容器池中获取一个句柄并保存容器到池中。一般由各类容器的create函数调用，客户端无需直接调用本函数
 * container:	要保存到池中的容器
 * type:	容器的具体类型
 *
 * 返回:	容器获得的句柄，是一个正整数，操作失败返回-1
//
int container_retrieve(Container container, ContainerType type)
{
	if (is_concurrency()) {
		if (!containers_pool)
			pthread_mutex_init(&pool_mut, NULL);
		pthread_mutex_lock(&pool_mut);
	}
	int ret = -1;
	if (containers_pool || pool_init() == 0) {				// 如果池还没有初始化，就先初始化池，如果初始化失败则失败返回
		if (((ret = slots_peak()) == -1) && (containers_capacity < containers_elements || pool_expand() == 0))
			ret = containers_elements;
		// 此时ret==-1的唯一可能是没有空隙而且容器池已经满而且扩展容器池失败
		if (ret != -1)
			if ((containers_pool[ret] = (node_p)malloc(sizeof(node_t)))) {
				containers_pool[ret]->container = container;
				containers_pool[ret]->type = type;
				containers_elements++;
				if (ret != containers_elements)
					slots_pop();
			} else {
				ret = -1;
			}
	}
	if (is_concurrency()) {
		pthread_mutex_unlock(&pool_mut);
	}
	return ret;
}

**
 * 从容器池中释放一个容器。一般由各类容器的free函数调用，客户端无需直接调用本函数
 * handler:	要释放的容器的句柄
 * type:	容器的具体类型
 *
 * 返回:	被释放的容器，操作失败返回NULL
//
Container container_release(int handler, ContainerType type)
{
	if (is_concurrency()) {
		pthread_mutex_lock(&pool_mut);
	}
	Container ret = NULL;
	if (containers_capacity > 0 && containers_elements > 0 && handler >= 0 && handler < containers_capacity && containers_pool[handler] != NULL && containers_pool[handler]->type == type) {
		ret = containers_pool[handler]->container;
		free(containers_pool[handler]);
		containers_pool[handler] = NULL;
		if (handler < containers_elements + slots_top - 1) {		// 释放了一个中间节点，形成了一个空隙句柄
			slots_push(handler);
		}
		containers_elements--;
		if (containers_elements == 0) {					// 全部容器都释放了，池空，销毁容器池
			pool_destroy();
		}
	}
	if (is_concurrency()) {
		pthread_mutex_unlock(&pool_mut);
		if (containers_pool == NULL) {
			pthread_mutex_destroy(&pool_mut);
		}
	}
	return ret;
}

**
 * 根据句柄从容器池中获得实际的容器
 * handler:	要获取的容器的句柄
 * type:	容器的具体类型
 *
 * 返回:	获取的容器，无效的句柄返回NULL
 //
Container container_get(int handler, ContainerType type)
{
	if (is_concurrency()) {
		pthread_mutex_lock(&pool_mut);
	}
	Container ret = NULL;
	if (containers_capacity > 0 && containers_elements > 0 && handler >= 0 && handler < containers_capacity && containers_pool[handler] != NULL && containers_pool[handler]->type == type) {
		ret = containers_pool[handler]->container;
	}
	if (is_concurrency()) {
		pthread_mutex_unlock(&pool_mut);
	}
	return ret;
}
*/

