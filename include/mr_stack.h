/**
 * "mr_stack.h"，堆栈数据结构处理函数
 *
 * mr_stack库提供一种基于数组构造的线性堆栈，用于实现先进后出的存取元素。
 * 压入新元素时如超出当前堆栈容量，Stack会自动扩展容量，但弹出栈顶元素时不会自动释放空余的空间
 * Stack可以保存重复的元素，但不接受NULL
 *
 * Version 1.0.0, 李斌，2016/03/03
 */
#ifndef MR_STACK_H
#define MR_STACK_H

#include "mr_common.h"

/**
 * 创建一个Stack，返回句柄
 * type:	元素的类型
 * cmpfunc:	元素比较函数，传入NULL表示采用mr_common.h中定义的与type对应的默认比较函数
 *
 * 返回:	新创建的Stack的句柄(一个大于等于0的正整数)，创建失败返回-1
 */
extern Stack st_create(ElementType type, CmpFunc cmpfunc);

/**
 * 销毁一个Stack，释放列表的空间，但不会销毁其中的元素
 * st:		Stack句柄
 *
 * 返回:	销毁完成返回0，销毁失败或无效Stack句柄返回-1
 */
extern int st_destroy(Stack st);

/**
 * 判断一个Stack是否为空
 * st:		Stack句柄
 *
 * 返回:	为空返回1，不为空返回0，无效句柄返回-1
 */
extern int st_isempty(Stack st);

/**
 * 获取一个Stack中的元素数量
 * st:		Stack句柄
 *
 * 返回:	元素数量，空栈或无效句柄返回0
 */
extern size_t st_size(Stack st);

/**
 * 新元素压入堆栈
 * st:		Stack句柄
 * ele:		元素，不能为NULL
 *
 * 返回:	压栈成功返回0，压栈失败或st无效时返回-1
 */
extern int st_push(Stack st, Element ele);

/**
 * 读取当前栈顶元素但不弹出
 * st:		Stack句柄
 * 
 * 返回:	当前栈顶元素，空栈或st无效时返回NULL
 */
extern Element st_peak(Stack st);

/**
 * 弹出栈顶元素
 * st:		Stack句柄
 *
 * 返回:	当前栈顶元素，空栈或st无效时返回NULL
 */
extern Element st_pop(Stack st);

#endif
