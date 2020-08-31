// -------------------------------------------------------------------
// @author: qinhj@lsec.cc.ac.cn
// @brief:  easy log (fake)
// @date:   2019/08/07
// -------------------------------------------------------------------

#ifndef _MY_LOG_H_
#define _MY_LOG_H_

#include <stdio.h>

#define log_err(...)    do { printf("[E] "); printf(__VA_ARGS__); } while (0)
#define log_info(...)   do { printf("[I] "); printf(__VA_ARGS__); } while (0)
#define log_debug(...)  do { printf("[D] "); printf(__VA_ARGS__); } while (0)
#ifdef _DEBUG
#define log_verbose log_debug
#else
#define log_verbose(...)
#endif

#endif /* _MY_LOG_H_ */
