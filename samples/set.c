#include <stdio.h>
#include <stdlib.h>

#include <mr_set.h>

void show_set(Container set)
{
	Iterator it = set_iterator(set, Forward);
	Element e;
	printf("ISEMPTY = %d, SIZE = %zu：\n", set_isempty(set), set_size(set));
	int count = 0;
	while ((e = it_next(it))) {
		printf("%2d, ", VALUEOF(e,int));
		if ((count++) % 10 == 9)
			printf("\n");
	}
	printf("\n");
	it_destroy(it);
}

void show_set_r(Container set)
{
	Iterator it = set_iterator(set, Reverse);
	Element e;
	printf("ISEMPTY = %d, SIZE = %zu：\n", set_isempty(set), set_size(set));
	int count = 0;
	while ((e = it_next(it))) {
		printf("%2d, ", VALUEOF(e, int));
		if ((count++) % 10 == 9)
			printf("\n");
	}
	printf("\n");
	it_destroy(it);
}

int main(void)
{
	int elements[50];
	printf("生成随机整数50个：\n");
	for (int i = 0; i < 50; i++) {
		int r;
		printf("%2d, ", elements[i] = rand() % 50);
		if (i % 10 == 9)
			printf("\n");
	}

	printf("创建一个集合：\n");
	Container set = set_create(integer, NULL);
	show_set(set);

	printf("添加一个元素：\n");
	set_add(set, elements, integer, sizeof(int));
	show_set(set);

	printf("删除这个元素：\n");
	set_remove(set, elements, integer, sizeof(int));
	show_set(set);

	printf("连续添加20个元素，数值重复的不会被添加：\n");
	for (int i = 0; i < 20; i++)
		set_add(set, elements + i, integer, sizeof(int));
	show_set(set);

	printf("用后10个随机数连续10次查找：\n");
	for (int i = 20; i < 30; i++)
		printf("查找数字%d，集合中%s该数字\n", elements[i], set_contains(set, elements + i, integer, sizeof(int)) ? "存在" : "没有");

	printf("用最后20个随机数逐个从集合中尝试删除元素：\n");
	for (int i = 30; i < 50; i++)
		printf("删除数字%d，集合中%s\n", elements[i], set_remove(set, elements + i, integer, sizeof(int)) ? "存在该数字，删除对应元素" : "没有该数字，跳过");
	show_set(set);

	printf("递减顺序迭代集合元素：\n");
	show_set_r(set);

	printf("用removeall()函数清空所有元素：\n");
	set_removeall(set);
	show_set(set);

	printf("添加最后20个元素：\n");
	for (int i = 30; i < 50; i++)
		set_add(set, elements + i, integer, sizeof(int));
	show_set(set);

	printf("用remove()逐个删除所有元素：\n");
	for (int i = 0; i < 50; i++)
		set_remove(set, elements + i, integer, sizeof(int));
	show_set(set);

	Container empty = set_create(integer, NULL);

	printf("集合运算：先创建两个集合，分别存放25个数字\n");
	Container set2 = set_create(integer, NULL);
	for (int i = 0; i < 25; i++) {
		set_add(set, elements + i, integer, sizeof(int));
		set_add(set2, elements + i + 25, integer, sizeof(int));
	}
	printf("集合1：\n");
	show_set(set);
	printf("集合2：\n");
	show_set(set2);
	printf("并集：\n");
	Container result = set_union(set, set2);
	show_set(result);
	set_destroy(result);

	printf("自己并自己：\n");
	result = set_union(set, set);
	show_set(result);
	set_destroy(result);

	printf("并空集：\n");
	result = set_union(set, empty);
	show_set(result);
	set_destroy(result);

	printf("交集：\n");
	result = set_intersection(set, set2);
	show_set(result);
	set_destroy(result);
	
	printf("自己交自己：\n");
	result = set_intersection(set, set);
	show_set(result);
	set_destroy(result);

	printf("交空集：\n");
	result = set_intersection(set, empty);
	show_set(result);
	set_destroy(result);

	printf("减集(set1 - set2)：\n");
	result = set_minus(set, set2);
	show_set(result);
	set_destroy(result);

	printf("减集(set2 - set1)：\n");
	result = set_minus(set2, set);
	show_set(result);
	set_destroy(result);

	printf("减集(set1 - set1)：\n");
	result = set_minus(set, set);
	show_set(result);
	set_destroy(result);

	printf("减空集(set1 - empty)：\n");
	result = set_minus(set, empty);
	show_set(result);
	set_destroy(result);

	printf("空集减(empty - set2)：\n");
	result = set_minus(empty, set2);
	show_set(result);
	set_destroy(result);

	printf("结束，销毁集合\n");
	set_destroy(set);
	set_destroy(set2);
	set_destroy(empty);
	return 0;
}
