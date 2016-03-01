/**
 * "mr_linkedlist.h"，链表数据结构处理函数
 *
 * mr_linkedlist库提供一种基于双向链表构造的线性表数据结构，用于连续、依次和有序地存放元素。
 * LinkedList中存放的元素不连续存储，通过双向链接保持固定的存放顺序，删除元素不会释放元素本身的内存空间
 * LinkedList可以保存重复的元素，但不接受NULL
 * LinkedList使用自定义数据类型元素位置(LLPos)来进行随机访问，客户端程序可以通过ll_pos()/ll_head()/ll_tail()函数来获取指定的位置
 * 受链表数据结构本身特征的影响，链表不提供按索引随机访问的操作，而且排序、查找的操作效率比较低，元素数量很大且需要随机访问元素时建议使用ArrayList
 *
 * Version 1.0.0, 李斌，2016/02/29
 */
#ifndef MR_LINKEDLIST_H
#define MR_LINKEDLIST_H

#include "mr_common.h"

/**
 * LinkedList中元素的位置
 */
typedef void *LLPos;

/**
 * 创建一个LinkedList，返回句柄
 * type:	元素的类型
 * cmpfunc:	元素比较函数，传入NULL表示采用mr_common.h中定义的与type对应的默认比较函数
 *
 * 返回:	新创建的LinkedList的句柄(一个大于等于0的正整数)，创建失败返回-1
 */
extern LinkedList ll_create(ElementType type, CmpFunc cmpfunc);

/**
 * 销毁一个LinkedList，释放列表的空间，但不会销毁其中的元素
 * ll:		LinkedList句柄
 *
 * 返回:	销毁完成返回0，销毁失败或无效LinkedList句柄返回-1
 */
extern int ll_destroy(LinkedList ll);

/**
 * 判断一个LinkedList是否为空
 * ll:		LinkedList句柄
 *
 * 返回:	为空返回0，不为空返回1，无效句柄返回-1
 */
extern int ll_isempty(LinkedList ll);

/**
 * 获取一个LinkedList中的元素数量
 * ll:		LinkedList句柄
 *
 * 返回:	元素数量，空列表或无效句柄返回0
 */
extern size_t ll_size(LinkedList ll);

/**
 * 获取表头元素的位置
 * ll:		LinkedList句柄
 *
 * 返回:	表头的元素位置，列表句柄无效或空表时返回NULL
 */
extern LLPos ll_head(LinkedList ll);

/**
 * 获取表尾元素的位置
 * ll:		LinkedList句柄
 *
 * 返回:	表尾的元素位置，列表句柄无效或空表时返回NULL
 */
extern LLPos ll_tail(LinkedList ll);

/**
 * 获取pos位置的下一个元素的位置
 * pos:		当前位置
 *
 * 返回:	下一个元素的位置，列表句柄无效或空表或已经到达表尾时返回NULL
 */
extern LLPos ll_next(LLPos pos);

/**
 * 获取pos位置的前一个元素的位置
 * pos:		当前位置
 *
 * 返回:	前一个元素的位置，列表句柄无效或空表或已经到达表头时返回NULL
 */
extern LLPos ll_prev(LLPos pos);

/**
 * 获取指定位置处的元素
 * pos:		指定的元素位置
 *
 * 返回:	元素，如果位置无效或列表句柄无效则返回NULL
 */
extern Element ll_get(LLPos pos);

/**
 * 在列表指定位置之后插入一个元素
 * ele:		元素，不能为NULL
 * pos:		要插入元素的位置，不能为NULL
 *
 * 返回:	元素插入后所在位置，如果元素或位置为NULL或者ll句柄无效，或发生其他错误导致添加失败返回NULL
 */
extern LLPos ll_insert_after(Element ele, LLPos pos);

/**
 * 在列表指定位置之前插入一个元素
 * ele:		元素，不能为NULL
 * pos:		要插入元素的位置，不能为NULL
 *
 * 返回:	元素插入后所在位置，如果元素或位置为NULL或者ll句柄无效，或发生其他错误导致添加失败返回NULL
 */
extern LLPos ll_insert_before(Element ele, LLPos pos);

/**
 * 在列表最后添加一个元素
 * ll:		LinkedList句柄
 * ele:		元素，不能为NULL
 *
 * 返回:	元素添加后所在位置，如果元素为NULL或者ll句柄无效，或发生其他错误导致添加失败返回NULL
 */
extern LLPos ll_append(LinkedList ll, Element ele);

/**
 * 在列表最前添加一个元素
 * ll:		LinkedList句柄
 * ele:		元素，不能为NULL
 *
 * 返回:	元素添加后所在位置，如果元素为NULL或者ll句柄无效，或发生其他错误导致添加失败返回NULL
 */
extern LLPos ll_prepend(LinkedList ll, Element ele);

/**
 * 删除指定位置的元素，删除成功后pos所指的节点被释放，节点失效
 * pos:		要删除的元素位置，删除后*pos被置为NULL
 *
 * 返回:	删除的元素，pos无效时返回NULL
 */
extern Element ll_remove(LLPos *pos);

/**
 * 在指定位置存储一个元素，覆盖原有的元素，原元素将被返回，其所占的内存空间不会被释放
 * ll:		LinkedList句柄
 * ele:		元素，不能为NULL
 * pos:		要存储元素的位置
 *
 * 返回:	原元素，如果元素为NULL或者ll句柄无效，或位置无效时返回NULL
 */
extern Element ll_replace(LinkedList ll, Element ele, LLPos pos);

/**
 * 从列表中查找一个元素，元素的查找使用列表创建时提供的对象比较函数
 * ll:		LinkedList句柄
 * ele:		要查找的元素
 *
 * 返回:	查找到的时候返回元素位置，有多个相同元素时返回最前面的那个，ll句柄无效或ele为NULL或查找不到返回-1
 */
extern LLPos ll_search(LinkedList ll, Element ele);

/**
 * 清空列表，清空列表并不会释放列表所用的内存空间
 * ll:		LinkedList句柄
 *
 * 返回:	清空成功返回被清空的元素个数，ll句柄无效或清空不成功返回-1
 */
extern int ll_clear(LinkedList ll);

/**
 * 列表元素排序，使用列表创建时提供的对象比较函数进行比较，采用快速排序算法
 * ll:		LinkedList句柄
 *
 */
extern void ll_sort(LinkedList ll);

/**
 * 列表元素排序，使用列表创建时提供的对象比较函数进行比较，采用稳定的插入排序算法，适用于需要稳定排序或数据量较小的场合(建议数据量在5000以内)
 * ll:		LinkedList句柄
 *
 */
extern void ll_stsort(LinkedList ll);

/**
 * 为列表设置或清除元素的数据比较函数
 * ll:		LinkedList句柄
 * cmpfunc:	比较函数，传入NULL表示清除原比较函数改为采用mr_common.h中定义的objcmp()函数
 *
 */
extern void ll_comparator(LinkedList ll, CmpFunc cmpfunc);

#endif
