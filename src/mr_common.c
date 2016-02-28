#include <string.h>

#include "mr_common.h"

int __MultiThreads__ = 1;

/**
 * (unsigned) char,(unsigned) short,(unsigned) int,(unsigned) long,(unsigned) long long,float,double,long double的比较函数，传入数据的指针进行比较
 * NULL指针认为比非NULL指针小，两个NULL指针认为相等
 * d1,d2:	用于比较的数的指针
 *
 * 返回:	两数相等返回0，*d1>*d2返回1，*d1<*d2返回-1
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
