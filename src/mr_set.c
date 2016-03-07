#include <stdlib.h>
#include <pthread.h>

#include "mr_containers.h"
#include "mr_set.h"

static const int __IT_ASC = 1;
static const int __IT_DESC = 0;

/**
 * 红黑树节点颜色
 */
typedef enum {
	Red,
	Black
} RBT_Color;

/**
 * 集合节点结构，即红黑树的节点结构
 */
typedef struct RBT_Node {
	Element element;		// 元素
	struct RBT_Node *left;		// 左子树根节点
	struct RBT_Node *right;		// 右子树根节点
	struct RBT_Node *parent;	// 父节点
	RBT_Color color;		// 节点颜色
} rbt_node_t, *rbt_node_p;

/**
 * 集合结构
 */
typedef struct {
	ElementType type;		// 元素的数据类型
	rbt_node_p root;		// 根节点
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
	rbt_node_p *stack;		// 迭代用的堆栈
	rbt_node_p *top;		// 栈顶指针
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

static rbt_node_p __rbt_new_node(Element ele);					// 创建一个新节点
static void __rbt_removeall(rbt_node_p root, onRemove onremove);		// 后序遍历删除所有节点

static rbt_node_p __rbt_search_aux(Element ele, rbt_node_p root, CmpFunc cmpfunc, rb_node_p *save);	// 从root开始搜索指定元素所在节点的辅助函数，如果指定元素没有找到，可以通过save保存插入点
static rbt_node_p __rbt_search(Element ele, rbt_node_p root, CmpFunc cmpfunc);				// 从root开始查找元素与ele相等的节点并返回，找不到返回NULL

static rbt_node_p __rbt_rotate_left(rbt_node_p node, rbt_node_p root);		// 以node节点为轴左旋，返回旋转后的根节点
static rbt_node_p __rbt_rotate_right(rbt_node_p node, rbt_node_p root);		// 以node节点为轴右旋，返回旋转后的根节点

static rbt_node_p __rbt_insert(Element ele, rbt_node_p root, CmpFunc cmpfunc);	// 向根为root的红黑树中插入一个元素，如果元素存在则不做任何操作，返回插入完成后的根节点
static rbt_node_p __rbt_delete(rbt_node_p dele, rbt_node_p root)		// 从根为root的红黑树中删除一个节点，返回删除后的根节点
static rbt_node_p __rbt_insert_rebalance(rbt_node_p node, rbt_node_p root);	// 红黑树插入节点后重新平衡
static rbt_node_p __rbt_delete_rebalance(rbt_node_p node, rbt_node_p parent, rbt_node_p root);		// 红黑树删除节点后重新平衡

static void __it_push(set_it_p it, rbt_node_p node);			// 迭代用的压栈函数
static rbt_node_p __it_pop(set_it_p it);				// 迭代用的弹栈函数
static int __it_stack_empty(set_it_p it);				// 迭代用的空栈判断函数

static set_it_p __iterator(set_p s, int asc);				// 生成一个迭代器
static rbt_node_p __it_next(set_it_p it);				// 用Mirros算法中序迭代一个迭代器

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
		__rbt_removeall(set->root, NULL);		/* 删除所有节点，用后序遍历逐个释放每一个节点，但不释放其中的元素 */
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
	if (ele != NULL && set != NULL && set->root != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(set->mut));
		rbt_node_p result = __rbt_search(ele, set->root, set->cmpfunc);
		ret = (result == NULL ? NULL : node->element);
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
		rbt_node_p r = __rbt_insert(ele, set->root, set->cmpfunc);
		if (r != NULL) {		// 插入时如果元素重复则返回NULL，否则返回插入后的红黑树的根节点，这是因为插入操作可能改变树的根节点
			set->root = r;
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
	if (ele != NULL && set != NULL && set->root != NULL) {
		if (__MultiThreads__ == 1)
			pthread_mutex_lock(&(set->mut));
		rbt_node_p node = __rbt_search(ele, set->root, set->cmpfunc);
		if (node != NULL) {		// 找到要删除的元素
			ret = node->element;
			set->root = __rbt_delete(node, set->root);
			set->size--;
		}
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
		__rbt_removeall(set->root, onremove);	/* 删除所有节点，用后序遍历逐个释放每一个节点，用onremove参数进行元素的后续处理 */
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
		ret = __iterator(set, __IT_ASC);
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
		ret = __iterator(set, __IT_DESC);
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
		rbt_node_p next = __it_next(iterator);
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

static void __rbt_removeall(rbt_node_p root, onRemove onremove)		// 后序遍历删除所有节点
{
	if (root != NULL) {
		__rbt_removeall(root->left, onremove);
		__rbt_removeall(root->right, onremove);
		if (onremove != NULL)
			onremove(root->element);
		free(root);
	}
}

static void __it_push(set_it_p it, rbt_node_p node)			// 迭代用的压栈函数
{
	*(it->top++) = node;
}

static rbt_node_p __it_pop(set_it_p it)					// 迭代用的弹栈函数
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
	unsigned int len = lg2(set->size + 1);
	len = len << 1;			// 红黑树最大树高度小于2*lg2(size+1)
	ret->stack = (rbt_node_p *)malloc(len  * sizeof(rbt_node_p));
	ret->top = ret->stack;
	rbt_node_p current = set->root;
	while (current != NULL) {
		__it_push(ret, current);
		current = ret->asc ? current->left : current->right;
	}
	return ret;
}

static rbt_node_p __it_next(set_it_p it) {					// 中序迭代一个迭代器
	rbt_node_p ret = NULL;
	if (!__it_stack_empty(it)) {
		ret = __it_pop(it);
		if (it->asc ? ret->right != NULL : ret->left != NULL) {
			rbt_node_p current = it->asc ? ret->right : ret->left;
			while (current != NULL) {
				__it_push(it, current);
				current = it->asc ? current->left : current->right;
			}
		}
	}
	return ret;
}

/**
 * 算法描述：
 * ITERATIVE-TREE-SEARCH(x, k)
 * 1	while x != NIL and k != key[x]
 * 2		do if k < key[x]
 * 3			then x := left[x]
 * 4			else x := right[x]
 * 5	return x
 */
static rbt_node_p __rbt_search_aux(Element ele, rbt_node_p root, CmpFunc cmpfunc, rb_node_p *save)	// 从root开始搜索指定元素所在节点的辅助函数，如果指定元素没有找到，可以通过save保存插入点
{
	rbt_node_p ret = root, parent = NULL;
	int cmp;
	while (ret != NULL && (cmp = cmpfunc(ele, p->element)) != 0) {
		parent = ret;
		if (cmp < 0)
			ret = ret->left;
		else
			ret = ret->right;
	}
	if (!ret && save)	// ret == NULL: 1) root == NULL, 此时parent == NULL; 2) root != NULL, 此时parent指向插入点
		*save = parent;
	return ret;
}

static rbt_node_p __rbt_search(Element ele, rbt_node_p root, CmpFunc cmpfunc)		// 从root开始查找元素与ele相等的节点并返回，找不到返回NULL，调用__rbt_search_aux()实现
{
	return __rbt_search_aux(ele, root, cmpfunc, NULL);
}

/**
 * 算法描述：
 *-----------------------------------------------------------
 *   node             rnode
 *    / \     	      / \
 *   a  rnode  ==>   node  y
 *      / \         / \
 *     b   y       a   b
 *-----------------------------------------------------------
 * LEFT-ROTATE(T, x)
 * 1	y := right[x]				// Set y.
 * 2	right[x] := left[y]			// Turn y's left subtree into x's right subtree
 * 3	if left[y] != nil[T]
 * 4		then p[left[y]] := x
 * 5	p[y] := p[x]
 * 6	if p[x] == nil[T]
 * 7		then root[T] := y;
 * 8		else if x == left[p[x]]
 * 9			then left[p[x]] := y
 *10			else right[p[x]] := y
 *11	left[y] := x;				// Put x on y's left
 *12	p[x] := y;
 */
static rbt_node_p __rbt_rotate_left(rbt_node_p node, rbt_node_p root)		// 以node节点为轴左旋，返回旋转后的根节点
{
	rbt_node_p rnode = node->right;			// 1
	if (node->right = rnode->left)			// 2,3
		rnode->left->parent = node;		// 4
	if (rnode->parent = node->parent)		// 5,6
		if (node == node->parent->right)	// 8,9,10
			node->parent->right = rnode;
		else
			node->parent->left = rnode;
	else
		root = rnode;				// 7
	rnode->left = node;				// 11
	node->parent = rnode;				// 12
	return root;
}

/**
 * 算法描述
 *----------------------------------------------------------
 *     node           lnode 
 *     /  \           /  \
 *   lnode y   ==>   a   node 
 *   / \                 / \
 *  a   b               b   y
 *----------------------------------------------------------
 * 算法与左旋对称，即对换left和right分支即可，不再描述伪代码
 */
static rbt_node_p __rbt_rotate_right(rbt_node_p node, rbt_node_p root)		// 以node节点为轴右旋，返回旋转后的根节点
{
	rbt_node_p lnode = node->left;
	if (node->left = lnode->right)
		lnode->right->parent = node;
	if (lnode->parent = node->parent)
		if (node == node->parent->right)
			node->parent->right = lnode;
		else
			node->parent->left = lnode;
	else
		root = lnode;
	lnode->right = node;
	node->parent = lnode;
	return root;
}

static rbt_node_p __rbt_new_node(Element ele)					// 创建一个新节点
{
	set_node_p nnode = (set_node_p)malloc(sizeof(set_node_t));
	nnode->element = ele;
	nnode->left = NULL;
	nnode->right = NULL;
	nnode->parent = NULL;
	nnode->color = Red;
}

/**
 * 红黑树插入新节点算法描述：
 * RB-INSERT(T, z)
 * 1	y := nil[T]
 * 2	x := root[T]
 * 3	while x != nil[T]
 * 4		do y := x
 * 5			if key[z] < key[x]
 * 6				then x := left[x]
 * 7				else x := right[x]
 * 8	p[z] := y
 * 9	if y == nil[T]
 *10		then root[T] := z
 *11		else if key[z] < key[y]
 *12			then left[y] := z
 *13			else right[y] := z
 *14	left[z] := nil[T]
 *15	right[z] := nil[T]
 *16	color[z] := RED
 *17	RB-INSERT-FIXUP(T, z)
 *-------------------------------------------------------
 * 如果插入完成返回根节点，因为插入元素可能导致根节点发生变化；如果已经有相等的元素存在则返回NULL，表示未进行插入操作
 */
static rbt_node_p __rbt_insert(Element ele, rbt_node_p root, CmpFunc cmpfunc)		// 向根为root的红黑树中插入一个元素，如果元素存在则不做任何操作，返回插入完成后的根节点
{
	rbt_node_p parent = NULL, node;
	if (node = __rbt_search_aux(ele, root, cmpfunc, &parent))	// 寻找插入点，如果相等的元素已经存在则返回原节点，否则返回NULL并在parent中存放插入点
		return NULL;
	node = __rbt_new_node(ele);
	node->parent = parent;     
	if (parent)	// 插入点非空，即原树不为空
		if (cmpfunc(parent->element, ele) > 0)
			parent->left = node;
		else
			parent->right = node;
	else		// 原树为空树，新节点即为根节点
		root = node;
	// 至此二叉查找树的节点插入结束，接下来进行再平衡，修复红黑树的性质并返回根节点
	return __rbt_insert_rebalance(node, root);
}

/**
 * 红黑树删除节点的算法描述：
 * RB-DELETE(T, z)
 * 1	if left[z] == nil[T] or right[z] == nil[T]		// 找到真正要删除的节点y
 * 2		then y := z
 * 3		else y := TREE-SUCCESSOR(z)			// 求最小后继，就是中序遍历的后继节点
 * 4	if left[y] != nil[T]
 * 5		then x := left[y]				// y只有一棵子树
 * 6		else x := right[y]
 * 7	p[x] := p[y]						// 红黑树的算法描述中没有NULL，只有哨兵节点nil[T]，所以在算法描述中不判断x是否为NULL
 * 8	if p[y] == nil[T]
 * 9		then root[T] := x
 *10		else if y == left[p[y]]
 *11			then left[p[y]] := x
 *12			else right[p[y]] := x
 *13	if y != z
 *14		then key[z] := key[y]
 *15		     copy y's satellite data into z
 *16	if color[y] == BLACK
 *17		then RB-DELETE-FIXUP(T, x)
 *18	return y
 *--------------------------------------------------------------
 * 在实现时对算法进行了一些优化调整，在使用中序后继作为替身节点时并非复制替身数据再删除替身，而是把替身节点接到待删除节点的位置，然后销毁待删除节点，从而节省复制元素数据的开销
 */
static rbt_node_p __rbt_delete(rbt_node_p dele, rbt_node_p root)	// 从根为root的红黑树中删除一个节点，返回删除后的根节点，注意如果删除了最后一个节点（一定是根节点）那么要返回NULL
{
	rbt_node_p child, parent, old, left, node;
	RBT_Color color;

	old = dele;					// old  = 待删除的节点
	node = dele;					// 替身节点node，可能就是old自身，也可能是他的中序后继
	if (node->left && node->right) {	// 情况1：待删除节点左右子节点均存在，在右子树中寻找中序后继作为替身，该替身必定没有左子节点
		node = node->right;
		while ((left = node->left) != NULL)
			node = left;
		child = node->right;			// child  = 实际删除的节点node的右子节点，也是删除后进行修复的起点
		parent = node->parent;			// parent = 实际删除的节点node的父节点，也是删除后进行修复的起点的父节点
		color = node->color;
		if (child)				// node的右子节点child接替node，接入node的父节点的对应方向上，如果node是根节点，那么child成为新的根节点
			child->parent = parent;  
		if (parent)
			if (parent->left == node)
				parent->left = child;
			else
				parent->right = child;
		else
			root = child;
		if (node->parent == old)		// 如果node的父节点就是要删除的节点old，那么把parent改成node自己，保持在删除后修复时parent和child的正确取值
			parent = node;
		node->parent = old->parent;		// 把替身接入到待删除节点的位置，并染成和待删除节点相同的颜色，注意要处理抵达根节点的情况
		node->color = old->color;
		node->right = old->right;
		node->left = old->left;
		if (old->parent)
			if (old->parent->left == old)
				old->parent->left = node;
			else
				old->parent->right = node;
		else
			root = node;
		old->left->parent = node;
		if (old->right)
			old->right->parent = node;	// 待删除节点左右子树均存在的情况处理完毕，修复的起点child, parent和根节点root均处理正确，可以释放old并进行红黑树修复
	} else {				// 情况2：待删除节点最多只有一个子树
		if (!node->left)
			child = node->right;		// 左子树为空，此处包含右子树也为空的情况，所以当待删除节点左子树为空时，取到的child有可能为空
		else if (!node->right)
			child = node->left;		// 右子树为空，此处else if是多余的，因为已经判断过左子树不为空且最多只有一棵子树了，所以此时右子树必然为空，左子树必然不空，child必然不空
		parent = node->parent;
		color = node->color;
		if (child)				// 子树绕过待删除节点直接接到待删除节点的父节点相应方向上，游离出待删除节点，注意处理抵达根节点的情况
			child->parent = parent;
		if (parent)
			if (parent->left == node)
				parent->left = child;
			else
				parent->right = child;
		else
			root = child;
	}
	free(old);				// 销毁待删除的节点
	if (color == BLACK)			// 实际消失的位置原有节点的颜色，对于情况一是替身的颜色，情况二则没有替身，就是被删除节点的颜色
		root = __rbt_delete_rebalance(child, parent, root);	// 恢复红黑树性质，此处child是真正消失了一个节点的位置，parent是出现在消失节点位置上的新节点的父节点
	return root;
}

/**
 * 红黑树插入新节点后重新平衡的算法描述：
 * 需要修复的情况有三种，以当前节点的父节点是其祖父节点的左子节点为例
 * Case 1: 父节点和叔叔节点都是红色
 * Case 2: 父节点为红色，叔叔节点为黑色，当前节点是父节点的右子节点
 * Case 3: 父节点为红色，叔叔节点为黑色，当前节点是父节点的左子节点
 * 修复刚开始的第一个循环里，只有可能出现Case 1，对Case 1进行修复后有可能导致在祖父节点处变成Case 2或Case 3
 * 修复Case 2必然会导致原父节点处出现Case 3，因此应该接下去马上进行修复
 * 修复Case 3后可能会变成Case 1，因此重新进入循环即可
 * RB-INSERT-FIXUP(T, z)
 * 1	while color[p[z]] == RED
 * 2		do if p[z] == left[p[p[z]]]					// Parent is the left son of grandparent
 * 3			then y := right[p[p[z]]]				// Put uncle node to y
 * 4				if color[y] == RED				// Case 1 detected
 * 5					then color[p[z]] := BLACK		// Fix Case 1: Paint parent to BLACK
 * 6					     color[y] := BLACK			// Fix Case 1: Paint uncle to BLACK
 * 7					     color[p[p[z]]] := RED		// Fix Case 1: Paint grandparent to RED
 * 8					     z := p[p[z]]			// Fix Case 1: Let z := grandparent and loop again
 * 9					else if z == right[p[z]]		// Case 2 detected
 *10						then z := p[z]			// Fix Case 2: Let z := parent as the pivot of rotate
 *11						     LEFT-RETATE(T, z)		// Fix Case 2: Left rotate
 *12						color[p[z]] := BLACK		// Fix Case 3: Paint parent to BLACK
 *13						color[p[p[z]]] := RED		// Fix Case 3: Paint grandparent to RED
 *14						RIGHT-ROTATE(T, p[p[z]])	// Fix Case 3: Right rotate on grandparent
 *15			else (For the condition that the parent is the right son of grandparent, same as then clause with "right" and "left" exchanged)
 */
static rbt_node_p __rbt_insert_rebalance(rbt_node_p node, rbt_node_p root)	// 红黑树插入节点后重新平衡
{
	rbt_node_p parent, grandpa, uncle, temp;
	while ((parent = node->parent) && parent->color == Red) {		// 1
		grandpa = parent->parent;
		if (parent == grandpa->left) {					// 2
			uncle = grandpa->right;					// 3
			if (uncle && uncle->color == Red) {			// 4 如果uncle是NULL，那么按规则默认NULL的颜色为黑
				parent->color = Black;				// 5
				uncle->color = Black;				// 6
				grandpa->color = Red;				// 7
				node = grandpa;					// 8
			} else {
				if (parent->right == node) {			// 9
					root = __rbt_rotate_left(parent, root);	// 10,11
					temp = parent;
					parent = node;
					node = temp;
				}
				parent->color = Black;				// 12
				grandpa->color = Red;				// 13
				root = __rbt_rotate_right(grandpa, root);	// 14
			}
		} else {							// 15
			uncle = grandpa->left;
			if (uncle && uncle->color == Red) {
				parent->color = Black;
				uncle->color = Black;
				grandpa->color = Red;
				node = grandpa;
			} else {
				if (parent->left == node) {
					root = __rbt_rotate_right(parent, root);
					temp = parent;
					parent = node;
					node = temp;
				}
				parent->color = Black;
				grandpa->color = Red;
				root = __rbt_rotate_left(grandpa, root);
			}
		}
	}
	root->color = Black;							// 根节点必须为黑色
	return root;
}

/**
 * 红黑树删除节点后修复平衡算法的描述：
 * 当前节点的颜色为黑才需要修复，需要修复的情况有四种，以当前节点在父节点的左分支为例
 * Case 1: 兄弟节点为红色
 * Case 2: 兄弟节点为黑色，且兄弟节点的两个子节点均为黑色
 * Case 3: 兄弟节点为黑色，兄弟节点的左子节点为红色，右子节点为黑色
 * Case 4: 兄弟节点为黑色，兄弟节点的右子节点为红色，左子节点颜色任意
 * RB-DELETE-FIXUP(T, x)
 * 1	while x != root[T] and color[x] == BLACK
 * 2		do if x == left[p[x]]
 * 3			then w := right[p[x]]					// w: sibling node
 * 4				if color[w] == RED				// Case 1 detected, fix case 1 will paint sibling to black, so could cause case 2, 3
 * 5					then color[w] := BLACK			// Fix Case 1
 * 6					     color[p[x]] := RED			// Fix Case 1
 * 7					     LEFT-ROTATE(T, p[x])		// Fix Case 1
 * 8					     w := right[p[x]]			// Fix Case 1
 * 9				if color[left[w]] == BLACK and color[right[w]] == BLACK		// Case 2 detected
 *10					then color[w] := RED			// Fix Case 2
 *11					     x := p[x]				// Fix Case 2, paint sibling to red, loop again as parent, then loop will definitely stop and paint parent to black
 *12					else if color[right[w]] == BLACK	// Case 3 detected
 *13						then color[left[w]] := BLACK	// Fix Case 3
 *14						     color[w] := RED		// Fix Case 3
 *15						     RIGHT-ROTATE(T, w)		// Fix Case 3
 *16						     w := right[p[x]]		// Fix Case 3
 *17						color[w] := color[p[x]]		// Fix Case 4, fix case 3 definitely cause case 4, and case 4 can only be caused by fixing case 3
 *18						color[p[x]] := BLACK		// Fix Case 4
 *19						color[right[w]] := BLACK	// Fix Case 4
 *20						LEFT-ROTATE(T, p[x])		// Fix Case 4
 *21						x := root[T]			// Fix Case 4, algorithm must be finished after fixed case 4
 *22			else (same as then clause with "right" and "left" exchanged)
 *23	color[x] := BLACK
 */
static rbt_node_p __rbt_delete_rebalance(rbt_node_p node, rbt_node_p parent, rbt_node_p root)	// 红黑树删除节点后重新平衡
{
	rbt_node_p sibling;
	while ((!node || node->color == BLACK) && node != root) {
		if (parent->left == node) {
			sibling = parent->right;
			if (sibling->color == RED) {		//情况1：兄弟节点是红色的
				sibling->color = BLACK;
				parent->color = RED;
				root = __rbt_rotate_left(parent, root);
				sibling = parent->right;
			}
			if ((!sibling->left || sibling->left->color == BLACK) && (!sibling->right || sibling->right->color == BLACK)) {		// 情况2：兄弟和兄弟的两个子节点都是黑色的
				sibling->color = RED;
				node = parent;
				parent = node->parent;	// 修正parent变量，保证进入循环时变量正确
			} else {
				if (!sibling->right || sibling->right->color == BLACK) {	//情况3：兄弟是黑色的，且兄弟的左子节点是红色，右子节点是黑色
					if (sibling->left)
						sibling->left->color = BLACK;
					sibling->color = RED;
					root = __rbt_rotate_right(sibling, root);
					sibling = parent->right;
				}
				//情况4：兄弟是黑色的，且兄弟的右子节点是红色的，左子节点无所谓，这种情况只能且必然在情况3修复后出现
				sibling->color = parent->color;
				parent->color = BLACK;
				if (sibling->right)
					sibling->right->color = BLACK;
				root = __rbt_rotate_left(parent, root);
				node = root;
				break;		// 情况4修复完毕后，算法必然结束，因此跳出循环，把根节点染成黑色即可
			}
		} else {
			sibling = parent->left;
			if (sibling->color == RED) {
				sibling->color = BLACK;
				parent->color = RED;
				root = __rbt_rotate_right(parent, root);
				sibling = parent->left;
			}
			if ((!sibling->left || sibling->left->color == BLACK) && (!sibling->right || sibling->right->color == BLACK)) {  
				sibling->color = RED;
				node = parent;
				parent = node->parent;
			} else {
				if (!sibling->left || sibling->left->color == BLACK) {
					if (sibling->right)
						sibling->right->color = BLACK;
					sibling->color = RED;
					root = __rbt_rotate_left(sibling, root);
					sibling = parent->left;
				}
				sibling->color = parent->color;
				parent->color = BLACK;
				if (sibling->left)
					sibling->left->color = BLACK;
				root = __rbt_rotate_right(parent, root);
				node = root;
				break;
			}
		}
	}
	if (node)
		node->color = BLACK;		// 根节点设置为黑色
	return root;
}

