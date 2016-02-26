/**
 * "mr_containers.h"，容器管理工具
 *
 * 由容器类库函数使用，提供一个运行时动态存储管理容器的池，一般客户端不直接调用容器管理工具函数。
 * Version 0.0.1 李斌 20160223
 */
#ifndef MR_CONTAINERS_H
#define MR_CONTAINERS_H

#include "mr_common.h"

/**
 * 容器的基本类型
 */
typedef void *Container;

/**
 * 容器具体类型枚举
 */
typedef enum {
	ArrayList_t,
	LinkedList_t,
	HashSet_t,
	LinkedHashSet_t,
	Stack_t,
	Queue_t,
	CircularQueue_t,
	HashMap_t,
	LinkedHashMap_t,
	BTree_t,
	Tree_t
} ContainerType;

/**
 * 在容器池中获取一个句柄并保存容器到池中。一般由各类容器的create函数调用，客户端无需直接调用本函数
 * container:	要保存到池中的容器
 * type:	容器的具体类型
 *
 * 返回:	容器获得的句柄，是一个正整数，操作失败返回-1
 */
extern int container_retrieve(Container container, ContainerType type);

/**
 * 从容器池中释放一个容器。一般由各类容器的free函数调用，客户端无需直接调用本函数
 * handler:	要释放的容器的句柄
 * type:	容器的具体类型
 *
 * 返回:	被释放的容器，操作失败返回NULL
 */
extern Container container_release(int handler, ContainerType type);

/**
 * 根据句柄从容器池中获得实际的容器
 * handler:	要获取的容器的句柄
 * type:	容器的具体类型
 *
 * 返回:	获取的容器，无效的句柄返回NULL
 */
extern Container container_get(int handler, ContainerType type);

#endif
