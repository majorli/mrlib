/**
 * "mr_containers.h"，容器管理工具
 *
 * 由容器类库函数使用，提供一个运行时动态存储管理容器的池，一般客户端不直接调用容器管理工具函数。
 * Version 0.0.1 李斌 20160223
 */
#ifndef MR_CONTAINERS_H
#define MR_CONTAINERS_H

/**
 * 通用的容器元素类型
 */
typedef void *Container;

/**
 * 在容器池中获取一个句柄并保存容器到池中。一般由各类容器的create函数调用，客户端无需直接调用本函数
 * container:	要保存到池中的容器
 *
 * 返回:	容器获得的句柄，是一个正整数，操作失败返回-1
 */
extern int container_retrieve(Container container);

/**
 * 从容器池中释放一个容器。一般由各类容器的free函数调用，客户端无需直接调用本函数
 * handler:	要释放的容器的句柄
 *
 * 返回:	被释放的容器，操作失败返回NULL
 */
extern Container container_release(int handler);

/**
 * 根据句柄从容器池中获得实际的容器
 * handler:	要获取的容器的句柄
 *
 * 返回:	获取的容器，无效的句柄返回NULL
 */
extern Container container_get(int handler);

#endif
