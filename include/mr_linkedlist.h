/**
 * "mr_linkedlist.h"，链表数据结构处理函数
 *
 * mr_linkedlist库提供一种基于双向链表构造的线性表数据结构，用于连续、依次和有序地存放元素，元素被包装在节点LLNode中
 * LinkedList中的节点不连续存储，通过双向链接保持固定的存放顺序，删除节点不会释放元素本身的内存空间
 * LinkedList可以保存重复的元素，但不接受NULL
 * LinkedList使用自定义数据类型节点(LLNode)来进行随机访问，客户端程序可以通过ll_head()/ll_tail()/ll_next()/ll_prev()函数来获取指定的节点，通过ll_get()函数获得节点中的元素
 * 受链表数据结构本身特征的影响，链表不提供按索引随机访问的操作，查找的效率比较低，元素数量很大且需要随机访问元素时建议使用ArrayList
 *
 * Version 1.0.1, 李斌，2016/03/02
 */
#ifndef MR_LINKEDLIST_H
#define MR_LINKEDLIST_H

#include "mr_common.h"

/**
 * LinkedList中元素的位置
 */
typedef void *LLNode;

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
extern LLNode ll_head(LinkedList ll);

/**
 * 获取表尾元素的位置
 * ll:		LinkedList句柄
 *
 * 返回:	表尾的元素位置，列表句柄无效或空表时返回NULL
 */
extern LLNode ll_tail(LinkedList ll);

/**
 * 获取pos位置的下一个元素的位置
 * pos:		当前位置
 *
 * 返回:	下一个元素的位置，列表句柄无效或空表或已经到达表尾时返回NULL
 */
extern LLNode ll_next(LLNode pos);

/**
 * 获取pos位置的前一个元素的位置
 * pos:		当前位置
 *
 * 返回:	前一个元素的位置，列表句柄无效或空表或已经到达表头时返回NULL
 */
extern LLNode ll_prev(LLNode pos);

/**
 * 获取指定位置处的元素
 * pos:		指定的元素位置
 *
 * 返回:	元素，如果位置无效或列表句柄无效则返回NULL
 */
extern Element ll_get(LLNode pos);

/**
 * 在列表指定位置之后插入一个元素
 * pos:		要插入元素的位置，不能为NULL
 * ele:		元素，不能为NULL
 *
 * 返回:	元素插入后所在位置，如果元素或位置为NULL或者ll句柄无效，或发生其他错误导致添加失败返回NULL
 */
extern LLNode ll_insert_after(LLNode pos, Element ele);

/**
 * 在列表指定位置之前插入一个元素
 * pos:		要插入元素的位置，不能为NULL
 * ele:		元素，不能为NULL
 *
 * 返回:	元素插入后所在位置，如果元素或位置为NULL或者ll句柄无效，或发生其他错误导致添加失败返回NULL
 */
extern LLNode ll_insert_before(LLNode pos, Element ele);

/**
 * 在列表最后添加一个元素
 * ll:		LinkedList句柄
 * ele:		元素，不能为NULL
 *
 * 返回:	元素添加后所在位置，如果元素为NULL或者ll句柄无效，或发生其他错误导致添加失败返回NULL
 */
extern LLNode ll_append(LinkedList ll, Element ele);

/**
 * 在列表最前添加一个元素
 * ll:		LinkedList句柄
 * ele:		元素，不能为NULL
 *
 * 返回:	元素添加后所在位置，如果元素为NULL或者ll句柄无效，或发生其他错误导致添加失败返回NULL
 */
extern LLNode ll_prepend(LinkedList ll, Element ele);

/**
 * 删除指定位置的元素，删除成功后pos所指的节点被释放，节点失效
 * pos:		要删除的节点，删除后*pos被置为NULL
 *
 * 返回:	删除的元素，pos无效时返回NULL
 */
extern Element ll_remove(LLNode *pos);

/**
 * 删除LinkedList中所有的元素，被清除的元素用onremove函数进行后续处理
 * ll:		LinkedList句柄
 * onremove:	元素后续处理函数，NULL表示不做任何处理，典型的可以传入标准库函数free
 *
 */
extern void ll_removeall(LinkedList ll, onRemove onremove);

/**
 * 在指定位置存储一个元素，覆盖原有的元素，原元素将被返回，其所占的内存空间不会被释放
 * pos:		要存储元素的节点
 * ele:		元素，不能为NULL
 *
 * 返回:	原元素，如果元素为NULL或者节点无效时返回NULL
 */
extern Element ll_replace(LLNode pos, Element ele);

/**
 * 从列表中查找一个元素，元素的查找使用列表创建时提供的对象比较函数
 * ll:		LinkedList句柄
 * ele:		要查找的元素
 *
 * 返回:	查找到的时候返回节点，有多个相同元素时返回最前面的那个，ll句柄无效或ele为NULL或查找不到返回-1
 */
extern LLNode ll_search(LinkedList ll, Element ele);

/**
 * 从列表中反向一个元素，元素的查找使用列表创建时提供的对象比较函数
 * ll:		LinkedList句柄
 * ele:		要查找的元素
 *
 * 返回:	查找到的时候返回节点，有多个相同元素时返回最后面的那个，ll句柄无效或ele为NULL或查找不到返回-1
 */
extern LLNode ll_rsearch(LinkedList ll, Element ele);

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
