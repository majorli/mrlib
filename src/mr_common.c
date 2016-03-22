#include <stdlib.h>

#include "mr_common.h"

const int Forward = 1;
const int Reverse = 0;

struct Iterator_t {
	void *iterator;
	Element (*next)(void *);
	Element (*remove)(void *);
	void (*reset)(void *);
	void (*destroy)(void *);
};

unsigned int lg2(unsigned int n)
{
	unsigned int ret = 0;
	unsigned int tmp = n;
	while ((tmp = tmp >> 1) > 0)
		ret++;
	return ret;
}

Iterator it_create(void *iterator, Element (*next)(void *), Element (*remove)(void *), void (*reset)(void *), void (*destroy)(void *))
{
	Iterator it = (Iterator)malloc(sizeof(struct Iterator_t));
	it->iterator = iterator;
	it->next = next;
	it->remove = remove;
	it->reset = reset;
	it->destroy = destroy;
	return it;
}

Element it_next(Iterator it)
{
	return it->next(it->iterator);
}

int it_remove(Iterator it)
{
	return it->remove(it->iterator);
}

void it_reset(Iterator it)
{
	it->reset(it->iterator);
}

void it_destroy(Iterator it)
{
	it->destroy(it->iterator);
	free(it);
}
