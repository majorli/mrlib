#include <stdio.h>
#include <stdlib.h>

#include <mr_pool.h>

#define PSTAT(P) printf("池状态：是否空池 = %c，元素数量 = %zu，使用率 = %f%%\n", pool_isempty(P) ? 'Y' : 'N', pool_size(P), pool_ratio(P));

int main(void)
{
	int handlers[20];

	// 创建一个初始容量为5的池，但由于池的容量下限为10，所以实际上创建的池初始容量将是10
	Container pool = pool_create(5);
	PSTAT(pool);

	// 向池中托管元素，连续托管直到容量不足
	int i = 0;
	while ((handlers[i] = pool_retrieve(pool, &i, integer, sizeof(int))) != -1)
		i++;
	printf("托管第%d个元素时发生错误\n", i);
	PSTAT(pool);

	// 通过句柄从池中取得托管的元素
	for (int j = 0; j < i; j += 2) {
		Element temp = pool_get(pool, handlers[j]);
		printf("通过句柄%d 获得元素%d\n", handlers[j], *(int *)temp);
		free(temp);
	}

	// 释放池中句柄对应的节点
	for (int j = 1; j < i; j+= 2) {
		Element temp = pool_release(pool, handlers[j]);
		printf("释放了句柄%d, 释放出的元素为%d\n", handlers[j], VALUEOF(temp, int));
		free(temp);
	}
	PSTAT(pool);

	// 已经释放的句柄再去获取元素或再次释放将返回错误的结果
	printf("尝试通过已经释放的句柄3获取元素，返回值为%p\n", pool_get(pool, 3));
	printf("尝试再次释放已经释放的句柄5，返回值为%p\n", pool_release(pool, 5));
	PSTAT(pool);

	// 把池里的元素清空
	for (int j = 0; j < i; j += 2) {
		Element temp = pool_release(pool, handlers[j]);
		printf("释放了句柄%d, 释放出的元素为%d\n", handlers[j], VALUEOF(temp, int));
		free(temp);
	}
	PSTAT(pool);

	// 继续添加10个元素
	printf("继续添加元素\n");
	i = 0;
	while ((handlers[i] = pool_retrieve(pool, &i, integer, sizeof(int))) != -1)
		i++;
	printf("托管第%d个元素时发生错误\n", i);
	PSTAT(pool);
	printf("扩展池的容量后继续添加\n");
	if (pool_expand(pool) == -1)
		printf("扩展池容量失败\n");
	else
		PSTAT(pool);
	while ((handlers[i] = pool_retrieve(pool, &i, integer, sizeof(int))) != -1)
		i++;
	printf("托管第%d个元素时发生错误，扩展池的容量后继续添加\n", i);
	PSTAT(pool);

	// 每隔2个释放一个句柄
	for (int j = 0; j < i; j+=3) {
		free(pool_release(pool, handlers[j]));
		printf("释放了句柄%d, ", handlers[j]);
		PSTAT(pool);
	}

	// 尝试缩小
	printf("尝试缩小\n");
	pool_shrink(pool);
	PSTAT(pool);
	
	// 再扩展
	printf("再扩展\n");
	pool_expand(pool);
	PSTAT(pool);
	pool_expand(pool);
	PSTAT(pool);

	// 再尝试缩小
	printf("再尝试缩小\n");
	pool_shrink(pool);
	PSTAT(pool);
	
	// 清除所有元素
	pool_removeall(pool);
	// 销毁池，销毁池的时候会自动销毁其中所有元素，不需要事先removeall
	pool_destroy(pool);
	// 如果pool这个变量还要继续使用的话，最好在destroy之后强制设置为NULL
	pool = NULL;

	return 0;
}
