/*
 * record_file.c  —— 数据存储层：充值退费记录文件读写
 *
 * 文件格式：文本，每行一条记录，字段用 '|' 分隔：
 *   cardNo|amount|type|opTime|delFlag
 * 文件路径：data/money.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/card.h"      /* NOT_DELETED */
#include "../../include/money.h"
#include "../../include/record_file.h"

#define RECORD_FILE "data/money.txt"
#define RECORD_LINE_LEN 128

/* 内部辅助：解析一行文本填充到 Money，成功返回1 */
static int parseMoneyLine(const char *line, Money *m)
{
    memset(m, 0, sizeof(Money));
    long opT = 0;
    int n = sscanf(line,
        "%18[^|]|%lf|%d|%ld|%d",
        m->cardNo, &m->amount, &m->type, &opT, &m->delFlag);
    m->opTime = (time_t)opT;
    return (n == 5) ? 1 : 0;
}

/* 追加一条充值/退费记录（文本行）。成功返回1，失败返回0 */
int saveRecord(Money *record)
{
    FILE *fp = fopen(RECORD_FILE, "a");
    if (fp == NULL) return 0;
    fprintf(fp, "%s|%.2f|%d|%ld|%d\n",
            record->cardNo,
            record->amount,
            record->type,
            (long)record->opTime,
            record->delFlag);
    fclose(fp);
    return 1;
}

/* 读取所有有效充值退费记录，返回 malloc 数组，调用方负责 free */
Money *readAllRecords(int *count)
{
    *count = 0;
    FILE *fp = fopen(RECORD_FILE, "r");
    if (fp == NULL) return NULL;

    int total = 0;
    char line[RECORD_LINE_LEN];
    while (fgets(line, sizeof(line), fp) != NULL) total++;
    rewind(fp);

    if (total == 0) { fclose(fp); return NULL; }

    Money *arr = (Money *)malloc(total * sizeof(Money));
    if (arr == NULL) { fclose(fp); return NULL; }

    int i = 0;
    Money r;
    while (fgets(line, sizeof(line), fp) != NULL && i < total) {
        if (parseMoneyLine(line, &r) && r.delFlag == NOT_DELETED)
            arr[i++] = r;
    }
    *count = i;
    fclose(fp);
    return arr;
}
