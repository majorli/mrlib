#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <mr_hashtable.h>

Container hashtable = NULL;
Iterator iterator = NULL;

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
	"Thailand",
	"Philipin",
	"Italy",
	"Spain",
	"蒙古",
	"北朝鲜",
	"Russia",
	"Mexico",
	"Egypt",
	"India",
	"Pakistein",
	"Vietnam",
	"New Zealand",
	"Demark",
	"Norway"
};

void cont(void)
{
	printf("Press <Enter> to continue...");
	getchar();
}

void add(void)
{
	int idx = rand() % 30;
	printf("Register a nation: %s...", nations[idx]);
	if (hash_register(hashtable, nations[idx], string, strlen(nations[idx])) == 0)
		printf("OK!\n");
	else
		printf("Failed!\n");
}

void search(void)
{
	int idx = rand() % 30;
	printf("Search for \"%s\"...", nations[idx]);
	if (hash_contains(hashtable, nations[idx], string, strlen(nations[idx])))
		printf("Exists!\n");
	else
		printf("None!\n");
}

void rm(void)
{
	int idx = rand() % 30;
	printf("Remove \"%s\"...", nations[idx]);
	if (hash_remove(hashtable, nations[idx], string, strlen(nations[idx])) > 0)
		printf("OK!\n");
	else
		printf("Failed!\n");
}

void show(void)
{
	if (!hashtable || !iterator)
		return;
	printf("Browse hashtable: isempty = %s, size = %zu\n", hash_isempty(hashtable) ? "Yes" : "No", hash_size(hashtable));
	it_reset(iterator);
	Element e;
	while ((e = it_next(iterator))) {
		printf("\"%s\"\n", POINTOF(e, char));
		free(e);
	}
	printf("Browse finished.\n");
}

int main(void)
{
	int i;
	Element e;

	printf("Init hashtable...");
	hashtable = hash_create();
	iterator = hash_iterator(hashtable);
	printf("OK!\n");
	show();
	cont();

	printf("Sample 1: add elements\n");
	for (i = 0; i < 40; i++)
		add();
	show();
	cont();

	printf("Sample 2: search and remove element\n");
	for (i = 0; i < 10; i++)
		search();
	for (i = 0; i < 10; i++)
		rm();
	show();
	cont();

	printf("Sample 3: remove all...");
	hash_removeall(hashtable);
	printf("OK!\n");
	show();
	cont();

	printf("Sample 4: elements of real number\n");
	float u[] = { 3.14159265359, 1.41421356237, 2.71828182846, 1.61803398875 };
	for (i = 0; i < 4; i++)
		if (hash_register(hashtable, &u[i], real, sizeof(float)) == 0)
			printf("add %14.12f success\n", u[i]);
		else
			printf("add %14.12f failed\n", u[i]);
	it_reset(iterator);
	while ((e = it_next(iterator))) {
		printf("%14.12f, ", VALUEOF(e, float));
		free(e);
	}
	printf("\n");
	cont();

	for (i = 0; i < 4; i++)
		if (hash_register(hashtable, &u[i], real, sizeof(float)) == 0)
			printf("add %14.12f success\n", u[i]);
		else
			printf("add %14.12f failed\n", u[i]);
	it_reset(iterator);
	while ((e = it_next(iterator))) {
		printf("%14.12f, ", VALUEOF(e, float));
		free(e);
	}
	printf("\n");
	cont();

	printf("clear hashtable\n");
	for (i = 0; i < 4; i++)
		printf("remove %14.12f %s\n", u[i], hash_remove(hashtable, &u[i], real, sizeof(float)) == 0 ? "failed!" : "success!");
	show();
	cont();

	printf("Destroy hashtable...");
	it_destroy(iterator);
	hash_destroy(hashtable);
	printf("OK!\n");
	return 0;
}
