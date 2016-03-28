#include <stdlib.h>
#include <pthread.h>

#include "mr_set.h"
#include "private_element.h"

#define IS_VALID_SET(X) (X && X->container && X->type == Set)

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
	element_p element;		// 元素
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
	unsigned int changes;		// 集合内容发生变更的次数
	pthread_mutex_t mut;		// 共享锁
} set_t, *set_p;

/**
 * 集合迭代器
 */
typedef struct {
	set_p set;			// 迭代的集合，用于加访问锁
	int asc;			// 迭代方向，1=正向，0=反向
	rbt_node_p *stack;		// 迭代用的堆栈
	rbt_node_p *top;		// 栈顶指针
	unsigned int changes;		// 迭代器创建时的集合变更次数，用于fast-fail
} set_it_t, *set_it_p;

static rbt_node_p __rbt_new_node(element_p element);				// 创建一个新节点
static void __rbt_destroy_node(rbt_node_p node);				// 销毁一个节点及其中的元素
static void __rbt_removeall(rbt_node_p root);					// 后序遍历删除所有节点

static rbt_node_p __rbt_search_aux(element_p ele, rbt_node_p root, CmpFunc cmpfunc, rbt_node_p *save);	// 从root开始搜索指定元素所在节点的辅助函数，如果指定元素没有找到，可以通过save保存插入点
static rbt_node_p __rbt_search(element_p ele, rbt_node_p root, CmpFunc cmpfunc);			// 从root开始查找元素与ele相等的节点并返回，找不到返回NULL

static rbt_node_p __rbt_rotate_left(rbt_node_p node, rbt_node_p root);		// 以node节点为轴左旋，返回旋转后的根节点
static rbt_node_p __rbt_rotate_right(rbt_node_p node, rbt_node_p root);		// 以node节点为轴右旋，返回旋转后的根节点

static rbt_node_p __rbt_insert(element_p ele, rbt_node_p root, CmpFunc cmpfunc);		// 向根为root的红黑树中插入一个元素，如果元素存在则不做任何操作，返回插入完成后的根节点
static rbt_node_p __rbt_insert_rebalance(rbt_node_p node, rbt_node_p root);			// 红黑树插入节点后重新平衡
static rbt_node_p __rbt_delete(rbt_node_p node, rbt_node_p root);				// 从根为root的红黑树中删除一个节点，返回删除后的根节点
static rbt_node_p __rbt_delete_rebalance(rbt_node_p node, rbt_node_p parent, rbt_node_p root);	// 红黑树删除节点后重新平衡

static void __it_push(set_it_p it, rbt_node_p node);	// 迭代用的压栈函数
static rbt_node_p __it_pop(set_it_p it);		// 迭代用的弹栈函数
static int __it_stack_empty(set_it_p it);		// 迭代用的空栈判断函数

static set_it_p __set_iterator(set_p s, int dir);	// 生成一个迭代器
static rbt_node_p __set_it_next_node(set_it_p it);	// 中序迭代一个迭代器

static Element __set_it_next(void *it);			// Iterator的next函数
static size_t __set_it_remove(void *it);		// Iterator的remove函数，直接返回NULL
static void __set_it_reset(void *it);			// Iterator的reset函数
static void __set_it_destroy(void *it);			// Iterator的destroy函数

static void __set_clone(set_p dest, set_p src);		// 将集合src复制一份到dest中
static void __rbt_clone(set_p dest, rbt_node_p src);	// 二叉树复制，采用先序遍历的顺序复制，插入新节点的开销最小

Container set_create(ElementType type, CmpFunc cmpfunc) {
	Container cont = NULL;
	set_p set = NULL;
	if ((set = (set_p)malloc(sizeof(set_t))) && (cont = (Container)malloc(sizeof(Container_t)))) {
		set->type = type;
		set->root = NULL;
		set->size = 0;
		set->cmpfunc = cmpfunc ? cmpfunc : __default_cmpfunc(type);
		set->changes = 0;
		pthread_mutex_init(&set->mut, NULL);
		cont->container = set;
		cont->type = Set;
	} else {
		free(set);
		free(cont);
		cont = NULL;
	}
	return cont;
}

