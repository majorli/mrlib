#include <stdlib.h>
#include <pthread.h>

#include "mr_pqueue.h"
#include "private_element.h"

#define IS_VALID_PQ(X) (X && X->container && X->type == PriorityQueue)

#define PQ_INIT_CAPA 10
#define PQ_NEXT_CAPA(CC) ((CC) * 3 / 2 + 1)

#define PQ_PARENT(x) (((x) - 1) / 2)
#define PQ_LEFT(x) (((x) * 2) + 1)
#define PQ_RIGHT(x) (((x) + 1) * 2)

/**
 * @brief 队列节点结构
 */
typedef struct {
	int priority;
	element_p element;
} pq_node_t, *pq_node_p;

/**
 * @brief 优先级队列结构
 */
typedef struct {
	pq_node_p *queue;
	ElementType etype;
	PriorityType ptype;
	CmpFunc cmpfunc;
	size_t capacity;
	size_t size;
	pthread_mutex_t mut;
} pq_t, *pq_p;

static pq_node_p __pq_node_create(Element ele, ElementType type, size_t len, int priority);	// 创建一个节点
static void __pq_node_destroy(pq_node_p node);							// 销毁一个节点

static void __pq_removeall(pq_p pq);								// 清空所有节点

static void __pq_bubble_up(pq_p pq, int pos);							// pos位置的节点上浮
static void __pq_bubble_dn(pq_p pq, int pos);							// pos位置的节点下沉

static int __pq_expand(pq_p pq);								// 扩展队列的容量

Container pq_create(PriorityType ptype, ElementType etype, CmpFunc cmpfunc)
{
	Container pq = (Container)malloc(sizeof(Container_t));
	if (!pq)
		return NULL;
	pq_p q = (pq_p)malloc(sizeof(pq_t));
	if (!q) {
		free(pq);
		return NULL;
	}
	pq_node_p *queue = (pq_node_p *)malloc(PQ_INIT_CAPA * sizeof(pq_node_t));
	if (!queue) {
		free(pq);
		free(q);
		return NULL;
	}
	q->queue = queue;
	q->etype = etype;
	q->ptype = ptype;
	q->cmpfunc = cmpfunc ? cmpfunc : __default_cmpfunc(etype);
	q->capacity = PQ_INIT_CAPA;
	q->size = 0;
	pthread_mutex_init(&q->mut, NULL);
	pq->container = q;
	pq->type = PriorityQueue;
	return pq;
}

int pq_destroy(Container pq)
{
	int ret = -1;
	if (IS_VALID_PQ(pq)) {
		pq_p q = (pq_p)pq->container;
		pthread_mutex_lock(&q->mut);
		__pq_removeall(q);
		free(q->queue);
		q->size = 0;
		pthread_mutex_unlock(&q->mut);
		pthread_mutex_destroy(&q->mut);
		free(q);
		free(pq);
		ret = 0;
	}
	return ret;
}

int pq_isempty(Container pq)
{
	return IS_VALID_PQ(pq) ? ((pq_p)pq->container)->size == 0 : 1;
}

size_t pq_size(Container pq)
{
	return IS_VALID_PQ(pq) ? ((pq_p)pq->container)->size : 0;
}

int pq_enqueue(Container pq, Element ele, ElementType type, size_t len, int priority)
{
	int pos = -1;
	pq_node_p node = NULL;
	if (IS_VALID_PQ(pq) && ((pq_p)pq->container)->etype == type && (node = __pq_node_create(ele, type, len, priority))) {
		pq_p q = (pq_p)pq->container;
		pthread_mutex_lock(&q->mut);
		int pos;
		if (q->size < q->capacity || __pq_expand(q) == 0) {
			pos = q->size;
			q->queue[pos] = node;
			q->size++;
			__pq_bubble_up(q, pos);
		} else {
			__pq_node_destroy(node);
		}
		pthread_mutex_unlock(&q->mut);
	}
	return pos;
}

Element pq_dequeue(Container pq, int *priority)
{
	Element e = NULL;
	int p = -1;
	if (IS_VALID_PQ(pq) && ((pq_p)pq->container)->size > 0) {
		pq_p q = (pq_p)pq->container;
		pthread_mutex_lock(&q->mut);
		if ((e = __element_clone_value(q->queue[0]->element))) {
			p = q->queue[0]->priority;
			__pq_node_destroy(q->queue[0]);
			q->size--;
			if (q->size > 0) {
				q->queue[0] = q->queue[q->size];
				__pq_bubble_dn(q, 0);
			}
		}
		pthread_mutex_unlock(&q->mut);
	}
	if (priority)
		*priority = p; 
	return e;
}

