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
Element set_search(Set s, Element ele);
int set_add(Set s, Element ele);
Element set_remove(Set s, Element ele);
void set_removeall(Set s, onRemove onremove);
SetIterator set_iterator(Set s);
SetIterator set_riterator(Set s);
Element set_next(SetIterator *it);
Set set_intersection(Set s1, Set s2);
Set set_union(Set s1, Set s2);
Set set_minus(Set s1, Set s2);

static void __postorder_remove(set_node_p root, onRemove onremove);	// 后序遍历删除所有节点

static void __it_push(set_it_p it, set_node_p node);			// 迭代用的压栈函数
static set_node_p __it_pop(set_it_p it);				// 迭代用的弹栈函数
static int __it_stack_empty(set_it_p it);				// 迭代用的空栈判断函数

static set_it_p __iterator(set_p s, int asc);				// 生成一个迭代器
static set_node_p __it_next(set_it_p it);				// 用Mirros算法中序迭代一个迭代器

static set_node_p __set_get(set_p set, Element ele);			// 在集合set中搜索与ele相同的元素，返回其节点，搜索不到返回NULL

static void __set_fix_balance(set_node_p from, set_node_p to);		// 从from到to自下而上的修正平衡因子
static void __set_r_left(set_p set, set_node_p node);			// 以node节点为轴左旋
static void __set_r_right(set_p set, set_node_p node);			// 以node节点为轴右旋

static set_node_p __set_successor(set_node_p p);			// 返回已知节点的后继节点，即按递增顺序排列的后一个节点，当前节点是树中最大值时返回NULL
static set_node_p __set_min(set_node_p root);				// 返回树中最小的节点
static set_node_p __set_max(set_node_p root);				// 返回树中最大的节点
static set_node_p __set_first_ubparent(set_node_p node);		// 从当前节点开始向上调整父节点的平衡因子直到第一个平衡因子非0的父节点，并返回该父节点
static void __set_after_remove(set_p set, set_node_p del);		// 调整删除节点后的AVL树结构并修正平衡因子

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
		ret = (set->root == NULL);
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
 * 在集合中搜索一个元素
 * s:		Set句柄
 * ele:		要搜索的元素
 *
 * 返回:	搜索到集合中存在与ele相同的元素时返回集合中的元素，搜索不到或搜索出错返回NULL
 */
