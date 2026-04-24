/*
 * tool.c  —— 工具函数（数据访问层辅助）
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <direct.h>          /* _mkdir (MinGW/Windows) */
#include "../../include/tool.h"

/* 创建 data 目录（已存在时忽略错误） */
void initDataDir(void)
{
    _mkdir("data");
}

/* 将 time_t 转为 "YYYY-MM-DD HH:MM:SS" 字符串 */
void timeToString(time_t t, char *buf, int bufSize)
{
    if (t == 0) {
        strncpy(buf, "N/A", bufSize - 1);
        buf[bufSize - 1] = '\0';
        return;
    }
    struct tm *tm_info = localtime(&t);
    strftime(buf, bufSize, "%Y-%m-%d %H:%M:%S", tm_info);
}

/* 解析 "YYYY-MM-DD HH:MM:SS" 为 time_t */
int parseTimeString(const char *text, time_t *outTime)
{
    if (text == NULL || outTime == NULL) return 0;

    int year, month, day, hour, minute, second;
    if (sscanf(text, "%d-%d-%d %d:%d:%d",
               &year, &month, &day, &hour, &minute, &second) != 6) {
        return 0;
    }

    struct tm tmInfo;
    memset(&tmInfo, 0, sizeof(tmInfo));
    tmInfo.tm_year = year - 1900;
    tmInfo.tm_mon  = month - 1;
    tmInfo.tm_mday = day;
    tmInfo.tm_hour = hour;
    tmInfo.tm_min  = minute;
    tmInfo.tm_sec  = second;
    tmInfo.tm_isdst = -1;

    time_t value = mktime(&tmInfo);
    if (value == (time_t)-1) return 0;

    *outTime = value;
    return 1;
}

/* 清空标准输入缓冲区中残留字符 */
void clearInputBuf(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}
