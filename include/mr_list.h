/**
 * "mr_list.h"，多用途的列表容器，可基于数组或链表两种实现方式
 * 列表容器可以实现按索引随机存取，但基于链表结构的列表在随机存取时效率较低
 * 列表容器也可以使用迭代器顺序访问，且列表容器在迭代时可以删除当前迭代位置的元素，对基于数组和链表两种结构的列表迭代器的效率都很好
 * 列表容器可以进行排序和反向排列操作，对已经排序的列表可以进行二分搜索，但基于链表结构的列表二分搜索的效率较低
 * 列表容器提供堆栈和队列的访问接口，可以把容器直接作为堆栈或队列来使用
 * 基于数组的列表在中间插入或删除元素时效率明显低于基于链表的列表
 * 列表的迭代器同样使用Fast-Fail模式，如果在迭代过程中目标列表被其他线程或其他迭代器修改，则迭代立即终止，再次迭代需要重置迭代器
 * 列表容器可以存入重复元素（元素值相等）和空元素（元素值为NULL或元素长度为0），元素默认比较时，元素值为NULL的空元素会认为比元素值不为NULL但元素长度为0的元素更小
 * 列表容器是强类型容器，在创建时必须指定一种元素类型，表内只能存取相同类型的元素，也可以使用object类型以实现多类型容器
 *
 * 2.0.0, 李斌, 2016/03/25
 */

#ifndef MR_LIST_H
#define MR_LIST_H

#include "mr_common.h"

/**
 * @brief 列表容器的构造类型
 */
typedef enum {
	ArrayList,
	LinkedList
} ListType;

/**
 * @brief 创建一个列表容器
 *
 * @param etype
 *	列表的元素类型
 * @param ltype
 *	列表的构造类型，LinkedList或ArrayList
 * @param cmpfunc
 *	列表元素的比较函数，NULL表示使用元素类型对应的默认比较函数
 *
 * @return 
 *	创建成功返回列表容器，失败返回NULL
 */
extern Container list_create(ElementType etype, ListType ltype, CmpFunc cmpfunc);

/**
 * @brief 销毁一个列表容器，同时销毁其中所有元素
 *
 * @param list
 *	待销毁的列表容器
 *
 * @return 
 *	销毁成功返回0，失败返回-1
 */
extern int list_destroy(Container list);

/**
 * @brief 设置列表容器的比较函数
 *
 * @param list
 * 	列表容器
 * @param cmpfunc
 * 	比较函数，NULL则根据容器的元素数据类型使用系统默认的比较函数
 */
extern void list_set_cmpfunc(Container list, CmpFunc cmpfunc);

/**
 * @brief 判断一个列表容器是否为空
 *
 * @param list
 * 	列表容器
 *
 * @return 
 * 	容器为空返回1，容器不空返回0，无效容器返回1
 */
extern int list_isempty(Container list);

/**
 * @brief 获取一个列表容器中的元素数量
 *
 * @param list
 * 	列表容器
 *
 * @return 
 * 	返回容器中的元素数量，无效容器返回0
 */
extern size_t list_size(Container list);

/**
 * @brief 在列表最后添加一个新元素，本函数调用list_insert()函数实现添加
 *
 * @param list
 * 	列表容器
 * @param element
 * 	待添加元素的值
 * @param type
 * 	待添加元素的类型
 * @param len
 * 	待添加元素的字节长度
 *
 * @return 
 * 	添加成功返回0，失败返回-1
 */
extern int list_append(Container list, Element element, ElementType type, size_t len);

/**
 * @brief 在列表指定位置插入一个元素
 *
 * @param list
 * 	列表容器
 * @param index
 * 	插入的位置，超过现有列表尾部的在尾部插入
 * @param element
 * 	待插入元素的值
 * @param type
 * 	待插入元素的类型
 * @param len
 * 	待插入元素的字节长度
 *
 * @return 
 * 	插入成功返回0，失败返回-1
 */
extern int list_insert(Container list, size_t index, Element element, ElementType type, size_t len);

/**
 * @brief 获取列表容器中第index个位置的元素，index从0开始计数
 *
 * @param list
 * 	列表容器
 * @param index
 * 	元素所在位置，从0开始计数
 *
 * @return 
 * 	第index个元素的值，index超过范围时返回NULL
 */
extern Element list_get(Container list, size_t index);

/**
 * @brief 删除列表容器中第index个位置的元素，index从0开始计数
 *
 * @param list
 * 	列表容器
 * @param index
 * 	元素所在位置，从0开始计数
 *
 * @return 
 * 	实际删除的元素数量，0或1
 */
extern size_t list_remove_at(Container list, size_t index);