Element set_search(Set s, Element ele)
{
	Element ret = NULL;
	set_p set = (set_p)container_get(s, Set_t);
	if (ele != NULL && set != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(set->mut));
		set_node_p node = __set_get(set, ele);
		ret = (node == NULL ? NULL : node->element);
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
	set_p set = (set_p)container_get(s, Set_t);
	if (ele != NULL && set != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(set->mut));
		int exists = 0;				// 元素是否已经存在的标识
		set_node_p fbn = NULL;			// firstBalanceNode
		unsigned int path = 1;			// 记录从fbn开始往下的路径，以1为开始标记，每一位的0表示向左1表示向右
		int k = 0;				// 开始位置
		set_node_p p = set->root;		// 当前搜索点
		set_node_p parent = NULL;		// 搜索点的父节点
		int cmp = 0;
		while (p != NULL) {
			if (p->balance != 0) {
				fbn = p;
				path = 1;
				k = 0;
			}
			parent = p;
			cmp = set->cmpfunc(ele, p->element);
			if (cmp < 0) {			// ele < p->element，向左走
				k++;
				p = p->left;
				path = (path << 1);	// 向左走就是在path后面加一个0
			} else if (cmp > 0) {		// ele > p->element，向右走
				k++;
				p = p->right;
				path = (path << 1);
				path++;			// 向右走就是在path后面加一个1
			} else {			// ele == p->element，已经有这个元素，不做任何操作直接退出
				exists = 1;
				break;
			}
		}
		if (!exists) {				// 元素不存在的情况下进行添加，否则直接退出并返回-1
			set_node_p nnode = (set_node_p)malloc(sizeof(set_node_t));	// 新节点
			nnode->element = ele;
			nnode->left = NULL;
			nnode->right = NULL;
			nnode->parent = parent;			// 如果root为NULL，那么上一个搜索循环必定直接结束，可以断言此时parent也必定为NULL
			nnode->balance = 0;
			if (set->root == NULL)
				set->root = nnode;
			else					// 如果root不为NULL，cmp至少经历过一次比较，因为元素重复的情况在这里已经被排除，所以最后一次必定是ele和parent的比较
				if (cmp < 0)			// 比父节点小，插入为左子节点
					parent->left = nnode;
				else				// 否则肯定比父节点大，因为相同的情况已经被排除，插入为右子节点
					parent->right = nnode;
			/* 
			 * 开始进行AVL树维护，首先要判断是否需要旋转，有三种情况不需要旋转：
			 *	1. 没有找到非平衡点，判断依据为fbn == NULL，记为C1
			 *	2. 存在非平衡点，但因为新节点的加入变成了平衡点，分为以下两种情况:
			 *		2.1 最后一个非平衡点向右倾斜，而新节点在它的左子树，判断依据为fbn->balance == -1 && ((path >> (k - 1)) & 1) == 0，记为C2
			 *		2.2 最后一个非平衡点向左倾斜，而新节点在它的右子树，判断依据为fbn->balance == 1 && ((path >> (k - 1)) & 1) == 1，记为C3
			 * 由此可以得到需要进行旋转的条件是:
			 *	A == !(C1 || C2 || C3)
			 *	判断式为：fbn != NULL && !(fbn->balance == -1 && ((path >> (k - 1)) & 1) == 0) && !(fbn->balance == 1 && ((path >> (k - 1)) & 1) == 1)
			 *	又：bfn != NULL时，可以断言：fbn->balance != 0，即不是1就是-1，且k >= 1，即可以计算走向值(path >> (k - 1)) & 1，该走向值非0即1
			 *	所以可以断言该判断式有效，然后在!C1的前提下化简为更易理解的表达式
			 *	定义D1 := (fbn->balance == -1), D2 := (fbn->balance == 1)，可知(D1 || D2) == TRUE
			 *	定义变量dir = (path >> (k - 1)) & 1，可知该值存在
			 *	定义E1 := (dir == 0), E2 := (dir == 1)，可知(E1 || E2) == TRUE
			 *	化简：A == !C1 && !C2 && !C3
			 *		== !C1 && !C2 && !C3
			 *		== !C1 && (!(D1 && E1) && !(D2 && E2))
			 *		== !C1 && ((!D1 || !E1) && (!D2 && !E2))
			 *		== !C1 && ((!D1 && !D2) || (!D1 && !E2) || (!E1 && !D2) || (!E1 && !E2))
			 *		== !C1 && (!(D1 || D2) || (!D1 && !E2) || (!D2 && !E1) || !(E1 || E2))
			 *		== !C1 && (0 || (!D1 && !E2) || (!D2 && !E1) || !(E1 || E2))
			 *		== !C1 && ((!D1 && !E2) || (!D2 && !E1))
			 *	含义：从新节点到根的自下而上的路径上，存在非平衡节点，且在最后一个非平衡节点处，插入的方向和原倾斜方向相同，导致更加倾斜，破坏了AVL树的平衡性
			 */
			if ((fbn == NULL) || (fbn->balance == -1 && ((path >> (k - 1)) & 1) == 0) || (fbn->balance == 1 && ((path >> (k - 1)) & 1) == 1))
				// 不需要旋转的三种情况，只需要修正平衡因子即可
				__set_fix_balance(nnode, fbn);
			else {
				// 需要旋转的情况，首先还是修正平衡因子
				__set_fix_balance(nnode, fbn);
				// 计算从fbn开始的两步走向，因为需要旋转的情况都是走向与倾向相同，所以从最后一个不平衡点开始都至少要走两步，即k >= 2
				unsigned int dir = (path >> (k - 2)) & 3;	// dir = d1 d2
				if (dir == 0) {				// 连续两步向左，以fbn->left为轴进行一次右旋
					fbn->balance = 0;
					fbn->left->balance = 0;
					__set_r_right(set, fbn->left);
				} else if (dir == 3) {			// 连续两步向右，以fbn->right为轴进行一次右旋
					fbn->balance = 0;
					fbn->right->balance = 0;
					__set_r_left(set, fbn->right);
				} else if (dir == 1) {			// 先左后右，进行两次旋转，先以fbn->left->right为轴左旋，再以fbn->left为轴右旋
					int bal = fbn->left->right->balance;
					fbn->left->right->balance = 0;
					if (bal == 0) {
						fbn->balance = 0;
						fbn->left->balance = 0;
					} else if (bal == 1) {
						fbn->balance = -1;
						fbn->left->balance = 0;
					} else {
						fbn->balance = 0;
						fbn->left->balance = 1;
					}
					__set_r_left(set, fbn->left->right);
					__set_r_right(set, fbn->left);
				} else {				// 先右后左，进行两次旋转，先以fbn->right->left为轴右旋，再以fbn->right为轴左旋
					int bal = fbn->right->left->balance;
					fbn->right->left->balance = 0;
					if (bal == 0) {
						fbn->balance = 0;
						fbn->right->balance = 0;
					} else if (bal == 1) {
						fbn->balance = 0;
						fbn->right->balance = -1;
					} else {
						fbn->balance = 1;
						fbn->right->balance = 0;
					}
					__set_r_right(set, fbn->right->left);
					__set_r_left(set, fbn->right);
				}
			}
			set->size++;
			ret = 0;
		}
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(set->mut));
	}
	return ret;
}

