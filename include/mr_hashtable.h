/**
 * "mr_hashtable.h"，基于线性探测的三值哈希表
 * 哈希表实现的功能是元素的登记和查询，可以通过一个迭代器进行顺序的读取访问，可以在迭代过程中删除元素，但是不能修改元素
 * 哈希表不保证元素的顺序，也不能进行排序，不支持按索引随机访问
 * 哈希表不可以存入重复元素（元素值相等）或空元素（元素值为NULL或元素长度为0）
 * 哈希表不限制元素的数据类型，可以存放任何类型的元素
 *
 * 2.0.0, 李斌, 2016/03/30
 */
#ifndef MR_HASHTABLE_H
#define MR_HASHTABLE_H

#include "mr_common.h"

/**
 * @brief 创建一个哈希表
 *
 * @return 
 * 	哈希表容器，创建失败返回NULL
 */
extern Container hash_create(void);

/**
 * @brief 销毁一个哈希表容器，销毁其中所有保存的元素
 *
 * @param hash
 * 	哈希表容器
 *
 * @return 
 * 	销毁成功返回0，失败返回-1
 */
extern int hash_destroy(Container hash);

/**
 * @brief 判断哈希表容器是否为空
 *
 * @param hash
 * 	哈希表容器
 *
 * @return 
 * 	为空或容器无效返回1，非空返回0
 */
extern int hash_isempty(Container hash);

/**
 * @brief 获取哈希表容器中的元素数量
 *
 * @param hash
 * 	哈希表容器
 *
 * @return 
 * 	元素数量，失败返回0
 */
extern size_t hash_size(Container hash);

/**
 * @brief 向哈希表中注册一个新元素
 *
 * @param hash
 * 	哈希表容器
 * @param ele
 * 	元素值
 * @param type
 * 	元素数据类型
 * @param len
 * 	元素长度
 *
 * @return 
 * 	注册成功返回0，注册失败或相同的元素已经存在返回-1
 */
extern int hash_register(Container hash, Element ele, ElementType type, size_t len);

/**
 * @brief 判断一个元素是否在哈希表中已经注册存在
 *
 * @param hash
 * 	哈希表容器
 * @param ele
 * 	元素值
 * @param type
 * 	元素数据类型
 * @param len
 * 	元素长度
 *
 * @return 
 * 	元素存在返回1，不存在或查找失败返回0
 */
extern int hash_contains(Container hash, Element ele, ElementType type, size_t len);

/**
 * @brief 从哈希表中删除一个元素
 *
 * @param hash
 * 	哈希表容器
 * @param ele
 * 	元素值
 * @param type
 * 	元素数据类型
 * @param len
 * 	元素长度
 *
 * @return 
 * 	被删除的元素数量
 */
extern int hash_remove(Container hash, Element ele, ElementType type, size_t len);

/**
 * @brief 清空哈希表容器中的所有元素j
 *
 * @param hash
 * 	哈希表容器
 *
 * @return 
 * 	清空成功返回0，失败返回-1
 */
extern int hash_removeall(Container hash);

/**
 * @brief 获取哈希表的迭代器
 *
 * @param hash
 * 	哈希表容器
 *
 * @return 
 * 	迭代器，获取失败返回NULL
 */
extern Iterator hash_iterator(Container hash);

#endif
