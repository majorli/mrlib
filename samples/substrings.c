#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mr_string.h>

int main(void)
{
	char *s = "大家好，Hello, everybody!";
	size_t lob = strlen(s);
	size_t loc = strlen_c(s);

	printf("以字符串\"%s\"为例，用标准库函数strlen()可以获得字节长度为%zu个char，用strlen_c()函数可以获得字长为%zu个字\n", s, lob, loc);
	char *d = (char *)malloc(lob * sizeof(char));
	substr_b(d, s, 2, 12);
	printf("substr_b(d, s, 2, 12) = \"%s\"，出现截取到半个汉字的情况\n", d);
	substr_c(d, s, 2, 12);
	printf("substr_c(d, s, 2, 12) = \"%s\"，按字数截取子串，类似Java中的方法，不会截取到半个汉字\n", d);
	// UTF-8编码的汉字占三个字节
	left_b(d, s, 6);
	printf("left_b(d, s, 6) = \"%s\"\n", d);
	left_c(d, s, 6);
	printf("left_c(d, s, 6) = \"%s\"\n", d);
	right_b(d, s, 20);
	printf("right_b(d, s, 20) = \"%s\"\n", d);
	right_c(d, s, 20);
	printf("right_c(d, s, 20) = \"%s\"\n", d);

	free(d);
	return 0;
}
