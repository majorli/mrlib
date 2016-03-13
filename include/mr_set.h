/**
 * "mr_set.h"，集合数据结构
 *
 * mr_set库提供一种基于红黑树构造的线性堆栈，用于实现有序集合。
 * 向集合中添加新元素时根据cmpfunc比较的结果构造二叉查找树，默认的cmpfunc在比较自定义对象时使用对象的地址进行比较，因此如果自定义对象使用默认比较函数时将无法保证迭代时的顺序
 * Set不提供按索引位置随机访问的功能，而是提供基于cmpfunc的有序迭代访问，因此Set不提供排序的功能
 * Set不支持重复的元素，元素是否重复根据cmpfunc确定，默认情况下数值和字符串根据内容判断，自定义对象根据地址判断，插入重复元素将不予保存；Set不接受NULL元素
 * 注意：集合运算的结果集中所有元素都是直接从原集合中引用的，所以销毁原集合中的元素会同时销毁结果集中的元素，反之亦然
 *
 * Version 2.0.0-Dev, 李斌，2016/03/13
 */
#ifndef MR_SET_H
#define MR_SET_H

#include "mr_common.h"

/**
 * 集合迭代器的封装类型
 */
typedef void *SetIterator;

/**
 * 创建一个集合
 *
 * type
 *	元素的类型
 * cmpfunc
 *	元素比较函数，传入NULL表示采用mr_common.h中定义的与type对应的默认比较函数
 *
 * return	新创建的集合，创建失败返回NULL
 */
extern Container set_create(ElementType type, CmpFunc cmpfunc);

/**
 * 销毁一个集合，但不会销毁其中的元素
 *
 * set
 *	集合容器
 *
 * return	销毁完成返回0，销毁失败或无效集合容器返回-1
 */
extern int set_destroy(Container set);

/**
 * 判断一个集合是否为空
 *
 * set
 *	集合容器
 *
 * return	为空返回1，不为空返回0，无效集合返回-1
 */
extern int set_isempty(Container set);

/**
 * 获取一个集合的元素数量
 *
 * set
 *	集合容器
 *
 * return	元素数量，空集合或无效集合返回0
 */
extern size_t set_size(Container set);

/**
 * 在集合中搜索一个元素
 *
 * set
 *	集合容器
 * element
 *	要搜索的元素
 *
 * return	搜索到集合中存在与ele相同的元素时返回集合中的元素，搜索不到或搜索出错返回NULL
 */
extern Element set_search(Container set, Element element);

/**
 * 添加一个元素，重复元素将不予添加
 *
 * set
 *	集合容器
 * element
 *	待添加的元素
 *
 * return	添加成功返回0，添加失败或元素重复返回-1
 */
extern int set_add(Container set, Element element);

/**
 * 删除一个元素，根据参数ele查找集合中与之相同的元素，删除并返回该元素
 *
 * set
 *	集合容器
 * element
 *	待删除的元素
 *
 * return	删除成功返回集合中的元素，删除失败或未找到返回NULL
 */
extern Element set_remove(Container set, Element element);

/**
 * 删除集合中的所有元素，被清除的元素用onremove函数进行后续处理
 *
 * set
 *	集合容器
 * onremove
 *	元素后续处理函数，NULL表示不做任何处理，典型的可以传入标准库函数free
 */
extern void set_removeall(Container set, OnRemove onremove);

/**
 * 获取一个集合的递增顺序迭代器
 *
 * set
 *	集合容器
 * 
 * return	集合迭代器，集合为空则返回NULL
 */
extern SetIterator set_iterator(Container set);

/**
 * 获取一个集合的递减反序迭代器
 *
 * set
 *	集合容器
 * 
 * return	集合迭代器，集合为空则返回NULL
 */
extern SetIterator set_riterator(Container set);

/**
 * 迭代访问一个迭代器中的下一个元素
 *
 * it
 *	集合迭代器
 *
 * return	迭代中的下一个元素，迭代器无效或已经迭代完成时返回NULL，再次迭代会从头开始
 */
extern Element set_next(SetIterator it);

/**
 * 求两个集合的交集，如果两个集合的元素数据类型不一致则返回空集合
 * 如果两个集合的元素比较函数不同则使用s1的cmpfunc进行元素比较，并且结果集合也采用s1的cmpfunc为其元素比较函数
 * 注意：集合运算的结果集中所有元素都是直接从原集合中引用的，所以销毁原集合中的元素会同时销毁结果集中的元素，反之亦然
 *
 * s1, s2
 *	两个用于运算的集合
 *
 * return	s1和s2的交集的句柄，是一个新建的集合，如果s1和s2中有至少一个无效，则返回-1
 */
extern Container setet_intersection(Container s1, Container s2);

/**
 * 求两个集合的并集，如果两个集合的元素数据类型不一致则返回空集合
 * 如果两个集合的元素比较函数不同则使用s1的cmpfunc进行元素比较，并且结果集合也采用s1的cmpfunc为其元素比较函数
 * 注意：集合运算的结果集中所有元素都是直接从原集合中引用的，所以销毁原集合中的元素会同时销毁结果集中的元素，反之亦然
 *
 * s1, s2
 *	两个用于运算的集合
 *
 * return	s1和s2的并集的句柄，是一个新建的集合，如果s1和s2中有至少一个无效，则返回-1
 */
extern Container setet_union(Container s1, Container s2);

/**
 * 求两个集合的减集，即s1-s2，从s1中删除所有存在于s2中的元素，如果两个集合的元素数据类型不一致则结果集与s1的元素相同
 * 如果两个集合的元素比较函数不同则使用s1的cmpfunc进行元素比较，并且结果集合也采用s1的cmpfunc为其元素比较函数
 * 注意：集合运算的结果集中所有元素都是直接从原集合中引用的，所以销毁原集合中的元素会同时销毁结果集中的元素，反之亦然
 *
 * s1, s2
 *	两个用于运算的集合
 *
 * return	集合s1-s2的句柄，是一个新建的集合，如果s1和s2中有至少一个无效，则返回-1
 */
extern Container setet_minus(Container s1, Container s2);

#endif