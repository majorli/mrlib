/**
 * "mr_pqueue.h"，优先级队列库
 * 实现一个基于二叉堆构建的优先级队列
 * 优先级队列是元素分优先级的先进先出容器，也可以用迭代器按顺序访问，但是不能反向迭代，也不能在迭代过程中修改、删除元素
 * 优先级队列的迭代器同样使用Fast-Fail模式，如果在迭代过程中目标列表被其他线程或其他迭代器修改，则迭代立即终止，再次迭代需要重置迭代器
 * 优先级队列可以修改元素的优先级，但是不能从队列中间删除元素
 * 优先级队列可以存入重复元素（元素值相等）和空元素（元素值为NULL或元素长度为0），元素默认比较时，元素值为NULL的空元素会认为比元素值不为NULL但元素长度为0的元素更小
 * 优先级队列是强类型容器，在创建时必须指定一种元素类型，表内只能存取相同类型的元素，也可以使用object类型以实现多类型容器
 * 优先级队列按优先级数值越大优先级越高还是越小优先级越高，分为大优先级队列和小优先级队列
 *
 * 2.0.0-DEV, 李斌, 2016/03/28
 */
#ifndef MR_PQUEUE_H
#define MR_PQUEUE_H

#include "mr_common.h"

typedef enum {
	Max_Priority,
	Min_Priority
} PriorityType;

/**
 * @brief 创建一个优先级队列
 *
 * @param ptype
 * 	队列优先级顺序类型
 * @param etype
 * 	元素的数据类型
 * @param cmpfunc
 * 	元素比较函数，NULL则根据etype取默认的元素比较函数
 *
 * @return 
 * 	新建的优先级队列，创建失败返回NULL
 */
extern Container pq_create(PriorityType ptype, ElementType etype, CmpFunc cmpfunc);

/**
 * @brief 销毁一个优先级队列，销毁其中的所有元素
 *
 * @param pq
 * 	优先级队列容器
 *
 * @return 
 * 	销毁成功返回0，销毁失败返回-1
 */
extern int pq_destroy(Container pq);

/**
 * @brief 判断队列是否为空
 *
 * @param pq
 * 	优先级队列容器
 *
 * @return 
 * 	空队列返回1，否则返回0，队列容器无效返回1
 */
extern int pq_isempty(Container pq);

/**
 * @brief 获取队列中元素的数量
 *
 * @param pq
 * 	优先级队列
 *
 * @return 
 * 	队列容器中的元素数量，队列容器无效时返回0
 */
extern size_t pq_size(Container pq);
extern int pq_enqueue(Container pq, Element ele, ElementType type, size_t len, int priority);
extern Element pq_dequeue(Container pq);
extern Element pq_queuehead(Container pq);
extern int pq_contains(Container pq, Element ele, ElementType type, size_t len);
extern int pq_change_pri(Container pq, Element ele, ElementType type, size_t len, int priority);

/**
 * @brief 清空队列，销毁其中所有元素
 *
 * @param pq
 * 	优先级队列容器
 *
 * @return 
 * 	清空的元素数量
 */
extern int pq_removeall(Container pq);
extern Iterator pq_iterator(Container pq);

#endif
