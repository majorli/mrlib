#include <stdio.h>
#include <string.h>

#include "mr_common.h"
#include "mr_string.h"

/**
 * 按UTF-8编码规则依次顺序或逆序扫描一个字符串中的每个有效字符
 */
int main(void)
{
	char *str = "中国你好！Hello!";				// 扫描的字符串，包含4个汉字，1个中文标点，6个ASCII字符

	char *p = str;						// 从头到尾的顺序扫描，扫描指针的有效范围为从第1个字节到最后一个有效字节，不包含结尾的'\0'，即str <= p < str+strlen(str)
	int loc;
	printf("开始顺序扫描字符串\"%s\"中的文字：\n", str);
	while ((loc = scanChar(str, p)) != 0) {			// 从头开始顺序扫描每一个字，知道返回值为0表示扫描全部结束
		if (loc < 0) {
			printf("%d个无效字节\n", -loc);		// 返回值为负数，说明扫描到一串无效字节，长度为返回值的绝对值
			p -= loc;				// 将扫描指针减去返回值即相当于加上返回值的绝对值，跳过这些无效字节
		} else {
			printf("%d个有效字符：", loc);		// 返回值为正数，说明扫描到一个有效字，字节长度为返回值的绝对值
			while (loc) {
				printf("%02hhX ", *(p++));
				loc--;
			}
			printf("\n");
		}
	}

	printf("开始逆序扫描字符串\"%s\"中的文字：\n", str);
	p = str + strlen(str);					// 从尾到头的逆序扫描，扫描指针的有效范围为结尾的'\0'字节到第2个字节，即str < p <= str+strlen(str)
	while ((loc = scanbChar(str, p)) != 0) {		// 从尾部的0字符开始，逆向扫描每一个字
		if (loc < 0) {
			printf("%d个无效字节\n", -loc);		// 返回值为负数，说明扫描到一串无效字节，长度为返回值的绝对值
			p += loc;				// 由于是逆向扫描，所以扫描指针加上返回的负数即向前跳过这些无效字符
		} else {
			printf("%d个有效字符：", loc);		// 返回值为正数，说明扫描到一个有效字，字节长度为返回值的绝对值
			while (loc) {
				printf("%02hhX ", *(--p));
				loc--;
			}
			printf("\n");
		}
	}
	return 0;
}