int set_destroy(Container set)
{
	int ret = -1;
	if (IS_VALID_SET(set)) {
		set_p s = (set_p)set->container;
		pthread_mutex_lock(&s->mut);
		__rbt_removeall(s->root);
		pthread_mutex_unlock(&s->mut);
		pthread_mutex_destroy(&s->mut);
		free(s);
		free(set);
		ret = 0;
	}
	return ret;
}

int set_isempty(Container set)
{
	return IS_VALID_SET(set) ? ((set_p)set->container)->root == NULL : 1;
}

size_t set_size(Container set)
{
	return IS_VALID_SET(set) ? ((set_p)set->container)->size : 0;
}

int set_contains(Container set, Element element, ElementType type, size_t len)
{
	int ret = 0;
	element_p e = NULL;
	if (IS_VALID_SET(set) && element && len && ((set_p)set->container)->type == type && (e = __element_create(element, type, len))) {
		set_p s = (set_p)set->container;
		pthread_mutex_lock(&s->mut);
		rbt_node_p result = __rbt_search(e, s->root, s->cmpfunc);
		ret = result ? 1 : 0;
		__element_destroy(e);
		pthread_mutex_unlock(&s->mut);
	}
	return ret;
}

int set_add(Container set, Element element, ElementType type, size_t len)
{
	int ret = -1;
	element_p e = NULL;
	if (IS_VALID_SET(set) && element && len && ((set_p)set->container)->type == type && (e = __element_create(element, type, len))) {
		set_p s = (set_p)set->container;
		pthread_mutex_lock(&s->mut);
		rbt_node_p r = __rbt_insert(e, s->root, s->cmpfunc);
		if (r) {
			// 插入时如果元素重复或者发生错误插入失败则返回NULL，否则返回插入后的红黑树的新根节点
			s->root = r;
			s->size++;
			s->changes++;
			ret = 0;
		} else {
			// 插入失败时把生成的元素副本销毁
			__element_destroy(e);
		}
		pthread_mutex_unlock(&s->mut);
	}
	return ret;
}

size_t set_remove(Container set, Element element, ElementType type, size_t len)
{
	size_t ret = 0;
	element_p e = NULL;
	if (IS_VALID_SET(set) && element && len && ((set_p)set->container)->type == type && (e = __element_create(element, type, len))) {
		set_p s = (set_p)set->container;
		pthread_mutex_lock(&s->mut);
		rbt_node_p node = __rbt_search(e, s->root, s->cmpfunc);
		if (node != NULL) {		// 找到要删除的元素
			ret = 1;
			s->root = __rbt_delete(node, s->root);
			s->size--;
			s->changes++;
		}
		__element_destroy(e);
		pthread_mutex_unlock(&s->mut);
	}
	return ret;
}

void set_removeall(Container set)
{
	if (IS_VALID_SET(set)) {
		set_p s = (set_p)set->container;
		pthread_mutex_lock(&s->mut);
		__rbt_removeall(s->root);
		s->size = 0;
		s->root = NULL;
		s->changes++;
		pthread_mutex_unlock(&s->mut);
	}
}

Iterator set_iterator(Container set, int dir)
{
	set_it_p it = NULL;
	if (IS_VALID_SET(set)) {
		set_p s = (set_p)set->container;
		pthread_mutex_lock(&s->mut);
		it = __set_iterator(s, dir);
		pthread_mutex_unlock(&s->mut);
	}
	return it ? it_create(it, __set_it_next, __set_it_remove, __set_it_reset, __set_it_destroy) : NULL;
}

