#include <stdlib.h>
#include <pthread.h>

#include "mr_containers.h"
#include "mr_set.h"

/**
 * 集合节点结构
 */
typedef struct SetNode {
	Element element;		// 元素
	struct SetNode *left;		// 左子树
	struct SetNode *right;		// 右子树
	int balance;			// 平衡因子
} set_node_t, *set_node_p;

/**
 * 集合结构
 */
typedef struct {
	ElementType type;		// 元素的数据类型
	set_node_p root;		// 根节点
	size_t size;			// 节点数量
	CmpFunc cmpfunc;		// 元素比较函数
	pthread_mutex_t mut;		// 共享锁
} set_t, *set_p;

/**
 * 集合迭代器，基于Morris算法进行迭代
 */
typedef struct {
	int asc;			// 迭代方向，1=递增顺序，0=递减逆序
	Set set;			// 集合句柄
	set_node_p cur;			// Morris算法，当前节点
	set_node_p pre;			// Morris算法，前驱节点
} set_it_t, *set_it_p;

Set set_create(ElementType type, CmpFunc cmpfunc);
int set_destroy(Set s);
int set_isempty(Set s);
size_t set_size(Set s);
int set_add(Set s, Element ele);
int set_remove(Set s, Element ele);
void set_removeall(Set s, onRemove onremove);
SetIterator set_iterator(Set s);
SetIterator set_riterator(Set s);
Element set_next(SetIterator it);

/**
 * 创建一个Set，返回句柄
 * type:	元素的类型
 * cmpfunc:	元素比较函数，传入NULL表示采用mr_common.h中定义的与type对应的默认比较函数
 *
 * 返回:	新创建的Set的句柄(一个大于等于0的正整数)，创建失败返回-1
 */
Set set_create(ElementType type, CmpFunc cmpfunc)
{
	Set ret = -1;
	return ret;
}

/**
 * 销毁一个Set，释放列表的空间，但不会销毁其中的元素
 * s:		Set句柄
 *
 * 返回:	销毁完成返回0，销毁失败或无效Set句柄返回-1
 */
int set_destroy(Set s)
{
	int ret = -1;
	return ret;
}

/**
 * 判断一个Set是否为空
 * s:		Set句柄
 *
 * 返回:	为空返回1，不为空返回0，无效句柄返回-1
 */
int set_isempty(Set s)
{
	int ret = 1;
	return ret;
}

/**
 * 获取一个Set中的元素数量
 * s:		Set句柄
 *
 * 返回:	元素数量，空集合或无效句柄返回0
 */
size_t set_size(Set s)
{
	size_t ret = 0;
	return ret;
}

/**
 * 添加一个元素，重复元素将不予添加
 * s:		Set句柄
 * ele:		待添加的元素
 *
 * 返回:	添加成功返回0，添加失败或元素重复返回-1
 */
int set_add(Set s, Element ele)
{
	int ret = -1;
	return ret;
}

/**
 * 删除一个元素
 * s:		Set句柄
 * ele:		待删除的元素
 *
 * 返回:	删除成功返回0，删除失败或元素未找到返回-1
 */
int set_remove(Set s, Element ele)
{
	int ret = -1;
	return ret;
}

/**
 * 删除Set中所有的元素，被清除的元素用onremove函数进行后续处理
 * s:		Set句柄
 * onremove:	元素后续处理函数，NULL表示不做任何处理，典型的可以传入标准库函数free
 *
 */
void set_removeall(Set s, onRemove onremove)
{
}

/**
 * 获取一个集合的递增顺序迭代器
 * s:		Set句柄
 * 
 * 返回:	集合迭代器，Set为空返回NULL
 */
SetIterator set_iterator(Set s)
{
	set_it_p ret = NULL;
	return (SetIterator)ret;
}

/**
 * 获取一个集合的递减反序迭代器
 * s:		Set句柄
 * 
 * 返回:	集合迭代器，Set为空返回NULL
 */
SetIterator set_riterator(Set s)
{
	set_it_p ret = NULL;
	return (SetIterator)ret;
}

/**
 * 迭代访问一个迭代器中的下一个元素
 * it:		集合迭代器
 *
 * 返回:	迭代中的下一个元素，迭代器无效时返回NULL，已经迭代完成时返回NULL并销毁迭代器，置it为NULL
 */
Element set_next(SetIterator it)
{
	Element ret = NULL;
	return ret;
}