/**
 * 删除一个元素，根据参数ele查找集合中与之相同的元素，删除该节点，返回集合中的元素
 * s:		Set句柄
 * ele:		待删除的元素
 *
 * 返回:	删除成功返回集合中的元素，删除失败或未找到返回NULL
 */
Element set_remove(Set s, Element ele)
{
	Element ret = NULL;
	set_p set = (set_p)container_get(s, Set_t);
	if (ele != NULL && set != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(set->mut));
		set_node_p del = __set_get(set, ele);			// 查找要删除的节点
		set_node_p todel = NULL;				// 真正的待删除节点
		if (del != NULL) {					// 查找不到要删除的节点的话直接返回NULL
			if (del->left == NULL || del->right == NULL)
				todel = del;				// 待删除节点最多只有一个子节点，那么就删除它
			else
				todel = __set_successor(del);		// 待删除节点有两个子节点，那么删除它的后继节点
		}
		/* 寻找第一个调整后不平衡的父节点 */
		set_node_p fubp = __set_first_ubparent(todel);		// fubp: first unbalance parent
		/* 获取待删除节点的子节点，根据二叉搜索树删除节点的规则，todel节点最多只有一个子节点 */
		set_node_p next = NULL;
		if (todel->left != NULL)
			next = todel->left;
		else
			next = todel->right;
		/* 删除节点todel */
		if (next != NULL)
			next->parent = todel->parent;
		if (todel->parent == NULL)
			set->root = next;
		else if (todel->parent->left == todel)
			todel->parent->left = next;
		else
			todel->parent->right = next;
		/* 保留要返回的元素 */
		ret = del->element;
		/* 如果del和todel不是同一个节点，那么把todel节点中的元素换到del节点中去保存 */
		if (todel != del)
			del->element = todel->element;
		/* 销毁todel节点 */
		free(todel);
		/* 删除完成，从fubp开始修正平衡因子，维护AVL树结构 */
		__set_after_remove(set, fubp);
		/* 修改集合的size值 */
		set->size--;
		if (set->size == 0)
			set->root = NULL;
		if (__MultiThreads__ == 1)
			pthread_mutex_unlock(&(set->mut));
	}
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

/**
 * 求两个集合的交集，如果两个集合的元素数据类型不一致则返回空集合
 * 如果两个集合的元素比较函数不同则使用s1的cmpfunc进行元素比较，并且结果集合也采用s1的cmpfunc为其元素比较函数
 * s1,s2:	两个集合的句柄
 *
 * 返回:	s1和s2的交集的句柄，是一个新建的集合，如果s1和s2中有至少一个无效，则返回-1
 */
