/**
 * "mr_pool.h"，实现容器：池
 *
 * 池提供动态申请节点用以存储元素的功能，客户程序建立一个资源池后，可以向池中存放各种类型的元素，池向客户程序返回一个整型的池节点的句柄，客户程序使用池节点句柄可以进行增删查改操作
 * 池不提供迭代器遍历，一般来说，池用于托管一段客户代码中需要反复使用的一项资源，而非对整个客户程序中所有托管资源进行遍历访问
 * 申请节点出错会返回无效句柄-1，如果是池容量不足则可以阻止线程进行等待或者使用pool_expand()函数来扩展池容量
 * 池扩展后不会随着资源的释放而自动缩小容量，因此不受控制地经常扩展可能造成内存浪费，也可以使用pool_shrink()函数来人为地缩小池的容量
 * 池容器不接受NULL元素或长度为0的元素
 *
 * Version 2.0.0, 李斌，2016/03/11
 */
#ifndef MR_POOL_H
#define MR_POOL_H

#include "mr_common.h"

/**
 * 创建一个容量为capacity个节点的池，capacity小于10时取下限10
 *
 * capacity
 *	池容量
 *
 * return	创建成功返回一个封装了池的容器，创建失败返回NULL
 */
extern Container pool_create(size_t capacity);

/**
 * 销毁池，但不销毁其中的元素
 *
 * pool
 *	待销毁的池容器
 *
 * return	销毁成功返回0，销毁失败返回-1
 */
extern int pool_destroy(Container pool);

/**
 * 获取池中元素数量
 *
 * pool
 *	池容器
 *
 * return	池中的元素数量，池为空或容器无效或容器不是池时返回0
 */
extern size_t pool_size(Container pool);

/**
 * 判断当前池是否为空
 *
 * pool
 *	池容器
 *
 * return	池中有元素返回0，池为空或容器无效或容器不是池时返回1
 */
extern int pool_isempty(Container pool);

/**
 * 获取池的使用率，使用百分率数值
 *
 * pool
 *	池容器
 *
 * return	池的使用率，即(元素数量/池容量)×100.0，容器无效或容器不是池时返回0.0
 */
extern double pool_ratio(Container pool);

/**
 * 托管一个元素到池中
 *
 * pool
 *	池容器
 * element
 *	要托管到池中的元素
 * type
 *	元素的类型
 * len
 *	元素的长度
 *	当元素类型为integer, real时，元素长度参数为元素实际类型的长度，例如sizeof(int)
 *	当元素类型为string时，元素长度一般为strlen(string)，也可以用元素长度限定存入容器的字符串的最大长度，即前len个字符，中文字符串要注意汉字截断问题
 *	当元素类型为object时，元素长度应为sizeof(object)
 *
 * return	托管成功返回一个非负整数的句柄，托管失败返回-1
 */
extern int pool_retrieve(Container pool, Element element, ElementType type, size_t len);

/**
 * 从池中释放一个元素并销毁池中保存的元素
 *
 * pool
 *	池容器
 * handler
 *	要释放的元素的句柄
 *
 * return	释放成功返回被释放的元素，释放失败返回NULL
 */
extern Element pool_release(Container pool, int handler);

/**
 * 从池中获取一个元素
 *
 * pool
 *	池容器
 * handler
 *	要获取的容器的句柄
 * 
 * return	获取成功返回句柄对应的元素，获取失败返回NULL
 */
extern Element pool_get(Container pool, int handler);

/**
 * 扩展池的容量，扩展的容量为池创建时的初始容量
 *
 * pool
 *	池容器
 *
 * return	扩展成功返回0，扩展失败返回-1
 */
extern int pool_expand(Container pool);

/**
 * 缩小池的容量，缩小到当前最后一个非空节点后剩余10个空闲节点
 * 只有当前容量 > 计算得出的缩小后容量 >= 初始容量的时候，缩小才会得到执行
 * 这样是为了确保池确实得到了缩小，缩小后不会过于拥挤，且不会缩小到比初始容量更小
 *
 * pool
 *	池容器
 *
 * return	缩小成功返回0，缩小失败返回-1
 */
extern int pool_shrink(Container pool);

/**
 * 清空池中所有元素并销毁所分配的内存
 *
 * pool
 *	要清空的池
 *
 * return	清空成功返回被清空的元素数量，清空失败返回-1
 */
extern int pool_removeall(Container pool);

#endif
