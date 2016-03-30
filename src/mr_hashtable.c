#include <stdlib.h>
#include <pthread.h>

#include "mr_hashtable.h"
#include "private_element.h"

#define IS_VALID_HT(X) (X && X->container && X->type == HashTable)

static unsigned long crypt_table[0x500];
static char ct_ready = 0;
static const long CAPACITIES[] = {
	17,		37,		79,		163,		331,
	673,		1361,		2729,		5471,		10949,
	21911,		43853,		87719,		175447,		350899,
	701819,		1403641,	2807303,	5614657,	11229331,
	22458671,	44917381,	89834777,	179669557,	359339171,
	718678369,	1437356741,	2147483647
};
static const int CAPA_COUNT = 28;

static const int HASH_OFFSET = 0, HASH_A = 1, HASH_B = 2;

typedef struct {
	unsigned long hash[3];
	element_p element;
} ht_node_t, *ht_node_p;

typedef struct {
	ht_node_p *table;
	int capa_idx;
	long size;
	long changes;
	pthread_mutex_t mut;
} ht_t, *ht_p;

typedef struct {
	ht_p ht;
	long changes;
	long it_pos;
} ht_it_t, *ht_it_p;

static void __ht_prepare_crypt_table(void);							// 准备哈希函数所需的数据
static void __ht_hashcodes(element_p element, unsigned long *hashcodes);			// Hash函数

static int __ht_expand(ht_p ht);								// 哈希表容量扩展

static void __ht_node_destroy(ht_node_p node);							// 销毁节点以及其中的元素
static void __ht_removeall(ht_p ht);								// 销毁哈希表中所有节点及其中的元素
static ht_node_p __ht_node_create(element_p ele);						// 创建一个哈希表节点
static long __ht_index(ht_node_p node, long capacity, ht_node_p *table);			// 计算表中存放位置

static ht_it_p __ht_iterator(ht_p ht);								// 创建一个迭代器
static Element __ht_it_next(void *it);								// 迭代获取下一个元素
static size_t __ht_it_remove(void *it);								// 删除上一次迭代的元素
static void __ht_it_reset(void *it);								// 重置迭代器
static void __ht_it_destroy(void *it);								// 销毁迭代器

Container hash_create(void)
{
	Container cont = (Container)malloc(sizeof(Container_t));
	if (!cont)
		return NULL;
	ht_p ht = (ht_p)malloc(sizeof(ht_t));
	if (!ht) {
		free(cont);
		return NULL;
	}
	ht_node_p *table = (ht_node_p *)malloc(CAPACITIES[0] * sizeof(ht_node_p));
	memset(table, 0, CAPACITIES[0] * sizeof(ht_node_p));
	if (!table) {
		free(ht);
		free(cont);
		return NULL;
	}
	ht->table = table;
	ht->capa_idx = 0;
	ht->size = 0;
	ht->changes = 0;
	pthread_mutex_init(&ht->mut, NULL);
	cont->container = ht;
	cont->type = HashTable;
	if (!ct_ready) {
		__ht_prepare_crypt_table();
		ct_ready = 1;
	}
	return cont;
}

int hash_destroy(Container hash)
{
	if (IS_VALID_HT(hash)) {
		ht_p ht = (ht_p)hash->container;
		pthread_mutex_lock(&ht->mut);
		__ht_removeall(ht);
		free(ht->table);
		pthread_mutex_unlock(&ht->mut);
		pthread_mutex_destroy(&ht->mut);
		free(ht);
		free(hash);
		return 0;
	}
	return -1;
}

int hash_isempty(Container hash)
{
	if (IS_VALID_HT(hash))
		return ((ht_p)hash->container)->size == 0;
	else
		return 1;
}

size_t hash_size(Container hash)
{
	if (IS_VALID_HT(hash))
		return ((ht_p)hash->container)->size;
	else
		return 0;
}

int hash_register(Container hash, Element ele, ElementType type, size_t len)
{
	int ret = -1;
	element_p e;
	if (IS_VALID_HT(hash) && ele && len > 0 && (e = __element_create(ele, type, len))) {
		ht_p ht = (ht_p)hash->container;
		pthread_mutex_lock(&ht->mut);
		if (ht->size < CAPACITIES[ht->capa_idx] || __ht_expand(ht) == 0) {
			ht_node_p node = __ht_node_create(e);
			if (!node) {
				__element_destroy(e);
				return -1;
			}
			long pos = __ht_index(node, CAPACITIES[ht->capa_idx], ht->table);	// 因为事先扩容，所以不会返回返回-1
			if (ht->table[pos]) {							// 检查是不是已经有相同元素存在
				__ht_node_destroy(node);
				return -1;
			}
			ht->table[pos] = node;
			ht->size++;
			ht->changes++;
			ret = 0;
		}
		pthread_mutex_unlock(&ht->mut);
	}
	return ret;
}

