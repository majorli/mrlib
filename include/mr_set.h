/**
 * "mr_set.h"，集合数据结构
 *
 * mr_set库提供一种基于红黑树构造的有序集合
 * 向集合中添加新元素时根据cmpfunc比较的结果构造二叉查找树，如果创建集合时cmpfunc参数为NULL，则根据type参数选用默认的比较函数
 * 集合容器不提供按索引位置随机访问的功能，而是提供基于cmpfunc的有序迭代访问，也由于集合容器本身具备有序迭代的能力，因此不提供排序功能
 * 集合容器中不能存放重复的元素，元素是否重复根据cmpfunc确定，默认情况下数值和字符串根据内容判断，Object进行逐字节比较
 * 集合容器为强类型容器，创建时指定容器中元素的类型，与创建时指定的类型不同的元素无法存储，且集合容器不接受NULL元素
 * 如果创建集合容器时使用object元素类型，则相当于在容器中可以存储不同类型数据的能力，此时注意需提供符合客户程序需要的cmpfunc函数
 *
 * Version 2.0.1, 李斌，2016/03/23
 */
#ifndef MR_SET_H
#define MR_SET_H

#include "mr_common.h"

/**
 * @brief 创建一个集合
 *
 * @param type
 *	元素的类型
 * @param cmpfunc
 *	元素比较函数，传入NULL表示采用与type对应的默认比较函数
 *
 * @return
 *	新创建的集合，创建失败返回NULL
 */
extern Container set_create(ElementType type, CmpFunc cmpfunc);

/**
 * @brief 销毁一个集合及其中的所有元素
 *
 * @param set
 *	集合容器
 *
 * @return
 *	销毁完成返回0，销毁失败或无效集合容器返回-1
 */
extern int set_destroy(Container set);

/**
 * @brief 判断一个集合是否为空
 *
 * @param set
 *	集合容器
 *
 * @return
 *	为空返回1，不为空返回0，无效集合返回1
 */
extern int set_isempty(Container set);

/**
 * @brief 获取一个集合的元素数量
 *
 * @param set
 *	集合容器
 *
 * @return
 *	元素数量，空集合或无效集合返回0
 */
extern size_t set_size(Container set);

/**
 * @brief 判断在集合中是否存在指定元素
 *
 * @param set
 *	集合容器
 * @param element
 *	要搜索的元素
 * @param type
 *	要搜索的元素的类型
 * @param len
 *	要搜索的元素的长度
 *	当元素类型为integer, real时，元素长度参数为元素实际类型的长度，例如sizeof(int)
 *	当元素类型为string时，元素长度一般为strlen(string)，也可以用元素长度限定查找的长度，即前len个字符
 *	当元素类型为object时，元素长度应为sizeof(object)
 *
 * @return
 *	集合中存在与element相同的元素时返回1，搜索不到或搜索出错返回0
 */
extern int set_contains(Container set, Element element, ElementType type, size_t len);

/**
 * @brief 添加一个元素，重复元素将不予添加
 *
 * @param set
 *	集合容器
 * @param element
 *	待添加的元素
 * @param type
 *	元素的类型
 * @param len
 *	元素的长度
 *	当元素类型为integer, real时，元素长度参数为元素实际类型的长度，例如sizeof(int)
 *	当元素类型为string时，元素长度一般为strlen(string)，也可以用元素长度限定存入容器的字符串的最大长度，即前len个字符，中文字符串要注意汉字截断问题
 *	当元素类型为object时，元素长度应为sizeof(object)
 *
 * @return
 *	添加成功返回0，添加失败或元素重复返回-1
 */
extern int set_add(Container set, Element element, ElementType type, size_t len);

/**
 * @brief 删除一个元素，根据参数ele查找集合中与之相同的元素，删除并返回该元素
 *
 * @param set
 *	集合容器
 * @param element
 *	待删除的元素
 * @param type
 *	待删除的元素的类型
 * @param len
 *	待删除的元素的长度
 *	当元素类型为integer, real时，元素长度参数为元素实际类型的长度，例如sizeof(int)
 *	当元素类型为string时，元素长度一般为strlen(string)，也可以用元素长度限定查找的长度，即前len个字符
 *	当元素类型为object时，元素长度应为sizeof(object)
 *
 * @return
 *	删除的元素的数量，如果元素不存在或者其他原因删除失败则返回0
 */
extern size_t set_remove(Container set, Element element, ElementType type, size_t len);

/**
 * @brief 删除集合中的所有元素，被清除的元素用onremove函数进行后续处理
 *
 * @param set
 *	集合容器
 */
extern void set_removeall(Container set);

/**
 * @brief 获取一个集合的递增顺序迭代器
 *
 * @param set
 *	集合容器
 * @param dir
 *	迭代方向，Forward(1)或Reverse(0)
 * 
 * @return
 *	集合迭代器，集合为空则返回NULL
 */
extern Iterator set_iterator(Container set, int dir);

/**
 * @brief 求两个集合的交集，如果两个集合的元素数据类型不一致则返回空集合
 * 如果两个集合的元素比较函数不同则使用s1的cmpfunc进行元素比较，并且结果集合也采用s1的cmpfunc为其元素比较函数
 *
 * @param s1, s2
 *	两个用于运算的集合
 *
 * @return
 *	s1和s2的交集的句柄，是一个新建的集合，如果s1和s2中有至少一个无效，则返回-1
 */
extern Container set_intersection(Container s1, Container s2);

/**
 * @brief 求两个集合的并集，如果两个集合的元素数据类型不一致则返回空集合
 * 如果两个集合的元素比较函数不同则使用s1的cmpfunc进行元素比较，并且结果集合也采用s1的cmpfunc为其元素比较函数
 *
 * @param s1, s2
 *	两个用于运算的集合
 *
 * @return
 *	s1和s2的并集的句柄，是一个新建的集合，如果s1和s2中有至少一个无效，则返回-1
 */
extern Container set_union(Container s1, Container s2);

/**
 * @brief 求两个集合的减集，即s1-s2，从s1中删除所有存在于s2中的元素，如果两个集合的元素数据类型不一致则结果集与s1的元素相同
 * 如果两个集合的元素比较函数不同则使用s1的cmpfunc进行元素比较，并且结果集合也采用s1的cmpfunc为其元素比较函数
 *
 * @param s1, s2
 *	两个用于运算的集合
 *
 * @return
 *	集合s1-s2的句柄，是一个新建的集合，如果s1和s2中有至少一个无效，则返回-1
 */
extern Container set_minus(Container s1, Container s2);

#endif
