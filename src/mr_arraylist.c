#include <stdlib.h>

#include "mr_common.h"
#include "mr_containers.h"

typedef struct {							// ArrayList结构
	Element *elements;						// 元素存储区域
	size_t capacity;						// 当前列表总容量
	size_t size;							// 当前列表中元素数量
	int (*CmpFunc)(void *, void *);					// 元素比较函数
} al_t, *al_p;

static const size_t SECTION_SIZE = 50;					// 列表每一节的长度