Container set_intersection(Container s1, Container s2)
{
	Container ret = NULL;
	if (s1 != s2) {
		if (IS_VALID_SET(s1) && IS_VALID_SET(s2)) {	// 没有无效容器，进行交集运算，否则直接返回空容器
			set_p set1 = (set_p)s1->container;
			set_p set2 = (set_p)s2->container;
			set_p set = (set_p)malloc(sizeof(set_t));
			ret = (Container)malloc(sizeof(Container_t));
			if (!set || !ret) {	// 内存不足，返回空容器
				free(set);
				free(ret);
				return NULL;
			}
			set->type = set1->type;
			set->root = NULL;
			set->size = 0;
			set->changes = 0;
			set->cmpfunc = set1->cmpfunc;
			pthread_mutex_init(&set->mut, NULL);
			ret->container = set;
			ret->type = Set;
			pthread_mutex_lock(&set1->mut);
			pthread_mutex_lock(&set2->mut);
			if (set1->type == set2->type && (set1->size * set2->size) > 0) {
				// 两个集合数据类型一致，且两个集合都有数据时进行交集运算，否则返回空集合
				set_it_p it1 = __set_iterator(set1, Forward);
				set_it_p it2 = __set_iterator(set2, Forward);
				rbt_node_p n1 = __set_it_next_node(it1);
				rbt_node_p n2 = __set_it_next_node(it2);
				while (n1 && n2) {			// 只要有一个集合已经取完所有数据，那么交集就结束了
					int cmp = set->cmpfunc(n1->element->value, n2->element->value, n1->element->len, n2->element->len);
					if (cmp < 0) {			// 集合1中的当前元素比较小，取下一个，继续循环
						n1 = __set_it_next_node(it1);
					} else if (cmp > 0) {		// 集合2中的当前元素比较小，取下一个，继续循环
						n2 = __set_it_next_node(it2);
					} else {			// 两个集合的当前元素相等，添加到结果集中，两个集合都取下一个，继续循环
						element_p e = __element_create(n1->element->value, n1->element->type, n1->element->len);
						if (!e) {		// 复制元素出错，内存不足，返回空容器
							__rbt_removeall(set->root);
							free(set);
							free(ret);
							return NULL;
						}
						rbt_node_p root = __rbt_insert(e, set->root, set->cmpfunc);
						if (root) {
							set->root = root;
							set->size++;
						}
						n1 = __set_it_next_node(it1);
						n2 = __set_it_next_node(it2);
					}
				}
				__set_it_destroy(it1);
				__set_it_destroy(it2);
			}
			pthread_mutex_unlock(&set1->mut);
			pthread_mutex_unlock(&set2->mut);
		}
	} else {		// 自己交集自己，返回自己的clone
		if (IS_VALID_SET(s1)) {		// 无效容器时直接返回NULL
			set_p set1 = (set_p)s1->container;
			set_p set = (set_p)malloc(sizeof(set_t));
			ret = (Container)malloc(sizeof(Container_t));
			if (!set || !ret) {	// 内存不足，返回空容器
				free(set);
				free(ret);
				return NULL;
			}
			set->type = set1->type;
			set->root = NULL;
			set->size = 0;
			set->changes = 0;
			set->cmpfunc = set1->cmpfunc;
			pthread_mutex_init(&set->mut, NULL);
			ret->container = set;
			ret->type = Set;
			pthread_mutex_lock(&set1->mut);
			__set_clone(set, set1);
			pthread_mutex_unlock(&(set1->mut));
		}
	}
	return ret;
}

Container set_union(Container s1, Container s2)
{
	Container ret = NULL;
	if (s1 != s2) {
		if (IS_VALID_SET(s1) && IS_VALID_SET(s2)) {	// 如果有一个容器非法，那么直接返回NULL容器
			set_p set1 = (set_p)s1->container;
			set_p set2 = (set_p)s2->container;
			set_p set = (set_p)malloc(sizeof(set_t));
			ret = (Container)malloc(sizeof(Container_t));
			if (!set || !ret) {
				free(set);
				free(ret);
				return NULL;
			}
			set->type = set1->type;
			set->root = NULL;
			set->size = 0;
			set->changes = 0;
			set->cmpfunc = set1->cmpfunc;
			pthread_mutex_init(&set->mut, NULL);
			ret->container = set;
			ret->type = Set;
			pthread_mutex_lock(&set1->mut);
			pthread_mutex_lock(&set2->mut);
			if (set1->type == set2->type && (set1->size + set2->size) > 0) {
				// 两个集合数据类型一致，且至少有一个集合有数据时进行合并，否则直接返回空集合
				set_it_p it = NULL;
				if (set1->size > set2->size) {		// set1比较大，复制set1再逐个添加set2中的元素
					__set_clone(set, set1);
					it = (set2->root ? __set_iterator(set2, Forward) : NULL);
				} else {				// 反之
					__set_clone(set, set2);
					it = (set1->root ? __set_iterator(set1, Forward) : NULL);
				}
				if (it) {				// 另一个集合中有元素，则添加另一个集合的所有元素
					rbt_node_p node = NULL;
					while ((node = __set_it_next_node(it))) {
						element_p e = __element_create(node->element->value, node->element->type, node->element->len);
						if (!e) {		// 复制元素出错，内存不足，返回空容器
							__rbt_removeall(set->root);
							free(set);
							free(ret);
							return NULL;
						}
						rbt_node_p root = __rbt_insert(e, set->root, set->cmpfunc);
						if (root) {
							set->root = root;
							set->size++;
						}
					}
					__set_it_destroy(it);
				}
			}
			pthread_mutex_unlock(&set1->mut);
			pthread_mutex_unlock(&set2->mut);
		}
	} else {		// 自己并集自己，返回自己的clone
		if (IS_VALID_SET(s1)) {		// 非法集合的话直接返回NULL容器
			set_p set1 = (set_p)s1->container;
			set_p set = (set_p)malloc(sizeof(set_t));
			ret = (Container)malloc(sizeof(Container_t));
			if (!set || !ret) {	// 内存不足，返回空容器
				free(set);
				free(ret);
				return NULL;
			}
			set->type = set1->type;
			set->root = NULL;
			set->size = 0;
			set->changes = 0;
			set->cmpfunc = set1->cmpfunc;
			pthread_mutex_init(&set->mut, NULL);
			ret->container = set;
			ret->type = Set;
			pthread_mutex_lock(&set1->mut);
			__set_clone(set, set1);
			pthread_mutex_unlock(&(set1->mut));
		}
	}
	return ret;
}