int hash_contains(Container hash, Element ele, ElementType type, size_t len)
{
	int ret = 0;
	element_p e;
	if (IS_VALID_HT(hash) && ele && len > 0 && (e = __element_create(ele, type, len))) {
		ht_p ht = (ht_p)hash->container;
		pthread_mutex_lock(&ht->mut);
		ht_node_p node = __ht_node_create(e);
		if (!node) {
			__element_destroy(e);
			return 0;
		}
		long pos = __ht_index(node, CAPACITIES[ht->capa_idx], ht->table);
		if (pos != -1 && ht->table[pos])
			ret = 1;
		__ht_node_destroy(node);
		pthread_mutex_unlock(&ht->mut);
	}
	return ret;
}

int hash_remove(Container hash, Element ele, ElementType type, size_t len)
{
	int ret = 0;
	element_p e;
	if (IS_VALID_HT(hash) && ele && len > 0 && (e = __element_create(ele, type, len))) {
		ht_p ht = (ht_p)hash->container;
		pthread_mutex_lock(&ht->mut);
		ht_node_p node = __ht_node_create(e);
		if (!node) {
			__element_destroy(e);
			return 0;
		}
		long pos = __ht_index(node, CAPACITIES[ht->capa_idx], ht->table);
		if (pos != -1 && ht->table[pos]) {
			__ht_node_destroy(ht->table[pos]);
			ht->table[pos] = NULL;
			ht->size--;
			ht->changes++;
			ret = 1;
		}
		__ht_node_destroy(node);
		pthread_mutex_unlock(&ht->mut);
	}
	return ret;
}

int hash_removeall(Container hash)
{
	if (IS_VALID_HT(hash)) {
		ht_p ht = (ht_p)hash->container;
		pthread_mutex_lock(&ht->mut);
		__ht_removeall(ht);					// 在这个函数里已经修改了size和changes了
		pthread_mutex_unlock(&ht->mut);
		return 0;
	}
	return -1;
}

Iterator hash_iterator(Container hash)
{
	ht_it_p it = NULL;
	if (IS_VALID_HT(hash)) {
		ht_p ht = (ht_p)hash->container;
		pthread_mutex_lock(&ht->mut);
		it = __ht_iterator(ht);
		pthread_mutex_unlock(&ht->mut);
	}
	return it ? it_create(it, __ht_it_next, __ht_it_remove, __ht_it_reset, __ht_it_destroy) : NULL;
}

/**
 * @brief 生成一个长度为0x500的crypt_table[0x500]
 */
static void __ht_prepare_crypt_table(void)
{ 
	unsigned long seed = 0x00100001, index1 = 0, index2 = 0, i;
	for(index1 = 0; index1 < 0x100; index1++) {
		for(index2 = index1, i = 0; i < 5; i++, index2 += 0x100) {
			unsigned long temp1, temp2;
			seed = (seed * 125 + 3) % 0x2AAAAB;
			temp1 = (seed & 0xFFFF) << 0x10;
			seed = (seed * 125 + 3) % 0x2AAAAB;
			temp2 = (seed & 0xFFFF);
			crypt_table[index2] = (temp1 | temp2);
		}
	}
}

/**
 * @brief 计算元素Hash值
 *
 * @param element
 * 	元素
 * @param hashcodes
 * 	计算结果，三个哈希值，依次为HASH_OFFSET, HASH_A, HASH_B，调用前需确保该参数为一个长度至少为3的unsigned long数组
 */
static void __ht_hashcodes(element_p element, unsigned long *hashcodes)
{ 
	unsigned char *key  = (unsigned char *)element->value;
	unsigned long seed11 = 0x7FED7FED;
	unsigned long seed21 = 0xEEEEEEEE;
	unsigned long seed12 = 0x7FED7FED;
	unsigned long seed22 = 0xEEEEEEEE;
	unsigned long seed13 = 0x7FED7FED;
	unsigned long seed23 = 0xEEEEEEEE;
	int ch;
	int len = element->len;
	while (len-- > 0) {
		ch = *key++;
		seed11 = crypt_table[(HASH_OFFSET << 8) + ch] ^ (seed11 + seed21);
		seed21 = ch + seed11 + seed21 + (seed21 << 5) + 3;
		seed12 = crypt_table[(HASH_A << 8) + ch] ^ (seed12 + seed22);
		seed22 = ch + seed12 + seed22 + (seed22 << 5) + 3;
		seed13 = crypt_table[(HASH_B << 8) + ch] ^ (seed13 + seed23);
		seed23 = ch + seed13 + seed23 + (seed23 << 5) + 3;
	}
	hashcodes[HASH_OFFSET] = seed11;
	hashcodes[HASH_A] = seed12;
	hashcodes[HASH_B] = seed13;
}

/**
 * @brief 扩充哈希表容量
 *
 * @param ht
 * 	哈希表
 *
 * @return 
 * 	扩充成功返回0，失败或已经无法扩展返回-1
 */