Set set_intersection(Set s1, Set s2)
{
	Set ret = -1;
	return ret;
}

/**
 * 求两个集合的并集，如果两个集合的元素数据类型不一致则返回空集合
 * 如果两个集合的元素比较函数不同则使用s1的cmpfunc进行元素比较，并且结果集合也采用s1的cmpfunc为其元素比较函数
 * s1,s2:	两个集合的句柄
 *
 * 返回:	s1和s2的并集的句柄，是一个新建的集合，如果s1和s2中有至少一个无效，则返回-1
 */
Set set_union(Set s1, Set s2)
{
	Set ret = -1;
	return ret;
}

/**
 * 求两个集合的减集，即s1-s2，从s1中删除所有存在于s2中的元素，如果两个集合的元素数据类型不一致则结果集与s1的元素相同
 * 如果两个集合的元素比较函数不同则使用s1的cmpfunc进行元素比较，并且结果集合也采用s1的cmpfunc为其元素比较函数
 * s1,s2:	两个集合的句柄
 *
 * 返回:	集合s1-s2的句柄，是一个新建的集合，如果s1和s2中有至少一个无效，则返回-1
 */
Set set_minus(Set s1, Set s2)
{
	Set ret = -1;
	return ret;
}

static void __postorder_remove(set_node_p root, onRemove onremove)	// 后序遍历删除所有节点
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

static set_node_p __set_get(set_p set, Element ele)			// 在集合set中搜索与ele相同的元素，返回其节点，搜索不到返回NULL
{
	set_node_p p = set->root;
	while (p != NULL) {
		int cmp = set->cmpfunc(ele, p->element);
		if (cmp < 0)
			p = p->left;
		else if (cmp > 0)
			p = p->right;
		else
			break;
	}
	return p;
}

static void __set_fix_balance(set_node_p from, set_node_p to)		// 从from到to自下而上的修正平衡因子
{
	set_node_p nnode = from;
	while (nnode != to) {
		if (nnode->parent == NULL)
			break;
		if (nnode == nnode->parent->left)
			nnode->parent->balance++;
		else
			nnode->parent->balance--;
		nnode = nnode->parent;
	}
}

static void __set_r_left(set_p set, set_node_p node)			// 以node节点为轴左旋
{
	set_node_p parent = node->parent;				// 断言: 用作旋转的轴的节点必然不是root
	if (parent == set->root) {
		set->root = node;
		node->parent = NULL;
	} else {
		node->parent = parent->parent;
		if (parent->parent->left == parent)
			parent->parent->left = node;
		else
			parent->parent->right = node;
	}
	parent->right = node->left;
	if (node->left != NULL)
		node->left->parent = parent;
	node->left = parent;
	parent->parent = node;
}

static void __set_r_right(set_p set, set_node_p node)			// 以node节点为轴右旋
{
	set_node_p parent = node->parent;
	if (parent == set->root) {
		set->root = node;
		node->parent = NULL;
	} else {
		node->parent = parent->parent;
		if (parent->parent->left == parent)
			parent->parent->left = node;
		else
			parent->parent->right = node;
	}
	parent->left = node->right;
	if (node->right != NULL)
		node->right->parent = parent;
	node->right = parent;
	parent->parent = node;
}

static set_node_p __set_successor(set_node_p p)		// 返回已知节点的后继节点，即按递增顺序排列的后一个节点，当前节点是树中最大值时返回NULL
{
	// 如果这个节点有右子树则返回右子树中的最小节点，否则向上寻找直到第一个向右转的父节点(当前节点在该节点的左子树里)，没有向右转的父节点则返回NULL
	set_node_p ret = NULL;
	if (p != NULL) {
		set_node_p tmp = p;
		if (tmp->right != NULL) {
			ret = __set_min(tmp->right);
		} else {
			ret = tmp->parent;
			while (ret != NULL && tmp == ret->right) {
				tmp = ret;
				ret = ret->parent;
			}
		}
	}
	return ret;
}