Element pq_queuehead(Container pq, int *priority)
{
	Element e = NULL;
	int p = -1;
	if (IS_VALID_PQ(pq) && ((pq_p)pq->container)->size > 0) {
		pq_p q = (pq_p)pq->container;
		pthread_mutex_lock(&q->mut);
		if ((e = __element_clone_value(q->queue[0]->element))
			p = q->queue[0]->priority;
		pthread_mutex_unlock(&q->mut);
	}
	if (priority)
		*priority = p;
	return e;
}

int pq_contains(Container pq, Element ele, ElementType type, size_t len)
{
	return -1;
}

int pq_search(Container pq, Element ele, ElementType type, size_t len)
{
	return -1;
}

int pq_change_pri_at(Container pq, int index, int priority)
{
	return -1;
}

int pq_change_pri(Container pq, Element ele, ElementType type, size_t len, int priority)
{
	return -1;
}

int pq_removeall(Container pq)
{
	int ret = 0;
	if (IS_VALID_PQ(pq)) {
		pq_p q = (pq_p)pq->container;
		pthread_mutex_lock(&q->mut);
		ret = q->size;
		__pq_removeall(q);
		q->size = 0;
		pthread_mutex_unlock(&q->mut);
	}
	return ret;
}

/**
 * @brief 创建一个节点
 *
 * @param ele
 * 	元素值
 * @param type
 * 	元素类型
 * @param len
 * 	元素长度
 * @param priority
 * 	优先级
 *
 * @return 
 * 	创建成功返回节点，失败返回NULL
 */
static pq_node_p __pq_node_create(Element ele, ElementType type, size_t len, int priority)
{
	element_p e = __element_create(ele, type, len);
	if (!e)
		return NULL;
	pq_node_p node = (pq_node_p)malloc(sizeof(pq_node_t));
	if (!node) {
		__element_destroy(e);
		return NULL;
	}
	node->element = e;
	node->priority = priority;
	return node;
}

/**
 * @brief 销毁一个节点，同时销毁其中的元素
 *
 * @param node
 * 	节点
 */
static void __pq_node_destroy(pq_node_p node)
{
	__element_destroy(node->element);
	free(node);
}

/**
 * @brief 删除并销毁所有节点
 *
 * @param pq
 * 	优先级队列容器
 */
static void __pq_removeall(pq_p pq)
{
	int i;
	for (i = 0; i < pq->size; i++)
		__pq_node_destroy(pq->queue[i]);
}

/**
 * @brief 节点上浮
 *
 * @param pq
 * 	优先级队列
 * @param pos
 * 	要上浮的节点位置
 */
static void __pq_bubble_up(pq_p pq, int pos)
{
	if (pos < 0 || pos >= pq->size)
		return;
	pq_node_p key = pq->queue[pos];
	int slot = pos;
	int parent;
	while (slot > 0) {
		parent = PQ_PARENT(slot);
		if (pq->ptype == Min_Priority ? key->priority >= pq->queue[parent]->priority : key->priority <= pq->queue[parent]->priority)
			break;
		pq->queue[slot] = pq->queue[parent];
		slot = parent;
	}
	pq->queue[slot] = key;
}

/**
 * @brief 节点下沉
 *
 * @param pq
 * 	优先级队列
 * @param pos
 * 	要下沉的节点位置
 */
static void __pq_bubble_dn(pq_p pq, int pos)
{
	if (pos < 0 || pos >= pq->size)
		return;
	pq_node_p key = pq->queue[pos];
	int slot = pos;
	int left, right, child;
	while ((left = PQ_LEFT(slot)) < pq->size) {
		right = PQ_RIGHT(slot);
		child = right >= pq->size ? left : (pq->ptype == Min_Priority ? pq->queue[right]->priority < pq->queue[left]->priority : pq->queue[right]->priority > pq->queue[left]->priority) ? right : left;
		if (pq->ptype == Min_Priority ? key->priority < pq->queue[child]->priority : key->priority > pq->queue[child]->priority)
			break;
		pq->queue[slot] = pq->queue[child];
		slot = child;
	}
	pq->queue[slot] = key;
}

/**
 * @brief 扩展队列的容量
 *
 * @param pq
 * 	优先级队列
 *
 * @return 
 * 	扩容成功返回0，失败返回-1
 */
static int __pq_expand(pq_p pq)
{
	int ret = -1;
	size_t oc = pq->capacity;
	size_t nc = PQ_NEXT_CAPA(oc);
	pq_node_p *nl = (pq_node_p *)realloc(pq->queue, nc * sizeof(pq_node_p));
	if (nl) {
		pq->queue = nl;
		pq->capacity = nc;
		ret = 0;
	}
	return ret;
}

