/*
 * rate_file.c  —— 数据存储层：计费方案文件读写
 *
 * 文件格式：文本，每行一条方案，字段用 '|' 分隔：
 *   rateId|name|ratePerHour|updateTime|delFlag
 * 文件路径：data/rate.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/rate.h"
#include "../../include/rate_file.h"

#define RATE_FILE     "data/rate.txt"
#define RATE_LINE_LEN 128

/* 内部辅助：解析一行文本到 Rate，成功返回1 */
static int parseRateLine(const char *line, Rate *r)
{
    memset(r, 0, sizeof(Rate));
    long updateT = 0;
    int n = sscanf(line,
        "%d|%32[^|]|%lf|%ld|%d",
        &r->rateId, r->name, &r->ratePerHour, &updateT, &r->delFlag);
    r->updateTime = (time_t)updateT;
    return (n == 5) ? 1 : 0;
}

/* 追加一条计费方案。成功返回1，失败返回0 */
int appendRate(Rate *rate)
{
    FILE *fp = fopen(RATE_FILE, "a");
    if (fp == NULL) return 0;
    fprintf(fp, "%d|%s|%.4f|%ld|%d\n",
            rate->rateId, rate->name, rate->ratePerHour,
            (long)rate->updateTime, rate->delFlag);
    fclose(fp);
    return 1;
}

/* 按 rateId 更新 delFlag。成功返回1，未找到返回0 */
int updateRateDelFlag(int rateId, int delFlag)
{
    FILE *fp = fopen(RATE_FILE, "r");
    if (fp == NULL) return 0;

    char lines[256][RATE_LINE_LEN];
    int  total = 0;
    while (total < 256 && fgets(lines[total], RATE_LINE_LEN, fp) != NULL)
        total++;
    fclose(fp);

    int found = 0;
    for (int i = 0; i < total; i++) {
        Rate tmp;
        if (!parseRateLine(lines[i], &tmp)) continue;
        if (tmp.rateId == rateId) {
            snprintf(lines[i], RATE_LINE_LEN,
                     "%d|%s|%.4f|%ld|%d\n",
                     tmp.rateId, tmp.name, tmp.ratePerHour,
                     (long)tmp.updateTime, delFlag);
            found = 1;
            break;
        }
    }
    if (!found) return 0;

    fp = fopen(RATE_FILE, "w");
    if (fp == NULL) return 0;
    for (int i = 0; i < total; i++)
        fputs(lines[i], fp);
    fclose(fp);
    return 1;
}

/* 读取所有有效（delFlag==0）计费方案，返回 malloc 数组，调用方负责 free */
Rate *readAllRates(int *count)
{
    *count = 0;
    FILE *fp = fopen(RATE_FILE, "r");
    if (fp == NULL) return NULL;

    int total = 0;
    char line[RATE_LINE_LEN];
    while (fgets(line, sizeof(line), fp) != NULL) total++;
    rewind(fp);

    if (total == 0) { fclose(fp); return NULL; }

    Rate *arr = (Rate *)malloc(total * sizeof(Rate));
    if (arr == NULL) { fclose(fp); return NULL; }

    int i = 0;
    Rate r;
    while (fgets(line, sizeof(line), fp) != NULL && i < total) {
        if (parseRateLine(line, &r) && r.delFlag == 0)
            arr[i++] = r;
    }
    *count = i;
    fclose(fp);
    return arr;
}

/* 获取下一个可用的 rateId（文件中已有最大ID+1；文件不存在时返回1）*/
int getNextRateId(void)
{
    FILE *fp = fopen(RATE_FILE, "r");
    if (fp == NULL) return 1;

    char line[RATE_LINE_LEN];
    int maxId = 0;
    Rate r;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parseRateLine(line, &r) && r.rateId > maxId)
            maxId = r.rateId;
    }
    fclose(fp);
    return maxId + 1;
}