static set_node_p __set_min(set_node_p root)		// 返回树中最小的节点
{
	set_node_p ret = root;
	if (ret != NULL)
		while (ret->left != NULL)
			ret = ret->left;
	return ret;
}

static set_node_p __set_max(set_node_p root)		// 返回树中最大的节点
{
	set_node_p ret = root;
	if (ret != NULL)
		while (ret->right != NULL)
			ret = ret->right;
	return ret;
}

/**
 * 从父节点开始自下而上修正平衡因子直到找到一个非0平衡因子的节点或抵达root，根据二叉平衡树删除节点的规则，删除节点必定导致节点所在子树的高度减少1层，因此修正后有三种情况
 * 1. 修正后balance == 0: 说明原先的平衡因子为1或者-1，即原先由一层的平衡叉，删除后变为0，父节点的高度减1，因此要继续向上一层父节点循环查看
 * 2. 修正后balance == 1或-1: 说明原先完全平衡，现在某一子树减少了一层，但是父节点的总高度不变，因此不会影响更上层的平衡因子，修正结束，节点删除后不需要旋转处理
 * 3. 修正后balance == 2或-2: 树的平衡性已经破坏，节点删除后要进行旋转处理
 */
static set_node_p __set_first_ubparent(set_node_p node)			// 从当前节点开始向上调整父节点的平衡因子直到第一个平衡因子非0的父节点，并返回该父节点
{
	set_node_p pend = node;
	while (pend != NULL) {
		if (pend->parent == NULL)
			break;
		if (pend == pend->parent->left)
			pend->parent->balance--;
		else
			pend->parent->balance++;
		pend = pend->parent;
		if (pend->balance != 0)
			break;
	}
	return pend;
}

static void __set_after_remove(set_p set, set_node_p del)		// 调整删除节点后的AVL树结构并修正平衡因子
{
	if (del == NULL || del->balance == 1 || del->balance == -1 || del->balance == 0)	// 节点为NULL或已经平衡，则直接退出
		return;
	if (del->balance == 2) {					// 左子树比右子树高2层，要做右旋调整
		if (del->left->balance == 0) {				// R0旋转
			del->balance = 1;
			del->left->balance = -1;
			__set_r_right(set,del->left);
			return;						// R0旋转可以保证AVL树整体平衡，所以直接返回
		} else if (del->left->balance == 1) {			// R1旋转
			del->balance = 0;
			del->left->balance = 0;
			__set_r_right(set, del->left);
		} else {						// LR旋转
			if (del->left->right->balance == 0) {
				del->balance = 0;
				del->left->balance = 0;
			} else if (del->left->right->balance == 1) {
				del->balance = -1;
				del->left->balance = 0;
			} else {
				del->balance = 0;
				del->left->balance = 1;
			}
			del->left->right->balance = 0;
			__set_r_left(set, del->left->right);
			__set_r_right(set, del->left);
		}
	} else if (del->balance == -2) {				// 右子树比左子树高2层，要做左旋调整
		if (del->right->balance == 0) {				// L0旋转
			del->balance = -1;
			del->right->balance = 1;
			__set_r_left(set, del->right);
			return;						// L0旋转可以保证AVL树整体平衡，所以直接返回
		} else if (del->right->balance == -1) {			// L1旋转
			del->balance = 0;
			del->right->balance = 0;
			__set_r_left(set, del->right);
		} else {						// RL旋转
			if (del->right->left->balance == 0) {
				del->balance = 0;
				del->right->balance = 0;
			} else if (del->right->left->balance == -1) {
				del->balance = 1;
				del->right->balance = 0;
			} else {
				del->balance = 0;
				del->right->balance = -1;
			}
			del->right->left->balance = 0;
			__set_r_right(set, del->right->left);
			__set_r_left(set, del->right);
		}
	}
	// R1, LR, L1, RL旋转后可能更上层的平衡性被破坏，所以要继续向根部寻找非平衡点进行调整，直到根节点，使用递归方法进行循环
	del = __set_first_ubparent(del->parent);
	__set_after_remove(set, del);					// 递归
}

