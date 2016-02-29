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

int intcmpr(const void *d1, const void *d2)
{
	return -intcmp(d1, d2);
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

	printf("对列表进行逆序排序，采用插入排序算法：\n");
	al_comparator(list, intcmpr);
	al_stsort(list);
	al_comparator(list, NULL);
	show_list(list);

	printf("对列表进行快速排序：\n");
	al_sort(list);
	show_list(list);

	printf("删除第一个元素：\n");
	al_remove(list, 0);
	show_list(list);

	printf("删除最后一个元素：\n");
	al_remove(list, al_size(list) - 1);
	show_list(list);

	printf("从第10个元素开始连续删除10个元素：\n");
	for (int i = 0; i < 10; i++)
		al_remove(list, 9);
	show_list(list);

	printf("再连续添加测试数组中的10个数：\n");
	for (int i = 0; i < 10; i++)
		al_append(list, x + (i << 3));
	show_list(list);
	
	printf("再次快速排序：\n");
	al_sort(list);
	show_list(list);

	printf("清空列表：\n");
	al_clear(list);
	show_list(list);

	printf("空列表或只有一个元素的列表进行排序，不会有任何动作执行，直接返回：\n");
	al_stsort(list);
	show_list(list);
	printf("添加一个元素并排序，使用al_add()方法添加在超限的位置，会自动添加到最后：\n");
	al_add(list, x, 1);
	al_sort(list);
	show_list(list);

	printf("添加所有测试数据：\n");
	for (int i = 0; i < 80; i++)
		al_append(list, x + i);
	show_list(list);

	printf("把前10个元素替换成测试数组中的最后10个元素：\n");
	for (int i = 0; i < 10; i++)
		al_replace(list, x + 70 + i, i);
	show_list(list);

	printf("\n销毁列表，因为元素是基本数据类型，所以不需要销毁元素，否则需要先销毁所有其中的元素\n");
	al_destroy(list);

	return 0;
}
