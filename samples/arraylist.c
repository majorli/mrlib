#include <stdio.h>

#include <mr_arraylist.h>

void show_list(ArrayList list)
{
	Element e = NULL;
	size_t i = 0;
	printf("list.size = %zu, list.isempty = %d\n", al_size(list), al_isempty(list));
	while ((e = al_get(list, i)))
		printf("E[%zu] = %d, ", i++, *((int *)e));
	printf("\n");
}

int main(void)
{
	int x[80];
	for (int i = 0; i < 80; i++)
		x[i] = i - 40;
	int *p = x;

	printf("建立一个整数列表，采用默认的整数比较函数\n");
	ArrayList list = al_create(Int, NULL);
	printf("list handler = %d\n", list);

	printf("输出列表：\n");
	show_list(list);

	printf("添加测试数组中的第一个数到列表中：\n");
	al_append(list, p);
	show_list(list);

	printf("逐个添加测试数组中的第2个数开始的39个数到列表中：\n");
	for (int i = 0; i < 39; i++)
		al_append(list, ++p);
	show_list(list);

	printf("从第0个位置开始每隔一个位置插入一个测试数组中的数，顺序插入后续40个数：\n");
	size_t insp = 0;
	for (int i = 0; i < 40; i++) {
		al_add(list, ++p, insp);
		insp += 2;
	}
	show_list(list);

	printf("对列表进行排序：\n");
	al_sort(list);
	show_list(list);

	printf("销毁列表，因为元素是基本数据类型，所以不需要销毁元素，否则需要先销毁所有其中的元素\n");
	al_destroy(list);

	return 0;
}
