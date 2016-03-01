#include <string.h>
#include <stdlib.h>

#include "mr_common.h"

int __MultiThreads__ = 1;

/**
 * (unsigned) char,(unsigned) short,(unsigned) int,(unsigned) long,(unsigned) long long,float,double,long double的比较函数，传入数据的指针进行比较
 * NULL指针认为比非NULL指针小，两个NULL指针认为相等
 * d1,d2:	用于比较的数的指针
 *
 * 返回:	两数相等返回0，*d1>*d2返回一个正整数，*d1<*d2返回一个负整数
 */
int charcmp(const void *d1, const void *d2);
int ucharcmp(const void *d1, const void *d2);
int shortcmp(const void *d1, const void *d2);
int ushortcmp(const void *d1, const void *d2);
int intcmp(const void *d1, const void *d2);
int uintcmp(const void *d1, const void *d2);
int longcmp(const void *d1, const void *d2);
int ulongcmp(const void *d1, const void *d2);
int llcmp(const void *d1, const void *d2);
int ullcmp(const void *d1, const void *d2);
int floatcmp(const void *d1, const void *d2);
int doublecmp(const void *d1, const void *d2);
int ldoublecmp(const void *d1, const void *d2);
int stringcmp(const void *d1, const void *d2);
int objcmp(const void *d1, const void *d2);
CmpFunc default_cmpfunc(ElementType type);
void quicksort(Element *a, int left, int right, CmpFunc cmpfunc);
void insertionsort(Element *a, int left, int right, CmpFunc cmpfunc);

/**
 * 基础数据类型数据的装箱函数，用于将基础类型的临时变量、普通变量、字面量等没有固定内存分配的数据装箱成为一个具有固定地址的容器元素
 */
Element char_inbox(char ch);
Element uchar_inbox(unsigned char ch);
Element int_inbox(int i);
Element uint_inbox(unsigned int i);
Element short_inbox(short i);
Element ushort_inbox(unsigned short i);
Element long_inbox(long i);
Element ulong_inbox(unsigned long i);
Element llong_inbox(long long i);
Element ullong_inbox(unsigned long long i);
Element float_inbox(float x);
Element double_inbox(double x);
Element ldouble_inbox(long double x);

int charcmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				char c1 = *((const char *)d1);
				char c2 = *((const char *)d2);
				if (c1 > c2) {
					ret = 1;
				} else if (c1 < c2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int ucharcmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				unsigned char c1 = *((const unsigned char *)d1);
				unsigned char c2 = *((const unsigned char *)d2);
				if (c1 > c2) {
					ret = 1;
				} else if (c1 < c2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int shortcmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				short c1 = *((const short *)d1);
				short c2 = *((const short *)d2);
				if (c1 > c2) {
					ret = 1;
				} else if (c1 < c2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int ushortcmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				unsigned short c1 = *((const unsigned short *)d1);
				unsigned short c2 = *((const unsigned short *)d2);
				if (c1 > c2) {
					ret = 1;
				} else if (c1 < c2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int intcmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				int c1 = *((const int *)d1);
				int c2 = *((const int *)d2);
				if (c1 > c2) {
					ret = 1;
				} else if (c1 < c2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int uintcmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				unsigned int c1 = *((const unsigned int *)d1);
				unsigned int c2 = *((const unsigned int *)d2);
				if (c1 > c2) {
					ret = 1;
				} else if (c1 < c2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int longcmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				long c1 = *((const long *)d1);
				long c2 = *((const long *)d2);
				if (c1 > c2) {
					ret = 1;
				} else if (c1 < c2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int ulongcmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				unsigned long c1 = *((const unsigned long *)d1);
				unsigned long c2 = *((const unsigned long *)d2);
				if (c1 > c2) {
					ret = 1;
				} else if (c1 < c2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int llcmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				long long c1 = *((const long long *)d1);
				long long c2 = *((const long long *)d2);
				if (c1 > c2) {
					ret = 1;
				} else if (c1 < c2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int ullcmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				unsigned long long c1 = *((const unsigned long long *)d1);
				unsigned long long c2 = *((const unsigned long long *)d2);
				if (c1 > c2) {
					ret = 1;
				} else if (c1 < c2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int floatcmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				float c1 = *((const float *)d1);
				float c2 = *((const float *)d2);
				if (c1 > c2) {
					ret = 1;
				} else if (c1 < c2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int doublecmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				double c1 = *((const double *)d1);
				double c2 = *((const double *)d2);
				if (c1 > c2) {
					ret = 1;
				} else if (c1 < c2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int ldoublecmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				long double c1 = *((const long double *)d1);
				long double c2 = *((const long double *)d2);
				if (c1 > c2) {
					ret = 1;
				} else if (c1 < c2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

int stringcmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				ret = strcmp((const char *)d1, (const char *)d2);
			}
		}
	}
	return ret;
}

/**
 * 在不知道d1,d2指针到底指向什么类型数据的情况下，将其视为通用的对象来进行比较，比较规则如下：
 * d1, d2指针值相同则认为两个对象相同，返回0
 * d1, d2有一个为NULL指针的，认为非NULL指针大于NULL指针，返回-1或1
 * d1, d2均不是NULL指针且不相同的，按指针值大小进行判断，返回-1或1
 */
int objcmp(const void *d1, const void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				if (d1 > d2) {
					ret = 1;
				} else if (d1 < d2) {
					ret = -1;
				}
			}
		}
	}
	return ret;
}

/**
 * 根据元素类型获取默认的比较函数
 */
CmpFunc default_cmpfunc(ElementType type)
{
	CmpFunc ret = NULL;
	switch (type) {
		case Char:
			ret = charcmp;
			break;
		case UChar:
			ret = ucharcmp;
			break;
		case Short:
			ret = shortcmp;
			break;
		case UShort:
			ret = ushortcmp;
			break;
		case Int:
			ret = intcmp;
			break;
		case UInt:
			ret = uintcmp;
			break;
		case Long:
			ret = longcmp;
			break;
		case ULong:
			ret = ulongcmp;
			break;
		case LLong:
			ret = llcmp;
			break;
		case ULLong:
			ret = ullcmp;
			break;
		case Float:
			ret = floatcmp;
			break;
		case Double:
			ret = doublecmp;
			break;
		case LDouble:
			ret = ldoublecmp;
			break;
		case String:
			ret = stringcmp;
			break;
		case Object:
			ret = objcmp;
			break;
		default:
			ret = objcmp;
	}
	return ret;
}

/**
 * 对一组元素进行快速排序
 * a:		待排序元素数组
 * left:	左边界坐标
 * right:	右边界坐标
 * cmpfunc:	比较函数
 *
 */
void quicksort(Element *a, int left, int right, CmpFunc cmpfunc)
{
	if(left >= right)
		return;
	int i = left;
	int j = right;
	Element key = a[left];
	while (i < j) {
		while (i < j && cmpfunc(key, a[j]) <= 0)
			j--;
		if (i < j)
			a[i++] = a[j];
		while (i < j && cmpfunc(key, a[i]) >= 0)
			i++;
		if (i < j)
			a[j--] = a[i];
	}
	a[i] = key;
	quicksort(a, left, i - 1, cmpfunc);
	quicksort(a, i + 1, right, cmpfunc);
	return;
}

/**
 * 对一组元素进行插入排序
 * a:		待排序元素数组
 * left:	左边界坐标
 * right:	右边界坐标
 * cmpfunc:	比较函数
 *
 */
void insertionsort(Element *a, int left, int right, CmpFunc cmpfunc)
{
	int i, j;
	for (i = left + 1; i <= right; i++) {
		Element temp = a[i];
		j = i;
		while (j > 0 && cmpfunc(a[j - 1], temp) > 0) {
			a[j] = a[j - 1];
			j--;
		}
		a[j] = temp;
	}
}

/**
 * 基础数据类型数据的装箱函数，用于将基础类型的临时变量、普通变量、字面量等没有固定内存分配的数据装箱成为一个具有固定地址的容器元素
 */
Element char_inbox(char ch)
{
	char *ret = (char *)malloc(sizeof(char));
	*ret = ch;
	return ret;
}

Element uchar_inbox(unsigned char ch)
{
	unsigned char *ret = (unsigned char *)malloc(sizeof(unsigned char));
	*ret = ch;
	return ret;
}

Element int_inbox(int i)
{
	int *ret = (int *)malloc(sizeof(int));
	*ret = i;
	return ret;
}

Element uint_inbox(unsigned int i)
{
	unsigned int *ret = (unsigned int *)malloc(sizeof(unsigned int));
	*ret = i;
	return ret;
}

Element short_inbox(short i)
{
	short *ret = (short *)malloc(sizeof(short));
	*ret = i;
	return ret;
}

Element ushort_inbox(unsigned short i)
{
	unsigned short *ret = (unsigned short *)malloc(sizeof(unsigned short));
	*ret = i;
	return ret;
}

Element long_inbox(long i)
{
	long *ret = (long *)malloc(sizeof(long));
	*ret = i;
	return ret;
}

Element ulong_inbox(unsigned long i)
{
	unsigned long *ret = (unsigned long *)malloc(sizeof(unsigned long));
	*ret = i;
	return ret;
}

Element llong_inbox(long long i)
{
	long long *ret = (long long *)malloc(sizeof(long long));
	*ret = i;
	return ret;
}

Element ullong_inbox(unsigned long long i)
{
	unsigned long long *ret = (unsigned long long *)malloc(sizeof(unsigned long long));
	*ret = i;
	return ret;
}

Element float_inbox(float x)
{
	float *ret = (float *)malloc(sizeof(float));
	*ret = x;
	return ret;
}

Element double_inbox(double x)
{
	double *ret = (double *)malloc(sizeof(double));
	*ret = x;
	return ret;
}

Element ldouble_inbox(long double x)
{
	long double *ret = (long double *)malloc(sizeof(long double));
	*ret = x;
	return ret;
}

