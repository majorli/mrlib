#include <stdio.h>
#include <stdlib.h>

#include <mr_pqueue.h>

Container pq;

void showhead(void)
{
	int pri;
	Element e;
	e = pq_queuehead(pq, &pri);
	if (e)
		printf("Head：[%02d]%d\n", pri, VALUEOF(e, int));
	else
		printf("Head：NULL\n");
	free(e);
}

void flush(void)
{
	int pri;
	Element e;
	printf("Dequeue to empty：\n");
	while ((e = pq_dequeue(pq, &pri))) {
		printf("[%02d]%d, ", pri, VALUEOF(e, int));
		free(e);
	}
	printf("\n");
}

void init(int count)
{
	int i = 0, pri;
	while (i < count) {
		pri = rand() % 8;
		pq_enqueue(pq, &i, integer, sizeof(int), pri);
		i++;
	}
}

void show(void)
{
	int i = 0, pri;
	size_t s = pq_size(pq);
	Element e;
	printf("Status：%s, Size：%zu.\n", pq_isempty(pq) ? "EMPTY" : "NON-EMPTY", s);
	showhead();
	printf("Browse by index:\n");
	while(i < s) {
		e = pq_get(pq, i++, &pri);
		printf("[%02d]%d, ", pri, VALUEOF(e, int));
		free(e);
	}
	printf("\n");
}

void cont(void)
{
	printf("press <enter> to continue...");
	getchar();
}

int main(void)
{
	printf("Create a priority queue...");
	//pq = pq_create(Min_Priority, integer, NULL);
	pq = pq_create(Max_Priority, integer, NULL);
	printf("OK!\n");
	show();
	cont();
	
	printf("TEST1：Enqueue one element and browse the queue, then dequeue it.\n");
	init(1);
	show();
	flush();
	cont();

	printf("TEST2: Enqueue 10 elements and browse.\n");
	init(10);
	show();
	flush();
	cont();

	printf("TEST3: Search a element with value of 8 and 12, change priority to 5.\n");
	init(10);
	show();
	int v = 12;
	printf("Element with value of 12: %sexisted, index = %d.\n", pq_contains(pq, &v, integer, sizeof(int)) ? "" : "not ", pq_search(pq, &v, integer, sizeof(int)));
	v = 8;
	int pos;
	printf("Element with value of 8: %sexisted, index = %d.\n", pq_contains(pq, &v, integer, sizeof(int)) ? "" : "not ", pos = pq_search(pq, &v, integer, sizeof(int)));
	printf("Change priority to 5...");
	pq_change_pri_at(pq, pos, 5);
	printf("OK!\n");
	show();
	flush();
	cont();

	printf("TEST4: Enqueue 20 elements with some repeated values.\n");
	init(10);
	init(6);
	init(4);
	show();
	flush();
	cont();

	printf("TEST5: Change priority by element value.\n");
	init(5);
	init(5);
	init(5);
	init(5);
	show();
	v = 4;
	printf("Change priority of the elements with value 4 to 5, %d elements changed.\n", pq_change_pri(pq, &v, integer, sizeof(int), 5));
	v = 3;
	printf("Change priority of the elements with value 3 to 7, %d elements changed.\n", pq_change_pri(pq, &v, integer, sizeof(int), 7));
	show();
	flush();
	cont();

	printf("TEST6: Remove all elements.\n");
	init(10);
	show();
	pq_removeall(pq);
	show();
	cont();

	printf("END: Destroy the queue...");
	pq_destroy(pq);
	printf("OK!\n");
	return 0;
}
