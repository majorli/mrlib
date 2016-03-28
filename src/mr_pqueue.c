#include <stdlib.h>
#include <pthread.h>

#include "mr_pqueue.h"
#include "private_element.h"

#define IS_VALID_PQ(X) (X && X->container && X->type == PriorityQueue)

#define PQ_INIT_CAPA 10
#define PQ_NEXT_CAPA(CC) ((CC) * 3 / 2 + 1)

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
	unsigned int changes;
	pthread_mutex_t mut;
} pq_t, *pq_p;

/**
 * @brief 优先级队列迭代器
 */
typedef struct {
	pq_p queue;
	int pos;
	unsigned int changes;
} pq_it_t, *pq_it_p;

static pq_node_p __pq_node_create(Element ele, ElementType type, size_t len, int priority);	// 创建一个节点
static void __pq_node_destroy(pq_node_p node);							// 销毁一个节点

static void __pq_removeall(pq_p pq);								// 清空所有节点

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
	q->changes = 0;
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
		q->changes++;
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
	return -1;
}

Element pq_dequeue(Container pq)
{
	return NULL;
}

Element pq_queuehead(Container pq)
{
	return NULL;
}

int pq_contains(Container pq, Element ele, ElementType type, size_t len)
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
		q->changes++;
		pthread_mutex_unlock(&q->mut);
	}
	return ret;
}

Iterator pq_iterator(Container pq)
{
	return NULL;
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

