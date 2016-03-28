#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mr_list.h>

Container al = NULL;
Iterator fwd = NULL;
Iterator bwd = NULL;

char *nations[] = {
	"中华人民共和国",
	"中华民国",
	"Japan",
	"South Korea",
	"United States of America",
	"United Kingdom",
	"South Africa",
	"German",
	"France",
	"Australia",
	"Canada",
	"Argentina",
	"Brazil",
	"Hong Kong",
	"Singapore",
	"Thailand"
};

void show(void)
{
	size_t s;
	int i;
	Element e;
	printf("读取列表元素，列表%s，元素数量 = %zu\n", list_isempty(al) ? "为空" : "不空", (s = list_size(al)));
	printf("使用位置索引随机访问元素：\n");
	for (i = 0; i < s; i++) {
		e = list_get(al, i);
		printf("%2d) \"%s\"\n", i, POINTOF(e, char));		// String类型的元素不能用VALUEOF()宏
		free(e);						// 获取的元素值用完必须free
	}
	printf("Ok!\n");
	printf("使用正向迭代器迭代访问元素：\n");
	it_reset(fwd);
	i = 0;
	while ((e = it_next(fwd))) {
		printf("%2d) \"%s\"\n", i++, (char *)e);
		free(e);
	}
	printf("Ok!\n");
	printf("使用反向迭代器迭代访问元素：\n");
	it_reset(bwd);
	i = list_size(al);
	while ((e = it_next(bwd))) {
		printf("%2d) \"%s\"\n", --i, POINTOF(e, char));
		free(e);
	}
	printf("Ok!\n");
}

void type(void)
{
	size_t s;
	int i;
	Element e;
	printf("读取列表元素，列表%s，元素数量 = %zu\n", list_isempty(al) ? "为空" : "不空", (s = list_size(al)));
	printf("使用位置索引随机访问元素：\n");
	for (i = 0; i < s; i++) {
		e = list_get(al, i);
		printf("%2d) \"%s\"\n", i, POINTOF(e, char));		// String类型的元素不能用VALUEOF()宏
		free(e);						// 获取的元素值用完必须free
	}
	printf("Ok!\n");
}

void cont(void)
{
	printf("press <enter> to continue...");
	getchar();
}
void appends(int s)
{
	int os = list_size(al);
	int i, j;
	for (i = os; i < s; i++) {
		j = rand() % 16;
		list_append(al, nations[j], string, strlen(nations[j]));
	}
}

void inserts(int s, int p)
{
	int os = list_size(al);
	int i, j;
	for (i = os; i < s; i++) {
		j = rand() % 16;
		list_insert(al, p, nations[j], string, strlen(nations[j]));
	}
}