Container set_minus(Container s1, Container s2)
{
	Container ret = NULL;
	if (s1 != s2) {
		if (IS_VALID_SET(s1) && IS_VALID_SET(s2)) {	// 如果有非法容器，那么直接返回空容器
			set_p set1 = (set_p)s1->container;
			set_p set2 = (set_p)s2->container;
			set_p set = (set_p)malloc(sizeof(set_t));
			ret = (Container)malloc(sizeof(Container_t));
			if (!set || !ret) {	// 内存不足，返回空容器
				free(set);
				free(ret);
				return NULL;
			}
			set->type = set1->type;
			set->root = NULL;
			set->size = 0;
			set->changes = 0;
			set->cmpfunc = set1->cmpfunc;
			pthread_mutex_init(&set->mut, NULL);
			ret->container = set;
			ret->type = Set;
			pthread_mutex_lock(&set1->mut);
			pthread_mutex_lock(&set2->mut);
			if (set1->type == set2->type && set1->size > 0) {	// 两个集合数据类型一致，且被减集合有数据
				set_it_p it1 = __set_iterator(set1, Forward);
				set_it_p it2 = __set_iterator(set2, Forward);
				rbt_node_p n1 = __set_it_next_node(it1);
				rbt_node_p n2 = __set_it_next_node(it2);
				rbt_node_p root = NULL;
				while (n1 && n2) {			// set1结束则循环结束，set2结束则循环结束后把set1剩余的数据全部添加到结果集中
					int cmp = set->cmpfunc(n1->element->value, n2->element->value, n1->element->len, n2->element->len);
					if (cmp < 0) {			// 集合1中的当前元素比较小，复制并跳到下一个元素，继续循环
						element_p e = __element_create(n1->element->value, n1->element->type, n1->element->len);
						if (!e) {		// 复制元素出错，内存不足，返回空容器
							__rbt_removeall(set->root);
							free(set);
							free(ret);
							return NULL;
						}
						root = __rbt_insert(e, set->root, set->cmpfunc);
						if (root) {
							set->root = root;
							set->size++;
						}
						n1 = __set_it_next_node(it1);
					} else if (cmp > 0) {		// 集合2中的当前元素比较小，取下一个，继续循环
						n2 = __set_it_next_node(it2);
					} else {			// 两个集合的当前元素相等，两个集合都取下一个，继续循环
						n1 = __set_it_next_node(it1);
						n2 = __set_it_next_node(it2);
					}
				}
				while (n1) {				// 集合1中还有元素，全部复制到结果集中去
					element_p e = __element_create(n1->element->value, n1->element->type, n1->element->len);
					if (!e) {		// 复制元素出错，内存不足，返回空容器
						__rbt_removeall(set->root);
						free(set);
						free(ret);
						return NULL;
					}
					root = __rbt_insert(e, set->root, set->cmpfunc);
					if (root) {
						set->root = root;
						set->size++;
					}
					n1 = __set_it_next_node(it1);
				}
				__set_it_destroy(it1);
				__set_it_destroy(it2);
			}
			pthread_mutex_unlock(&set1->mut);
			pthread_mutex_unlock(&set2->mut);
		}
	} else {			// 自己减自己，返回一个空集
		if (IS_VALID_SET(s1)) {
			set_p set1 = (set_p)s1->container;
			set_p set = (set_p)malloc(sizeof(set_t));
			ret = (Container)malloc(sizeof(Container_t));
			if (!set || !ret) {	// 内存不足，返回空容器
				free(set);
				free(ret);
				return NULL;
			}
			set->type = set1->type;
			set->root = NULL;
			set->size = 0;
			set->changes = 0;
			set->cmpfunc = set1->cmpfunc;
			pthread_mutex_init(&set->mut, NULL);
			ret->container = set;
			ret->type = Set;
		}
	}
	return ret;
}

