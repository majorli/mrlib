#include <string.h>

#include "mr_common.h"

static int last_errcode = 0;

/**
 * 获取上一次出错的错误码，如果没有出错过则返回0
 * 错误码线程不安全，多线程时未及时读取可能会被后续的出错覆盖，也可能被其他线程误读
 * 
 * 返回:	上一次出错的错误码，没有错误返回0
 */
int errcode(void)
{
	return last_errcode;
}

/**
 * 获取错误消息
 * ecode:	错误码
 *
 * 返回:	错误码对应的错误消息，ecode == 0返回NULL，未定义的错误码返回"未知错误"
 */
string errmsg(int ecode)
{
	switch (ecode) {
		case ERR_NONE:
			return NULL;
			break;
		case ERR_OUT_OF_MEMORY:
			return "内存不足";
			break;
		case ERR_INVALID_PARAMETER:
			return "无效的参数";
			break;
		case ERR_CONTAINER_FULL:
			return "容器已满";
			break;
		default:
			return "未知错误";
	}
}

/**
 * 设置错误码，一般由库函数在发生错误时调用
 * ecode:	错误码
 */
void set_ecode(int ecode)
{
	last_errcode = ecode;
}