static int __ht_expand(ht_p ht)
{
	long oc = CAPACITIES[ht->capa_idx];
	if (oc == CAPA_COUNT - 1)
		return -1;
	long nc = CAPACITIES[ht->capa_idx + 1];
	ht_node_p *ntable = (ht_node_p *)malloc(nc * sizeof(ht_node_p));
	if (!ntable)
		return -1;
	memset(ntable, 0, nc * sizeof(ht_node_p));
	long i;
	long np;
	ht_node_p node;
	for (i = 0; i < oc; i++) {
		node = ht->table[i];
		if (node) {
			np = __ht_index(node, nc, ntable);		// 可以断言，这里np不会返回-1
			ntable[np] = node;
		}
	}
	free(ht->table);
	ht->table = ntable;
	ht->capa_idx++;
	return 0;
}

/**
 * @brief 生成一个哈希表节点
 *
 * @param ele
 * 	节点中的元素
 *
 * @return 
 * 	生成的节点，失败时返回NULL
 */
static ht_node_p __ht_node_create(element_p ele)
{
	ht_node_p node = (ht_node_p)malloc(sizeof(ht_node_t));
	if (node) {
		node->element = ele;
		__ht_hashcodes(ele, node->hash);
	}
	return node;
}

/**
 * @brief 计算节点在表中的存放位置，采用线性探测法解决冲突，用于插入元素时对返回值要进行存在性检查，如果已经存在则无需插入
 *
 * @param node
 * 	节点
 * @param capacity
 * 	表容量
 * @param table
 * 	表
 *
 * @return 
 * 	存放位置，找不到存放位置时返回-1
 */
static long __ht_index(ht_node_p node, long capacity, ht_node_p *table)
{
	long start = node->hash[HASH_OFFSET] % capacity;
	long pos = start;
	while (table[pos]) {
		if (table[pos]->element->type == node->element->type &&
				table[pos]->element->len == node->element->len &&
				table[pos]->hash[HASH_A] == node->hash[HASH_A] &&
				table[pos]->hash[HASH_B] == node->hash[HASH_B])
			return pos;			// 这个元素已经在表中有了，直接返回已有元素的位置
		pos = (pos + 1) % capacity;		// 向后探测
		if (pos == start)
			return -1;
	}
	return pos;
}

/**
 * @brief 销毁节点及其中的元素
 *
 * @param node
 * 	待销毁的节点
 */
static void __ht_node_destroy(ht_node_p node)
{
	if (!node)
		return;
	__element_destroy(node->element);
	free(node);
}

/**
 * @brief 清空所有节点，销毁其中元素
 *
 * @param ht
 * 	哈希表
 */
static void __ht_removeall(ht_p ht)
{
	long i = CAPACITIES[ht->capa_idx];
	while (--i >= 0)
		__ht_node_destroy(ht->table[i]);
	memset(ht->table, 0, CAPACITIES[ht->capa_idx] * sizeof(ht_node_p));
	ht->size = 0;
	ht->changes++;
}

static ht_it_p __ht_iterator(ht_p ht)
{
	ht_it_p it = (ht_it_p)malloc(sizeof(ht_it_t));
	if (!it)
		return NULL;
	it->ht = ht;
	it->changes = ht->changes;
	it->it_pos = -1;
	return it;
}

static Element __ht_it_next(void *it)
{
	Element ret = NULL;
	if (it && ((ht_it_p)it)->ht) {
		ht_it_p iterator = (ht_it_p)it;
		ht_p ht = iterator->ht;
		long capa = CAPACITIES[ht->capa_idx];
		if (ht->changes != iterator->changes)
			iterator->it_pos = capa;
		while (++iterator->it_pos < capa)
			if (ht->table[iterator->it_pos]) {
				ret = __element_clone_value(ht->table[iterator->it_pos]->element);
				break;
			}
	}
	return ret;
}

static size_t __ht_it_remove(void *it)
{
	size_t ret = 0;
	if (it && ((ht_it_p)it)->ht) {
		ht_it_p iterator = (ht_it_p)it;
		ht_p ht = iterator->ht;
		long capa = CAPACITIES[ht->capa_idx];
		if (ht->changes != iterator->changes)
			iterator->it_pos = capa;
		if (iterator->it_pos >= 0 && iterator->it_pos < capa) {
			__ht_node_destroy(ht->table[iterator->it_pos]);
			ht->table[iterator->it_pos] = NULL;
			ht->size--;
			ht->changes++;
			iterator->changes++;
			ret = 1;
		}
	}
	return ret;
}

static void __ht_it_reset(void *it)
{
	if (it && ((ht_it_p)it)->ht) {
		ht_it_p iterator = (ht_it_p)it;
		iterator->it_pos = -1;
		iterator->changes = iterator->ht->changes;
	}
}

static void __ht_it_destroy(void *it)
{
	free(it);
}

