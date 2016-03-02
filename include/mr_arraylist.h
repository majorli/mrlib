/**
 * "mr_arraylist.h"，线性表数据结构处理函数
 *
 * mr_arraylist库提供一种基于数组构造的线性表数据结构，用于连续、依次和有序地存放元素。
 * ArrayList中存放的元素连续存储，并保持固定的存放顺序，删除中间的元素时会自动移动后续元素以使存储空间始终保持连续。
 * 当元素超过当前表的容量时，ArrayList会自动扩展容量，但删除元素时不会自动释放空余的表空间
 * ArrayList可以保存重复的元素，但不接受NULL
 *
 * Version 1.0.0, 李斌，2016/02/29
 */
#ifndef MR_ARRAYLIST_H
#define MR_ARRAYLIST_H

#include "mr_common.h"

/**
 * 创建一个ArrayList，返回句柄
 * type:	元素的类型
 * cmpfunc:	元素比较函数，传入NULL表示采用mr_common.h中定义的与type对应的默认比较函数
 *
 * 返回:	新创建的ArrayList的句柄(一个大于等于0的正整数)，创建失败返回-1
 */
extern ArrayList al_create(ElementType type, CmpFunc cmpfunc);

/**
 * 销毁一个ArrayList，释放列表的空间，但不会销毁其中的元素
 * al:		ArrayList句柄
 *
 * 返回:	销毁完成返回0，销毁失败或无效ArrayList句柄返回-1
 */
extern int al_destroy(ArrayList al);

/**
 * 判断一个ArrayList是否为空
 * al:		ArrayList句柄
 *
 * 返回:	为空返回0，不为空返回1，无效句柄返回-1
 */
extern int al_isempty(ArrayList al);

/**
 * 获取一个ArrayList中的元素数量
 * al:		ArrayList句柄
 *
 * 返回:	元素数量，空列表或无效句柄返回0
 */
extern size_t al_size(ArrayList al);

/**
 * 获取指定位置的元素
 * al:		ArrayList句柄
 * index:	元素位置，从0开始计数到size - 1
 *
 * 返回:	index位置的元素指针，超出0 <= index < size的范围时返回NULL，无效ArrayList句柄返回NULL
 */
extern Element al_get(ArrayList al, size_t index);

/**
 * 在列表最后添加一个元素
 * al:		ArrayList句柄
 * ele:		元素，不能为NULL
 *
 * 返回:	元素添加后所在位置，如果元素为NULL或者al句柄无效，或发生其他错误导致添加失败返回-1
 */
extern int al_append(ArrayList al, Element ele);

/**
 * 在列表指定位置添加一个元素
 * al:		ArrayList句柄
 * ele:		元素，不能为NULL
 * index:	要插入元素的位置，原本该位置开始直至最后的元素将被向后移动一个位置
 *
 * 返回:	元素插入后所在位置，如果元素为NULL或者al句柄无效，或发生其他错误导致添加失败返回-1
 */
extern int al_add(ArrayList al, Element ele, size_t index);

/**
 * 删除指定位置的元素，元素将被返回，其所占的内存空间不会被释放
 * al:		ArrayList句柄
 * index:	要删除的元素位置
 *
 * 返回:	删除的元素，al句柄无效或index超范围时返回NULL
 */
extern Element al_remove(ArrayList al, size_t index);

/**
 * 删除ArrayList中所有的元素，被清除的元素用onremove函数进行后续处理
 * al:		ArrayList句柄
 * onremove:	元素后续处理函数，NULL表示不做任何处理，典型的可以传入标准库函数free
 *
 */
extern void al_removeall(ArrayList al, onRemove onremove);

/**
 * 在列表指定位置存储一个元素，覆盖原有的元素，原元素将被返回，其所占的内存空间不会被释放
 * al:		ArrayList句柄
 * ele:		元素，不能为NULL
 * index:	要存储元素的位置，必须在0 <= index < size的范围内
 *
 * 返回:	原元素，如果元素为NULL或者al句柄无效，或index超出范围时返回NULL
 */
extern Element al_replace(ArrayList al, Element ele, size_t index);

/**
 * 从列表中查找一个元素，元素的查找使用列表创建时提供的对象比较函数
 * al:		ArrayList句柄
 * ele:		要查找的元素
 *
 * 返回:	查找到的时候返回元素位置，有多个相同元素时返回最前面的那个，al句柄无效或ele为NULL或查找不到返回-1
 */
extern int al_search(ArrayList al, Element ele);

/**
 * 从列表中逆向查找一个元素，元素的查找使用列表创建时提供的对象比较函数
 * al:		ArrayList句柄
 * ele:		要查找的元素
 *
 * 返回:	查找到的时候返回元素位置，有多个相同元素时返回最后面的那个，al句柄无效或ele为NULL或查找不到返回-1
 */
extern int al_rsearch(ArrayList al, Element ele);

/**
 * 列表元素排序，使用列表创建时提供的对象比较函数进行比较，采用快速排序算法
 * al:		ArrayList句柄
 *
 */
extern void al_sort(ArrayList al);

/**
 * 列表元素排序，使用列表创建时提供的对象比较函数进行比较，采用稳定的插入排序算法，适用于需要稳定排序或数据量较小的场合(建议数据量在5000以内)
 * al:		ArrayList句柄
 *
 */
extern void al_stsort(ArrayList al);

/**
 * 为列表设置或清除元素的数据比较函数
 * al:		ArrayList句柄
 * cmpfunc:	比较函数，传入NULL表示清除原比较函数改为采用mr_common.h中定义的objcmp()函数
 *
 */
extern void al_comparator(ArrayList al, CmpFunc cmpfunc);

#endif
