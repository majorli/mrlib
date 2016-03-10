#include <string.h>
#include <stdlib.h>

#include "mr_common.h"

unsigned int lg2(unsigned int n)
{
	unsigned int ret = 0;
	unsigned int tmp = n;
	while ((tmp = tmp >> 1) > 0)
		ret++;
	return ret;
}

Element integer_inbox(long long value)
{
	long long *ret = (long long *)malloc(sizeof(long long));
	if (ret)
		*ret = value;
	return ret;
}

Element real_inbox(long double value)
{
	long double *ret = (long double *)malloc(sizeof(long double));
	if (ret)
		*ret = value;
	return ret;
}

long long integer_outbox(Element element)
{
	return element ? *(long long *)element : 0;
}

long double real_outbox(Element element)
{
	return element ? *(long double *)element : 0.0;
}

int integer_cmp(const Element e1, const Element e2)
{
	int ret = 0;
	if (e1 != e2) {
		if (e1 && e2)
			ret = *(long long *)e1 == *(long long *)e2 ? 0 : *(long long *)e1 > *(long long *)e2 ? 1 : -1;
		else
			ret = e1 ? 1 : -1;
	}
	return ret;
}

int real_cmp(const Element e1, const Element e2)
{
	int ret = 0;
	if (e1 != e2) {
		if (e1 && e2)
			ret = *(long double *)e1 == *(long double *)e2 ? 0 : *(long double *)e1 > *(long double *)e2 ? 1 : -1;
		else
			ret = e1 ? 1 : -1;
	}
	return ret;
}

int string_cmp(const Element e1, const Element e2)
{
	int ret = 0;
	if (e1 != e2) {
		if (e1 && e2)
			ret = strcmp((const char *)e1, (const char *)e2);
		else
			ret = e1 ? 1 : -1;
	}
	return ret;
}

int obj_cmp(const Element e1, const Element e2)
{
	return e1 == e2 ? 0 : (e1 > e2) ? 1 : -1;
}

CmpFunc default_cmpfunc(ElementType type)
{
	CmpFunc ret = NULL;
	switch (type) {
		case Integer:
			ret = integer_cmp;
			break;
		case Real:
			ret = real_cmp;
			break;
		case String:
			ret = string_cmp;
			break;
		default:
			ret = obj_cmp;
	}
	return ret;
}
