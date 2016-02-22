#include <stdio.h>
#include <stdlib.h>

#include "mr_common.h"
#include "mr_string.h"

int main(void)
{
	char *str = " \t  你好\tHello\t\n";
	printf("测试用的字符串为：\"%s\"\n.", str);

	// 测试是否为空字符串
	printf("is_empty(str) = %d, is_blank(str) = %d.\n", is_empty(str), is_blank(str));
	// trim, ltrim, strim的使用
	char *lstr = (char *)malloc(61 * sizeof(char));
	char *rstr = (char *)malloc(61 * sizeof(char));
	char *sstr = (char *)malloc(61 * sizeof(char));
	printf("ltrim(str) = \"%s\".\n", ltrim(lstr, str));
	printf("rtrim(str) = \"%s\".\n", rtrim(rstr, str));
	printf("trim(str) = \"%s\".\n", trim(sstr, str));
	free(lstr);
	free(rstr);
	free(sstr);

	return 0;
}
