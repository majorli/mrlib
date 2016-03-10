/**
 * "mr_pool.h"，实现容器：池
 *
 * 池提供动态申请节点用以存储元素的功能，客户程序建立一个资源池后，可以向池中存放各种类型的元素，池向客户程序返回一个整型的池节点的句柄，客户程序使用池节点句柄可以进行增删查改操作
 * 池不提供迭代器遍历，一般来说，池用于托管一段客户代码中需要反复使用的一项资源，而非对整个客户程序中所有托管资源进行遍历访问
 * 申请节点出错会返回无效句柄-1，此时客户程序可以通过errcode()函数来查询错误代码，如果是池容量不足则可以阻止线程进行等待或者使用pool_expand()函数来扩展池容量
 * 池扩展后不会随着资源的释放而缩小容量，因此不受控制地经常扩展可能造成内存浪费
 *
 * Version 2.0.0-Dev, 李斌，2016/03/10
 */
#ifndef MR_POOL_H
#define MR_POOL_H

#include "mr_common.h"

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
extern Container pool_create(size_t capacity);

/**
 * 销毁池，但不销毁其中的元素
 *
 * 参数:	pool		待销毁的池容器
 *
 * 返回:	销毁成功返回0，销毁失败返回-1
 *
 * 错误:	容器无效或不是一个池，错误码ERR_INVALID_PARAMETER
 */
extern int pool_destroy(Container pool);

/**
 * 获取池中元素数量
 *
 * 参数:	pool		池容器
 *
 * 返回:	池中的元素数量，池为空或容器无效或容器不是池时返回0
 */
extern size_t pool_size(Container pool);

/**
 * 判断当前池是否为空
 *
 * 参数:	pool		池容器
 *
 * 返回:	池中有元素返回0，池为空或容器无效或容器不是池时返回1
 */
extern int pool_isempty(Container pool);

/**
 * 获取池的使用率，使用百分率数值
 *
 * 参数:	pool		池容器
 *
 * 返回:	池的使用率，即(元素数量/池容量)×100.0，容器无效或容器不是池时返回0.0
 */
extern double pool_ratio(Container pool);

/**
 * 托管一个元素到池中
 *
 * 参数:	container	要托管到池中的元素
 *
 * 返回:	托管成功返回一个非负整数的句柄，托管失败返回-1
 *
 * 错误:	1. 池容量不足，错误码ERR_CONTAINER_FULL
 *		2. 内存不足，错误码ERR_OUT_OF_MEMORY
 */
extern int pool_retrieve(Element element);

/**
 * 从池中释放一个元素
 *
 * 参数:	handler		要释放的元素的句柄
 *
 * 返回:	释放成功返回被释放的元素，释放失败返回NULL
 *
 * 错误:	1. 句柄无效，错误码ERR_INVALID_PARAMETER
 */
extern Element pool_release(int handler);

/**
 * 从池中获取一个元素
 *
 * 参数:	handler		要获取的容器的句柄
 * 
 * 返回:	获取成功返回句柄对应的元素，获取失败返回NULL
 *
 * 错误:	1. 句柄无效，错误码ERR_INVALID_PARAMETER
 */
extern Element pool_get(int handler);

/**
 * 扩展池的容量
 *
 * 参数:	ext		要扩展的容量(ext个元素)，0表示扩展一个初始容量
 *
 * 返回:	扩展成功返回0，扩展失败返回-1
 *
 * 错误:	1. 内存不足，错误码ERR_OUT_OF_MEMORY
 */
extern int pool_expand(size_t ext);

/**
 * 清空池中所有元素，使用指定的方式对元素进行处置
 *
 * 参数:	pool		要清空的池
 *		onremove	用于处置池中元素的函数指针，NULL表示不对元素进行后续处置
 *
 * 返回:	清空成功返回被清空的元素数量，清空失败返回-1
 *
 * 错误:	容器无效或不是一个池，错误码ERR_INVALID_PARAMETER
 */
extern int pool_removeall(Container pool);

#endif