int main(void)
{
	printf("初始化ArrayList及其正反两个方向的迭代器...");
	al = list_create(string, ArrayList, NULL);
	fwd = list_iterator(al, Forward);
	bwd = list_iterator(al, Reverse);
	printf("Ok!\n");
	printf("TEST1：空列表时的查询\n");
	show();
	cont();
	printf("TEST2: 用append添加一个元素\n");
	appends(1);
	show();
	cont();
	printf("TEST3: 用remove_at删除一个元素\n");
	printf("删除了%zu个元素\n", list_remove_at(al, 0));
	show();
	cont();
	printf("TEST4: 用insert从头部开始连续添加18个元素\n");
	inserts(18, 0);
	show();
	int pos = 0;
	int from = -1;
	while ((pos = list_search(al, from, Forward, "South Korea", string, 11)) != -1) {
		printf("正向搜索所有韩国: %d\n", pos);
		from = pos + 1;
	}
	from = -1;
	while ((pos = list_search(al, from, Reverse, "Brazil", string, 6)) != -1) {
		printf("反向搜索所有巴西: %d\n", pos);
		from = pos - 1;
	}
	cont();
	printf("TEST5: 用remove删除所有Brazil\n");
	list_remove(al, "Brazil", string, 6);
	show();
	cont();
	printf("TEST6: 用removeall删除所有元素\n");
	list_removeall(al);
	show();
	cont();
	printf("TEST7: 用push连续添加12个元素后进行递增快速排序\n");
	int i, j;
	for (i = 0; i < 12; i++) {
		j = rand() % 16;
		list_push(al, nations[j], string, strlen(nations[j]));
	}
	printf("排序前:\n");
	type();
	printf("排序后:\n");
	list_qsort(al, Asc);
	type();
	printf("二分搜索找韩国: %d\n", list_bi_search(al, "South Korea", string, strlen("South Korea")));
	printf("二分搜索找中国: %d\n", list_bi_search(al, "中华人民共和国", string, strlen("中华人民共和国")));
	cont();
	printf("TEST8: 用enqueue连续添加12个元素后进行递减插入排序\n");
	for (i = 0; i < 12; i++) {
		j = rand() % 16;
		list_enqueue(al, nations[j], string, strlen(nations[j]));
	}
	printf("排序前:\n");
	type();
	printf("排序后:\n");
	list_isort(al, Desc);
	type();
	printf("二分搜索找日本: %d\n", list_bi_search(al, "Japan", string, 5));
	printf("二分搜索找台湾: %d\n", list_bi_search(al, "中华民国", string, strlen("中华民国")));
	printf("反向排列所有元素\n");
	list_reverse(al);
	type();
	printf("二分搜索找韩国: %d\n", list_bi_search(al, "South Korea", string, strlen("South Korea")));
	printf("二分搜索找中国: %d\n", list_bi_search(al, "中华人民共和国", string, strlen("中华人民共和国")));
	printf("二分搜索找日本: %d\n", list_bi_search(al, "Japan", string, 5));
	printf("二分搜索找台湾: %d\n", list_bi_search(al, "中华民国", string, strlen("中华民国")));
	cont();
	printf("TEST9: 用迭代器迭代删除所有元素\n");
	Iterator delit = list_iterator(al, Forward);
	Element ele = NULL;
	while ((ele = it_next(delit))) {
		it_remove(delit);
		printf("删除元素：\"%s\"\n", POINTOF(ele, char));
		free(ele);
	}
	type();
	cont();
	printf("TEST10: 模拟堆栈\n");
	printf("连续PUSH三次:\n");
	j = rand() % 16;
	list_push(al, nations[j], string, strlen(nations[j]));
	type();
	j = rand() % 16;
	list_push(al, nations[j], string, strlen(nations[j]));
	type();
	j = rand() % 16;
	list_push(al, nations[j], string, strlen(nations[j]));
	type();
	printf("用stacktop读取栈顶元素:");
	ele = list_stacktop(al);
	printf(" \"%s\"\n", POINTOF(ele, char));
	free(ele);
	printf("用pop弹空堆栈:\n");
	ele = list_pop(al);
	printf("\"%s\"\n", POINTOF(ele, char));
	free(ele);
	ele = list_pop(al);
	printf("\"%s\"\n", POINTOF(ele, char));
	free(ele);
	ele = list_pop(al);
	printf("\"%s\"\n", POINTOF(ele, char));
	free(ele);
	type();
	cont();
	printf("TEST11: 模拟队列\n");
	printf("连续enqueue三次:\n");
	j = rand() % 16;
	list_enqueue(al, nations[j], string, strlen(nations[j]));
	type();
	j = rand() % 16;
	list_enqueue(al, nations[j], string, strlen(nations[j]));
	type();
	j = rand() % 16;
	list_enqueue(al, nations[j], string, strlen(nations[j]));
	type();
	printf("用queuehead读取栈顶元素:");
	ele = list_queuehead(al);
	printf(" \"%s\"\n", POINTOF(ele, char));
	free(ele);
	printf("用dequeue全部出队:\n");
	ele = list_dequeue(al);
	printf("\"%s\"\n", POINTOF(ele, char));
	free(ele);
	ele = list_dequeue(al);
	printf("\"%s\"\n", POINTOF(ele, char));
	free(ele);
	ele = list_dequeue(al);
	printf("\"%s\"\n", POINTOF(ele, char));
	free(ele);
	type();
	cont();
	printf("TEST12: 两个列表相加\n");
	Container list2 = list_create(string, ArrayList, NULL);
	appends(9);
	printf("原列表:\n");
	type();
	printf("加上一个空列表:\n");
	list_plus(al, list2);
	type();
	printf("加上一个有9个元素的列表:\n");
	for (i = 0; i < 9; i++) {
		j = rand() % 16;
		list_append(list2, nations[j], string, strlen(nations[j]));
	}
	list_plus(al, list2);
	type();
	cont();
	printf("再减去这个列表:\n");
	list_minus(al, list2);
	type();
	printf("再减去一个空列表:\n");
	Container empty = list_create(string, ArrayList, NULL);
	list_minus(al, empty);
	type();
	cont();
	printf("添加到18个元素后再进行retain操作，类似取交集\n");
	appends(18);
	printf("原列表:\n");
	type();
	printf("list2:\n");
	for (i = 0; i < 9; i++) {
		ele = list_get(list2, i);
		printf("%s, ", POINTOF(ele, char));
		free(ele);
	}
	printf("\n");
	list_retain(al, list2);
	printf("retain后:\n");
	type();
	printf("retain一个空列表:\n");
	list_retain(al, empty);
	type();
	printf("FIN: 销毁列表和迭代器...");
	it_destroy(fwd);
	it_destroy(bwd);
	list_destroy(al);
	list_destroy(list2);
	list_destroy(empty);
	printf("Ok!\n");
	return 0;
}
