#ifndef TOOL_H
#define TOOL_H

#include <time.h>

/* 创建 data 目录 */
void initDataDir(void);

/* 将 time_t 转为可读字符串，格式 "YYYY-MM-DD HH:MM:SS" */
void timeToString(time_t t, char *buf, int bufSize);

/* 解析 "YYYY-MM-DD HH:MM:SS" 到 time_t，成功返回 1 */
int parseTimeString(const char *text, time_t *outTime);

/* 清空标准输入缓冲区 */
void clearInputBuf(void);

#endif /* TOOL_H */