/**
 * @brief 删除列表容器中所有与ele相等的元素
 *
 * @param list
 * 	列表容器
 * @param element
 * 	元素的值
 * @param type
 * 	元素的类型
 * @param len
 * 	元素的长度
 *
 * @return 
 * 	实际删除的元素数量
 */
extern size_t list_remove(Container list, Element element, ElementType type, size_t len);

/**
 * @brief 清空列表容器中所有元素
 *
 * @param list
 * 	列表容器
 */
extern void list_removeall(Container list);

/**
 * @brief 从第from个元素开始搜索元素
 *
 * @param list
 * 	列表容器
 * @param from
 * 	搜索开始的位置，负数表示从头或从尾开始，超限的from值修正为size - 1
 * @param dir
 * 	搜索的方向，Forward或Reverse
 * @param element
 * 	搜索的元素值
 * @param type
 * 	搜索的元素类型
 * @param len
 * 	搜索的元素长度
 *
 * @return 
 * 	找到相同元素返回位置，找不到返回-1
 */
extern int list_search(Container list, int from, int dir, Element element, ElementType type, size_t len);

/**
 * @brief 对列表进行二分搜索，注意：二分搜索是一种快速但不确定的搜索算法，当列表中有重复元素时，对重复元素的搜索不能确保返回的结果是确定的
 * 另外，如果列表处于无序状态，或排序和二分搜索用了不同的比较函数，可能导致无法进行正确的搜索
 *
 * @param list
 * 	列表容器
 * @param element
 * 	搜索的元素值
 * @param type
 * 	搜索的元素类型
 * @param len
 * 	搜索的元素长度
 *
 * @return 
 * 	找到相同元素则返回位置，找不到返回-1
 */
extern int list_bi_search(Container list, Element element, ElementType type, size_t len);

/**
 * @brief 对列表进行快速排序，注意：快速排序是一种速度快但不稳定的排序算法
 *
 * @param list
 * 	列表容器
 * @param order
 * 	排序顺序，使用预定义常量Asc或Desc表示升序和降序
 */
extern void list_qsort(Container list, int order);

/**
 * @brief 对列表进行插入排序，注意：插入排序是稳定的排序算法但元素数量很大时速度较慢
 *
 * @param list
 * 	列表容器
 * @param order
 * 	排序顺序，使用预定义常量Asc或Desc表示升序和降序
 */
extern void list_isort(Container list, int order);

/**
 * @brief 对列表容器中的元素进行顺序反转
 *
 * @param list
 * 	列表容器
 */
extern void list_reverse(Container list);

/**
 * @brief 用列表实现堆栈压栈操作，本函数调用list_insert()函数实现
 *
 * @param list
 * 	列表容器
 * @param element
 * 	待压栈元素的值
 * @param type
 * 	待压栈元素的类型
 * @param len
 * 	待压栈元素的字节长度
 *
 * @return 
 * 	压栈成功返回0，失败返回-1
 */
extern int list_push(Container list, Element element, ElementType type, size_t len);

/**
 * @brief 用列表实现堆栈弹栈操作
 *
 * @param list
 * 	列表容器
 *
 * @return 
 * 	栈顶元素，空栈时返回NULL
 */
extern Element list_pop(Container list);

/**
 * @brief 用列表实现堆栈查看栈顶元素的操作
 *
 * @param list
 * 	列表容器
 *
 * @return 
 * 	栈顶元素，空栈时返回NULL
 */
extern Element list_stacktop(Container list);

/**
 * @brief 用列表实现队列入队操作，本函数调用list_insert()函数实现
 *
 * @param list
 * 	列表容器
 * @param element
 * 	待入队元素的值
 * @param type
 * 	待入队元素的类型
 * @param len
 * 	待入队元素的字节长度
 *
 * @return 
 * 	入队成功返回0，失败返回-1
 */
extern int list_enqueue(Container list, Element element, ElementType type, size_t len);

/**
 * @brief 用列表实现队列出队操作
 *
 * @param list
 * 	列表容器
 *
 * @return 
 * 	队首元素，空队时返回NULL
 */
extern Element list_dequeue(Container list);

/**
 * @brief 用列表实现队列查看队首元素的操作
 *
 * @param list
 * 	列表容器
 *
 * @return 
 * 	队首元素，空队时返回NULL
 */
extern Element list_queuehead(Container list);

/**
 * @brief 获取列表容器的迭代器
 *
 * @param list
 * 	列表容器
 * @param dir
 * 	迭代方向，Forward或Reverse
 *
 * @return 
 * 	列表的迭代器，列表无效或空表时返回NULL，创建失败返回NULL
 */
extern Iterator list_iterator(Container list, int dir);

#endif
