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
int charcmp(void *d1, void *d2);
int ucharcmp(void *d1, void *d2);
int shortcmp(void *d1, void *d2);
int ushortcmp(void *d1, void *d2);
int intcmp(void *d1, void *d2);
int uintcmp(void *d1, void *d2);
int longcmp(void *d1, void *d2);
int ulongcmp(void *d1, void *d2);
int llcmp(void *d1, void *d2);
int ullcmp(void *d1, void *d2);
int floatcmp(void *d1, void *d2);
int doublecmp(void *d1, void *d2);
int ldoublecmp(void *d1, void *d2);
int stringcmp(void *d1, void *d2);
int objcmp(void *d1, void *d2);
CmpFunc default_cmpfunc(ElementType type);

int charcmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				char c1 = *((char *)d1);
				char c2 = *((char *)d2);
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

int ucharcmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				unsigned char c1 = *((unsigned char *)d1);
				unsigned char c2 = *((unsigned char *)d2);
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

int shortcmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				short c1 = *((short *)d1);
				short c2 = *((short *)d2);
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

int ushortcmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				unsigned short c1 = *((unsigned short *)d1);
				unsigned short c2 = *((unsigned short *)d2);
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

int intcmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				int c1 = *((int *)d1);
				int c2 = *((int *)d2);
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

int uintcmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				unsigned int c1 = *((unsigned int *)d1);
				unsigned int c2 = *((unsigned int *)d2);
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

int longcmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				long c1 = *((long *)d1);
				long c2 = *((long *)d2);
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

int ulongcmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				unsigned long c1 = *((unsigned long *)d1);
				unsigned long c2 = *((unsigned long *)d2);
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

int llcmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				long long c1 = *((long long *)d1);
				long long c2 = *((long long *)d2);
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

int ullcmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				unsigned long long c1 = *((unsigned long long *)d1);
				unsigned long long c2 = *((unsigned long long *)d2);
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

int floatcmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				float c1 = *((float *)d1);
				float c2 = *((float *)d2);
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

int doublecmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				double c1 = *((double *)d1);
				double c2 = *((double *)d2);
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

int ldoublecmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				long double c1 = *((long double *)d1);
				long double c2 = *((long double *)d2);
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

int stringcmp(void *d1, void *d2)
{
	int ret = 0;
	if (d1 != d2) {
		if (d1 == NULL) {
			ret = -1;
		} else {
			if (d2 == NULL) {
				ret = 1;
			} else {
				ret = strcmp((char *)d1, (char *)d2);
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
int objcmp(void *d1, void *d2)
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
