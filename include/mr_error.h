/**
 * "mr_error.h"，错误处理函数
 *
 * Version 2.0.0-Dev, 李斌，2016/03/10
 */
#ifndef MR_ERROR_H
#define MR_ERROR_H

#define ERR_NONE 0
#define ERR_OUT_OF_MEMORY 1
#define ERR_INVALID_PARAMETER 2
#define ERR_CONTAINER_FULL 3

/**
 * 获取上一次出错的错误码，如果没有出错过则返回0
 * 错误码线程不安全，多线程时未及时读取可能会被后续的出错覆盖，也可能被其他线程误读
 * 
 * 返回:	上一次出错的错误码，没有错误返回0
 */
extern int errcode(void);

/**
 * 获取错误消息
 * ecode:	错误码
 *
 * 返回:	错误码对应的错误消息，ecode == 0返回NULL，未定义的错误码返回"未知错误"
 */
extern string errmsg(int ecode);

/**
 * 设置错误码，一般由库函数在发生错误时调用
 * ecode:	错误码
 */
extern void set_ecode(int ecode);

#endif
