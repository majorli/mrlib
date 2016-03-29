#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>

#include "mr_hashtable.h"
#include "private_element.h"

#define IS_VALID_HT(X) (X && X->container && X->type == HashTable)

static unsigned long cryptTable[0x500];
static const size_t capacities[] = {
	17,		37,		79,		163,		331,
	673,		1361,		2729,		5471,		10949,
	21911,		43853,		87719,		175447,		350899,
	701819,		1403641,	2807303,	5614657,	11229331,
	22458671,	44917381,	89834777,	179669557,	359339171,
	718678369,	1437356741,	2147483647
};
static const int capa_count = 28;
static const int HASH_OFFSET = 0, HASH_A = 1, HASH_B = 2;

typedef struct {
	unsigned long hash_a;
	unsigned long hash_b;
	element_p element;
} ht_node_t, *ht_node_p;

typedef struct {
	ht_node_p *table;
	int capa_idx;
	size_t size;
	pthread_mutex_t mut;
} ht_t, *ht_p;

static int __element_isequal(element_p e1, element_p e2);					// 两个元素是否完全相同
static void __ht_prepare_crypt_table(void);							// 准备哈希函数所需的数据
static unsigned long __ht_hashcode(element_p element, unsigned long type);			// Hash函数

Container hash_create(void)
{
	return NULL;
}


int hash_destroy(Container hash)
{
	return -1;
}


int hash_register(Container hash, Element ele, ElementType type, size_t len)
{
	return -1;
}


int hash_contains(Container hash, Element ele, ElementType type, size_t len)
{
	return -1;
}


int hash_remove(Container hash, Element ele, ElementType type, size_t len)
{
	return -1;
}


int hash_removeall(Container hash)
{
	return -1;
}

Iterator hash_iterator(Container hash)
{
	return NULL;
}

/**
 * @brief 判断两个元素是否完全相同，即长度、数据类型、每个字节都相同
 *
 * @param e1
 * 	元素1
 * @param e2
 * 	元素2
 *
 * @return 
 * 	相同返回1，不相同返回0
 */
static int __element_isequal(element_p e1, element_p e2)
{
	return 0;
}

/**
 * @brief 生成一个长度为0x500的cryptTable[0x500]
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
			cryptTable[index2] = (temp1 | temp2);
		}
	}
}

/**
 * @brief 计算元素Hash值
 *
 * @param element
 * 	元素
 * @param type
 * 	哈希类型，HASH_OFFSET, HASH_A, HASH_B
 *
 * @return 
 * 	哈希值
 */
static unsigned long __ht_hashcode(element_p element, unsigned long type)
{ 
	unsigned char *key  = (unsigned char *)element->value;
	unsigned long seed1 = 0x7FED7FED;
	unsigned long seed2 = 0xEEEEEEEE;
	int ch;
	int len = element->len;
	while (len-- > 0) {
		ch = toupper(*key++);
		seed1 = cryptTable[(dwHashType << 8) + ch] ^ (seed1 + seed2);
		seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
	}
	return seed1; 
}
