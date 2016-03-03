#include <stdlib.h>
#include <pthread.h>

#include "mr_containers.h"
#include "mr_set.h"

/**
 * 集合节点结构
 */
typedef struct SetNode {
	Element element;		// 元素
	struct SetNode *left;		// 左子树根节点
	struct SetNode *right;		// 右子树根节点
	struct SetNode *parent;		// 父节点
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
 * 集合迭代器
 */
typedef struct {
	Set set;			// 迭代的集合，用于加访问锁
	int asc;			// 迭代方向，1=递增顺序，0=递减逆序
	set_node_p *stack;		// 迭代用的堆栈
	set_node_p *top;		// 栈顶指针
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
Element set_next(SetIterator *it);

static void __postorder_remove(set_node_p root, onRemove onremove);	// 后序遍历删除所有节点

static void __it_push(set_it_p it, set_node_p node);			// 迭代用的压栈函数
static set_node_p __it_pop(set_it_p it);				// 迭代用的弹栈函数
static int __it_stack_empty(set_it_p it);				// 迭代用的空栈判断函数

static set_it_p __iterator(set_p s, int asc);				// 生成一个迭代器
static set_node_p __it_next(set_it_p it);				// 用Mirros算法中序迭代一个迭代器

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
	set_p set = (set_p)malloc(sizeof(set_t));
	set->type = type;
	set->root = NULL;
	set->size = 0;
	if (cmpfunc == NULL)
		set->cmpfunc = default_cmpfunc(type);
	else
		set->cmpfunc = cmpfunc;
	ret = container_retrieve(set, Set_t);
	if (ret == -1) {
		free(set);
	} else {
		if (__MultiThreads__ == 1) {
			pthread_mutex_init(&(set->mut), NULL);
		}
	}
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
	set_p set = (set_p)container_release(s, Set_t);
	if (set != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(set->mut));
		set_node_p p = set->root;
		/* 删除所有节点，用后序遍历逐个释放每一个节点，但不释放其中的元素 */
		__postorder_remove(p, NULL);
		if (__MultiThreads__ == 1) {
			pthread_mutex_unlock(&(set->mut));
			pthread_mutex_destroy(&(set->mut));
		}
		free(set);
		ret = 0;
	}
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
	set_p set = (set_p)container_get(s, Set_t);
	if (set != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(set->mut));
		ret = (set->size == 0);
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(set->mut));
	}
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
	set_p set = (set_p)container_get(s, Set_t);
	if (set != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(set->mut));
		ret = set->size;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(set->mut));
	}
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
	set_p set = (set_p)container_get(s, Set_t);
	if (set != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(set->mut));
		set_node_p p = set->root;
		/* 删除所有节点，用后序遍历逐个释放每一个节点，用onremove参数进行元素的后续处理 */
		__postorder_remove(p, onremove);
		set->size = 0;
		set->root = NULL;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(set->mut));
	}
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
	set_p set = (set_p)container_get(s, Set_t);
	if (set != NULL && set->root != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(set->mut));
		ret = __iterator(set, 1);
		ret->set = s;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(set->mut));
	}
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
	set_p set = (set_p)container_get(s, Set_t);
	if (set != NULL && set->root != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(set->mut));
		ret = __iterator(set, 0);
		ret->set = s;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(set->mut));
	}
	return (SetIterator)ret;
}

/**
 * 迭代访问一个迭代器中的下一个元素
 * it:		集合迭代器的指针
 *
 * 返回:	迭代中的下一个元素，迭代器无效或已经迭代完成时返回NULL并销毁迭代器，设置迭代器指针为NULL
 */
Element set_next(SetIterator *it)
{
	Element ret = NULL;
	set_it_p iterator = (set_it_p)(*it);
	set_p set = (set_p)container_get(iterator->set, Set_t);
	if (set != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(set->mut));
		set_node_p next = __it_next(iterator);
		ret = (next == NULL ? NULL : next->element);
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(set->mut));
	}
	if (ret == NULL) {
		free(iterator->stack);
		free(iterator);
		*it = NULL;
	}
	return ret;
}

static void __postorder_remove(set_node_p root, onRemove onremove)		// 后序遍历删除所有节点
{
	if (root != NULL) {
		__postorder_remove(root->left, onremove);
		__postorder_remove(root->right, onremove);
		if (onremove != NULL)
			onremove(root->element);
		free(root);
	}
}

static void __it_push(set_it_p it, set_node_p node)			// 迭代用的压栈函数
{
	*(it->top++) = node;
}

static set_node_p __it_pop(set_it_p it)					// 迭代用的弹栈函数
{
	return *(--it->top);
}

static int __it_stack_empty(set_it_p it)				// 迭代用的空栈判断函数
{
	return it->stack == it->top;
}

static set_it_p __iterator(set_p set, int asc)				// 生成一个迭代器
{
	set_it_p ret = (set_it_p)malloc(sizeof(set_it_t));
	ret->asc = asc;
	ret->stack = (set_node_p *)malloc(set->size * sizeof(set_node_p));
	ret->top = ret->stack;
	set_node_p current = set->root;
	while (current != NULL) {
		__it_push(ret, current);
		current = ret->asc ? current->left : current->right;
	}
	return ret;
}

static set_node_p __it_next(set_it_p it) {				// 中序迭代一个迭代器
	set_node_p ret = NULL;
	if (!__it_stack_empty(it)) {
		ret = __it_pop(it);
		if (it->asc ? ret->right != NULL : ret->left != NULL) {
			set_node_p current = it->asc ? ret->right : ret->left;
			while (current != NULL) {
				__it_push(it, current);
				current = it->asc ? current->left : current->right;
			}
		}
	}
	return ret;
}
