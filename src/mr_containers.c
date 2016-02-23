#include <stdlib.h>

#include "mr_containers.h"

static Container *containers_pool = NULL;		// 容器池
static size_t containers_capacity = 0;			// 容器池总容量
static size_t containers_elements = 0;			// 容器池中实际容器数量

static const size_t section_size = 20;			// 容器池扩容时每节的长度

static int *slots = NULL;				// 空隙句柄堆栈
static size_t slots_capacity = 0;			// 空隙句柄堆栈总容量
static size_t slots_top = 0;				// 空隙句柄数量

int container_retrieve(Container container);
int container_release(int handler);

static void pool_init(void);
static void pool_destroy(void);
static void pool_expand(void);

static int slots_pop(void);
static void slots_push(int slot);

/**
 * 在容器池中获取一个句柄并保存容器到池中。一般由各类容器的create函数调用，客户端无需直接调用本函数
 * container:	要保存到池中的容器
 *
 * 返回:	容器获得的句柄，是一个正整数，操作失败返回-1
 */
int container_retrieve(Container container)
{
	int ret = -1;
	if (containers_capacity == 0) {			// 池还没有初始化，先初始化池
		pool_init();
	}
	if ((ret = slots_pop()) == -1) {		// 没有空隙句柄，需要顺序取下一个句柄
		if (containers_capacity == containers_elements) {	// 池已经满了，需要进行扩容
			pool_expand();
		}
		ret = containers_elements;
	}
	containers_pool[ret] = container;
	containers_elements++;
	return ret;
}

/**
 * 从容器池中释放一个容器。一般由各类容器的free函数调用，客户端无需直接调用本函数
 * handler:	要释放的容器的句柄
 *
 * 返回:	被释放容器的句柄，操作失败返回-1
 */
int container_release(int handler)
{
	int ret = -1;
	if (containers_capacity > 0 && containers_elements > 0 && containers_pool[handler] != NULL) {	// 有效的释放
		ret = handler;
		containers_pool[handler] = NULL;
		if (handler < containers_elements + slots_top - 1) {		// 释放了一个中间节点，形成了一个空隙句柄
			slots_push(handler);
		}
		containers_elements--;
		if (containers_elements == 0) {					// 全部容器都释放了，池空，销毁容器池
			pool_destroy();
		}
	}
	return ret;
}

/**
 * 初始化容器池，容量为一个section_size，同时初始化空隙句柄堆栈，容量为一个section_size
 */
static void pool_init(void)
{
	// 初始化容器池
	containers_pool = (Container *)malloc(section_size * sizeof(Container));
	containers_capacity = section_size;
	containers_elements = 0;
	// 容器池中新增的节点全部设置为NULL
	for (int i = 0; i < section_size; i++) {
		containers_pool[i] = NULL;
	}
	// 初始化空隙句柄堆栈
	slots = (int *)malloc(section_size * sizeof(int));
	slots_capacity = section_size;
	slots_top = 0;
	return;
}

/**
 * 释放池空间，释放空隙句柄堆栈
 */
static void pool_destroy(void)
{
	free(containers_pool);
	containers_capacity = 0;
	containers_elements = 0;
	free(slots);
	slots_capacity = 0;
	slots_top = 0;
	return;
}

/**
 * 扩容容器池，扩容的容量为一个section_size
 */
static void pool_expand(void)
{
	size_t nc = containers_capacity + section_size;
	containers_pool = (Container *)realloc(containers_pool, nc * sizeof(Container));
	for (size_t i = containers_capacity; i < nc; i++) {
		containers_pool[i] = NULL;
	}
	containers_capacity = nc;
	return;
}

/**
 * 弹出空隙句柄堆栈的栈顶句柄，如果堆栈为空则返回-1
 */
static int slots_pop(void)
{
	int ret;
	if (slots_top == 0) {
		ret = -1;
	} else {
		ret = slots[--slots_top];
	}
	return ret;
}

/**
 * 压入一个空隙句柄到堆栈中，如果堆栈已满则以一个section_size为单位扩容堆栈
 * slot:	空隙句柄
 */
static void slots_push(int slot)
{
	if (slots_top == slots_capacity) {	// 堆栈已满，先扩容堆栈
		slots_capacity += section_size;
		slots = (int *)realloc(slots, slots_capacity * sizeof(int));
	}
	slots[slots_top++] = slot;
	return;
}

