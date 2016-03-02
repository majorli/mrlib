#include <stdio.h>
#include <stdlib.h>

#include <mr_linkedlist.h>

void show_list(LinkedList list)
{
	size_t s = ll_size(list);
	printf("list.size = %zu, list.isempty = %d\n", s, ll_isempty(list));
	LLNode h = ll_head(list);
	LLNode t = ll_tail(list);
	printf("HEAD = %p, TAIL = %p\n", h, t);
	LLNode p = h;
	while (p != NULL) {
		LLNode next = ll_next(p);
		Element ele = ll_get(p);
		printf("NODE = %p, ELE_P = %p, ELEMENT = %f, PREV = %p, NEXT = %p\n", p, ele, *((double *)ele), ll_prev(p), next);
		p = next;
	}
}

int doublecmpr(const void *d1, const void *d2)
{
	return -doublecmp(d1, d2);
}

int main(void)
{
	double PI = 3.1415926535;
	double E = 2.7182818285;
	Element x[7];
	x[0] = double_inbox(PI);
	x[1] = double_inbox(E);
	x[2] = double_inbox(1.41421356237);
	x[3] = double_inbox(1.73205080757);
	x[4] = double_inbox(2.2360679775);
	x[5] = double_inbox(0.0);
	x[6] = double_inbox(1.0);
	printf("测试用元素：\n");
	for (int i = 0; i < 7; i++)
		printf("x[%d]: %f(%p)\n", i, *((double *)x[i]), x[i]);
	printf("*** 开始测试 ***\n");

	LinkedList list = ll_create(Double, NULL);
	printf("创建一个存放double类型数据的链表，句柄list = %d：\n", list);
	show_list(list);

	printf("用append或prepend可以添加第一个元素：\n");
	// ll_append(list, x[0]);
	ll_prepend(list, x[0]);
	show_list(list);
	printf("用append在表尾后添加一个元素：%p\n", ll_append(list, x[1]));
	show_list(list);

	printf("用prepend在表头前添加一个元素：%p\n", ll_prepend(list, x[2]));
	show_list(list);

	LLNode pos = ll_head(list);
	printf("用insert_before在表头前添加一个元素：\n");
	pos = ll_insert_before(pos, x[3]);	// 返回添加后的节点，所以直接赋值给pos，pos将指向新的表头
	show_list(list);

	printf("用insert_after在表头后添加一个元素：\n");
	ll_insert_after(pos, x[4]);
	show_list(list);

	pos = ll_tail(list);
	printf("用insert_after在表尾后添加一个元素：\n");
	pos = ll_insert_after(pos, x[5]);	// 返回添加后的节点，所以直接赋值给pos，pos将指向新的表尾
	show_list(list);

	pos = ll_tail(list);
	printf("用insert_before在表尾前添加一个元素：\n");
	ll_insert_before(pos, x[6]);
	show_list(list);

	pos = ll_head(list);
	printf("用remove删除表头节点，注意传入参数的数据类型为LLNode*，remove执行后返回被删除的元素(Element)而不是节点(LLNode)，可以用来free元素：\n");
	ll_remove(&pos);			// 删除一个节点后该节点指针会被置为NULL
	printf("注意：remove一个节点后该节点指针将被置空：pos = %p\n", pos);		// pos此时已经变成NULL，再次用来调用任何函数都不会有任何作用
	show_list(list);

	pos = ll_tail(list);
	printf("用remove删除表尾节点：\n");
	ll_remove(&pos);
	show_list(list);

	pos = ll_next(ll_next(ll_head(list)));
	printf("用remove删除一个中间节点：\n");
	ll_remove(&pos);
	show_list(list);

	pos = ll_head(list);
	printf("用remove依次删除所有节点：\n");
	while (pos != NULL) {
		LLNode next = ll_next(pos);
		ll_remove(&pos);
		pos = next;
	}
	show_list(list);

	printf("用append依次添加所有测试元素：\n");
	for (int i = 0; i < 7; i++)
		ll_append(list, x[i]);
	show_list(list);

	printf("用removeall清空整个列表，同时传入标准库函数free来清空所有的元素：\n");
	ll_removeall(list, free);
	show_list(list);

	printf("重新初始化所有测试元素并用prepend依次添加入表中，并进行快速排序：\n");
	x[0] = double_inbox(PI);
	x[1] = double_inbox(E);
	x[2] = double_inbox(1.41421356237);
	x[3] = double_inbox(1.73205080757);
	x[4] = double_inbox(2.2360679775);
	x[5] = double_inbox(0.0);
	x[6] = double_inbox(1.0);
	printf("测试用元素：\n");
	for (int i = 0; i < 7; i++) {
		printf("x[%d]: %f(%p)\n", i, *((double *)x[i]), x[i]);
		ll_prepend(list, x[i]);
	}
	show_list(list);
	printf("快速排序(空链表或只有一个节点的链表进行排序时不会发生任何改变)：\n");
	ll_sort(list);
	show_list(list);

	double t = 1.41421356237;
	printf("正向搜索数字%f，搜索到的节点= %p\n", t, ll_search(list, &t));
	printf("逆向搜索数字%f，搜索到的节点= %p\n", t, ll_rsearch(list, &t));
	t = 1.0;
	printf("正向搜索数字%f，搜索到的节点= %p\n", t, ll_search(list, &t));
	printf("逆向搜索数字%f，搜索到的节点= %p\n", t, ll_rsearch(list, &t));
	t = PI;
	printf("正向搜索数字%f，搜索到的节点= %p\n", t, ll_search(list, &t));
	printf("逆向搜索数字%f，搜索到的节点= %p\n", t, ll_rsearch(list, &t));
	t = 99.999999;
	printf("正向搜索数字%f，搜索到的节点= %p\n", t, ll_search(list, &t));
	printf("逆向搜索数字%f，搜索到的节点= %p\n", t, ll_rsearch(list, &t));

	printf("逆向排序，采用插入排序算法：\n");
	ll_comparator(list, doublecmpr);
	ll_stsort(list);
	ll_comparator(list, NULL);
	show_list(list);

	printf("销毁整个链表，在销毁前要清空所有存放在表中的元素\n");
	for (int i = 0; i < 7; i++)
		free(x[i]);
	ll_destroy(list);

	return 0;
}
