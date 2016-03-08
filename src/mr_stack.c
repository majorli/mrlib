#include <stdlib.h>
#include <pthread.h>

#include "container.h"
#include "mr_stack.h"

Stack st_create(ElementType type);
int st_destroy(Stack st);
int st_isempty(Stack st);
size_t st_size(Stack st);
int st_push(Stack st, Element ele);
Element st_peak(Stack st);
Element st_pop(Stack st);

/**
 * 创建一个Stack，返回句柄
 * type:	元素的类型
 *
 * 返回:	新创建的Stack的句柄(一个大于等于0的正整数)，创建失败返回-1
 */
Stack st_create(ElementType type)
{
	Stack ret = -1;
	return ret;
}

/**
 * 销毁一个Stack，释放列表的空间，但不会销毁其中的元素
 * st:		Stack句柄
 *
 * 返回:	销毁完成返回0，销毁失败或无效Stack句柄返回-1
 */
int st_destroy(Stack st)
{
	int ret = -1;
	return ret;
}

/**
 * 判断一个Stack是否为空
 * st:		Stack句柄
 *
 * 返回:	为空返回1，不为空返回0，无效句柄返回-1
 */
int st_isempty(Stack st)
{
	int ret = 1;
	return ret;
}

/**
 * 获取一个Stack中的元素数量
 * st:		Stack句柄
 *
 * 返回:	元素数量，空栈或无效句柄返回0
 */
size_t st_size(Stack st)
{
	size_t ret = 0;
	return ret;
}

/**
 * 新元素压入堆栈
 * st:		Stack句柄
 * ele:		元素，不能为NULL
 *
 * 返回:	压栈成功返回0，压栈失败或st无效时返回-1
 */
int st_push(Stack st, Element ele)
{
	int ret = -1;
	return ret;
}

/**
 * 读取当前栈顶元素但不弹出
 * st:		Stack句柄
 * 
 * 返回:	当前栈顶元素，空栈或st无效时返回NULL
 */
Element st_peak(Stack st)
{
	Element ret = NULL;
	return ret;
}

/**
 * 弹出栈顶元素
 * st:		Stack句柄
 *
 * 返回:	当前栈顶元素，空栈或st无效时返回NULL
 */
Element st_pop(Stack st)
{
	Element ret = NULL;
	return ret;
}