/**
 * 创建一个新的节点
 */
static rbt_node_p __rbt_new_node(element_p element)
{
	rbt_node_p nnode = (rbt_node_p)malloc(sizeof(rbt_node_t));
	if (nnode) {
		nnode->element = element;
		nnode->left = NULL;
		nnode->right = NULL;
		nnode->parent = NULL;
		nnode->color = Red;
	}
	return nnode;
}

/**
 * 销毁一个节点及其中的元素
 */
static void __rbt_destroy_node(rbt_node_p node)
{
	__element_destroy(node->element);
	free(node);
}

/**
 * 销毁从root开始的所有节点及其中的元素
 */
static void __rbt_removeall(rbt_node_p root)
{
	if (root) {
		__rbt_removeall(root->left);
		__rbt_removeall(root->right);
		__rbt_destroy_node(root);
	}
}

/**
 * 从root开始搜索指定元素所在节点的辅助函数，如果指定元素没有找到，可以通过save保存插入点
 * 算法描述：
 * ITERATIVE-TREE-SEARCH(x, k)
 * 1	while x != NIL and k != key[x]
 * 2		do if k < key[x]
 * 3			then x := left[x]
 * 4			else x := right[x]
 * 5	return x
 */
static rbt_node_p __rbt_search_aux(element_p ele, rbt_node_p root, CmpFunc cmpfunc, rbt_node_p *save)
{
	rbt_node_p ret = root, parent = NULL;
	int cmp;
	while (ret != NULL && (cmp = cmpfunc(ele->value, ret->element->value, ele->len, ret->element->len)) != 0) {
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

/**
 * 从root开始查找元素与ele相等的节点并返回，找不到返回NULL，调用__rbt_search_aux()实现
 */
static rbt_node_p __rbt_search(element_p ele, rbt_node_p root, CmpFunc cmpfunc)
{
	return __rbt_search_aux(ele, root, cmpfunc, NULL);
}

/**
 * 以node节点为轴左旋，返回旋转后的根节点
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
static rbt_node_p __rbt_rotate_left(rbt_node_p node, rbt_node_p root)
{
	rbt_node_p rnode = node->right;			// 1
	if ((node->right = rnode->left))		// 2,3
		rnode->left->parent = node;		// 4
	if ((rnode->parent = node->parent))		// 5,6
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
 * 以node节点为轴右旋，返回旋转后的根节点
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
static rbt_node_p __rbt_rotate_right(rbt_node_p node, rbt_node_p root)
{
	rbt_node_p lnode = node->left;
	if ((node->left = lnode->right))
		lnode->right->parent = node;
	if ((lnode->parent = node->parent))
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
static rbt_node_p __rbt_insert(element_p ele, rbt_node_p root, CmpFunc cmpfunc)
{
	rbt_node_p parent = NULL, node;
	if ((node = __rbt_search_aux(ele, root, cmpfunc, &parent)))	// 寻找插入点，如果相等的元素已经存在则返回原节点，否则返回NULL并在parent中存放插入点
		return NULL;
	node = __rbt_new_node(ele);
	if (!node)	// 创建节点失败，直接返回NULL
		return NULL;
	node->parent = parent;     
	if (parent)	// 插入点非空，即原树不为空
		if (cmpfunc(parent->element->value, ele->value, parent->element->len, ele->len) > 0)
			parent->left = node;
		else
			parent->right = node;
	else		// 原树为空树，新节点即为根节点
		root = node;
	// 至此二叉查找树的节点插入结束，接下来进行再平衡，修复红黑树的性质并返回根节点
	return __rbt_insert_rebalance(node, root);
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
static rbt_node_p __rbt_insert_rebalance(rbt_node_p node, rbt_node_p root)
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
 * 寻找中序后继节点的算法描述：
 * TREE-SUCCESSOR(x)
 * 1	if right[x] != nil[T]
 * 2		then return TREE-MINIMUN(right[x])
 * 3	y := p[x]
 * 4	while y != nil[T] and x == right[y]
 * 5		do x := y
 * 6		   y := p[y]
 * 7	return y
 *--------------------------------------------------------------
 * 寻找树中最小节点的算法描述：
 * TREE-MINIMUN(x)
 * 1	while left[x] != nil[T]
 * 2		do x := left[x]
 * 3	return x
 *--------------------------------------------------------------
 * 说明：由于删除节点时仅在符合条件left[z] != nil[T] and right[z] != nil[T]时才去z的中序后继作为替身，所以可以明确这里的TREE-SUCCESSOR算法可以简化为TREE-MINIMUN(right[z])
 */
static rbt_node_p __rbt_delete(rbt_node_p node, rbt_node_p root)
{
	rbt_node_p remove = node, dnode, parent;
	RBT_Color color;

	if (node->left && node->right) {				// 1, 2, 3
		remove = node->right;
		while (remove->left)
			remove = remove->left;
	}
	color = remove->color;
	parent = remove->parent;					// dnode为哨兵节点时，其自身无法传递parent参数，用此变量保存并在修正时作为参数
	if (remove->left)						// 4, 5, 6
		dnode = remove->left;
	else
		dnode = remove->right;
	if (dnode)							// 7
		dnode->parent = remove->parent;
	if (remove->parent)						// 8, 9, 10, 11, 12, remove的子树（唯一或没有）绕过remove接到remove->parent的相应子节点上
		if (remove->parent->left == remove)
			remove->parent->left = dnode;
		else
			remove->parent->right = dnode;
	else
		root = dnode;						// 经过4-12步，如果remove既没有parent又没有子树，那么说明remove是树里最后一个节点，此时root==dnode==NULL
	__element_destroy(node->element);
	if (remove != node)						// 13, 14, 15, 如果是用了中序后继作为替身，那么把替身中的元素复制到要被删除的节点node中去
		node->element = remove->element;
	free(remove);
	if (color == Black)
		root = __rbt_delete_rebalance(dnode, parent, root);
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
static rbt_node_p __rbt_delete_rebalance(rbt_node_p node, rbt_node_p parent, rbt_node_p root)
{
	rbt_node_p sibling;
	while ((!node || node->color == Black) && node != root) {
		if (parent->left == node) {			// 进入循环时，node == root的条件保证了parent不会为NULL
			sibling = parent->right;
			if (sibling->color == Red) {		//情况1：兄弟节点是红色的
				sibling->color = Black;
				parent->color = Red;
				root = __rbt_rotate_left(parent, root);
				sibling = parent->right;
			}
			if ((!sibling->left || sibling->left->color == Black) && (!sibling->right || sibling->right->color == Black)) {		// 情况2：兄弟和兄弟的两个子节点都是黑色的
				sibling->color = Red;
				node = parent;
				parent = node->parent;	// 修正parent变量，保证进入循环时变量正确
			} else {
				if (!sibling->right || sibling->right->color == Black) {	//情况3：兄弟是黑色的，且兄弟的左子节点是红色，右子节点是黑色
					if (sibling->left)
						sibling->left->color = Black;
					sibling->color = Red;
					root = __rbt_rotate_right(sibling, root);
					sibling = parent->right;
				}
				//情况4：兄弟是黑色的，且兄弟的右子节点是红色的，左子节点无所谓，这种情况只能且必然在情况3修复后出现
				sibling->color = parent->color;
				parent->color = Black;
				if (sibling->right)
					sibling->right->color = Black;
				root = __rbt_rotate_left(parent, root);
				node = root;
			}
		} else {
			sibling = parent->left;
			if (sibling->color == Red) {
				sibling->color = Black;
				parent->color = Red;
				root = __rbt_rotate_right(parent, root);
				sibling = parent->left;
			}
			if ((!sibling->left || sibling->left->color == Black) && (!sibling->right || sibling->right->color == Black)) {  
				sibling->color = Red;
				node = parent;
				parent = node->parent;
			} else {
				if (!sibling->left || sibling->left->color == Black) {
					if (sibling->right)
						sibling->right->color = Black;
					sibling->color = Red;
					root = __rbt_rotate_left(sibling, root);
					sibling = parent->left;
				}
				sibling->color = parent->color;
				parent->color = Black;
				if (sibling->left)
					sibling->left->color = Black;
				root = __rbt_rotate_right(parent, root);
				node = root;
			}
		}
	}
	if (node)
		node->color = Black;
	return root;
}

static void __it_push(set_it_p it, rbt_node_p node)
{
	*(it->top++) = node;
}

static rbt_node_p __it_pop(set_it_p it)
{
	return *(--it->top);
}

static int __it_stack_empty(set_it_p it)
{
	return it->stack == it->top;
}

static set_it_p __set_iterator(set_p set, int dir)
{
	set_it_p ret = (set_it_p)malloc(sizeof(set_it_t));
	unsigned int len = lg2(set->size + 1);
	len = len << 1;			// 红黑树最大树高度小于2*lg2(size+1)
	rbt_node_p *stack = (rbt_node_p *)malloc(len * sizeof(rbt_node_p));
	if (ret && stack) {
		ret->asc = dir;
		ret->set = set;
		ret->stack = stack;
		ret->top = ret->stack;
		ret->changes = set->changes;
		rbt_node_p current = set->root;
		while (current != NULL) {
			__it_push(ret, current);
			current = ret->asc ? current->left : current->right;
		}
	} else {
		free(ret);
		free(stack);
		ret = NULL;
	}
	return ret;
}

static rbt_node_p __set_it_next_node(set_it_p it)
{
	rbt_node_p ret = NULL;
	if (it && it->set) {
		if (it->changes != it->set->changes)	// 迭代时集合变更，迭代结束，返回NULL
			it->top = it->stack;
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
	}
	return ret;
}

/**
 * 重置迭代器
 */
static void __set_it_reset(void *it)
{
	if (it && ((set_it_p)it)->set) {
		set_it_p iterator = (set_it_p)it;
		set_p set = iterator->set;
		unsigned int len = lg2(set->size + 1);
		len = len << 1;			// 红黑树最大树高度小于2*lg2(size+1)
		rbt_node_p *stack = (rbt_node_p *)malloc(len * sizeof(rbt_node_p));
		if (stack) {
			free(iterator->stack);
			iterator->stack = stack;
			iterator->top = iterator->stack;
			pthread_mutex_lock(&set->mut);
			iterator->changes = set->changes;
			rbt_node_p current = set->root;
			while (current != NULL) {
				__it_push(iterator, current);
				current = iterator->asc ? current->left : current->right;
			}
			pthread_mutex_unlock(&set->mut);
		}
	}
}

/**
 * 迭代访问一个迭代器中的下一个元素
 *
 * it
 *	集合迭代器的指针
 *
 * return
 * 	迭代中的下一个元素，迭代器无效或已经迭代完成时返回NULL
 */
static Element __set_it_next(void *it)
{
	rbt_node_p node = NULL;
	if (it && ((set_it_p)it)->set) {
		set_p set = ((set_it_p)it)->set;
		pthread_mutex_lock(&set->mut);
		node = __set_it_next_node(it);
		pthread_mutex_unlock(&set->mut);
	}
	return node ? __element_clone_value(node->element) : NULL;
}

/**
 * 迭代器删除元素，集合迭代器不支持删除元素，直接返回0
 */
static size_t __set_it_remove(void *it)
{
	return 0;
}

/**
 * 迭代器销毁
 */
static void __set_it_destroy(void *it)
{
	if (it) {
		free(((set_it_p)it)->stack);
		free(it);
	}
}

/**
 * 复制集合，复制时元素只是复制了指针而没有复制内容
 */
static void __set_clone(set_p dest, set_p src)
{
	__rbt_clone(dest, src->root);
}

/**
 * 复制二叉树，采用先序遍历顺序复制，插入新节点开销最小
 */
static void __rbt_clone(set_p dest, rbt_node_p src)
{
	if (!src)
		return;
	element_p e = __element_create(src->element->value, src->element->type, src->element->len);
	if (!e)
		return;
	rbt_node_p root = __rbt_insert(e, dest->root, dest->cmpfunc);
	if (root) {
		dest->root = root;
		dest->size++;
	}
	__rbt_clone(dest, src->left);
	__rbt_clone(dest, src->right);
	return;
}
