/*
 * billing_file.c  —— 数据存储层：消费记录文件读写
 *
 * 文件格式：文本，每行一条记录，字段用 '|' 分隔：
 *   cardNo|startTime|endTime|amount|state|ratePerHour|delFlag
 * 兼容旧格式（6字段，无 ratePerHour），ratePerHour 默认为 0
 * 文件路径：data/billing.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/card.h"      /* NOT_DELETED */
#include "../../include/billing.h"
#include "../../include/billing_file.h"

#define BILLING_FILE "data/billing.txt"
#define BILLING_LINE_LEN 128

/* 内部辅助：将一条 Billing 写成文本行 */
static void writeBillingLine(FILE *fp, const Billing *b)
{
    fprintf(fp, "%s|%ld|%ld|%.2f|%d|%.4f|%d\n",
            b->cardNo,
            (long)b->startTime,
            (long)b->endTime,
            b->amount,
            b->state,
            b->ratePerHour,
            b->delFlag);
}

/* 内部辅助：解析一行文本填充到 Billing，成功返回1 */
static int parseBillingLine(const char *line, Billing *b)
{
    memset(b, 0, sizeof(Billing));
    long startT = 0, endT = 0;
    /* 先尝试 7 字段（包含 ratePerHour） */
    int n = sscanf(line,
        "%18[^|]|%ld|%ld|%lf|%d|%lf|%d",
        b->cardNo, &startT, &endT,
        &b->amount, &b->state, &b->ratePerHour, &b->delFlag);
    if (n == 7) {
        b->startTime = (time_t)startT;
        b->endTime   = (time_t)endT;
        return 1;
    }
    /* 兼容旧格式（6 字段，无 ratePerHour） */
    n = sscanf(line,
        "%18[^|]|%ld|%ld|%lf|%d|%d",
        b->cardNo, &startT, &endT,
        &b->amount, &b->state, &b->delFlag);
    b->startTime   = (time_t)startT;
    b->endTime     = (time_t)endT;
    b->ratePerHour = 0.0;  /* 旧数据默认值 */
    return (n == 6) ? 1 : 0;
}

/* 追加一条消费记录。成功返回1，失败返回0 */
int saveBilling(Billing *billing)
{
    FILE *fp = fopen(BILLING_FILE, "a");
    if (fp == NULL) return 0;
    writeBillingLine(fp, billing);
    fclose(fp);
    return 1;
}

/* 根据卡号找到第一条"未结算"记录并更新（读全部 -> 改 -> 覆盖写回） */
int updateBillingByCard(Billing *billing)
{
    FILE *fp = fopen(BILLING_FILE, "r");
    if (fp == NULL) return 0;

    char  lines[4096][BILLING_LINE_LEN];
    int   total = 0;
    while (total < 4096 && fgets(lines[total], BILLING_LINE_LEN, fp) != NULL)
        total++;
    fclose(fp);

    int found = 0;
    for (int i = 0; i < total; i++) {
        Billing tmp;
        if (!parseBillingLine(lines[i], &tmp)) continue;
        if (strcmp(tmp.cardNo, billing->cardNo) == 0
            && tmp.state   == BILLING_STATE_UNSETTLED
            && tmp.delFlag == NOT_DELETED) {
            snprintf(lines[i], BILLING_LINE_LEN,
                     "%s|%ld|%ld|%.2f|%d|%.4f|%d\n",
                     billing->cardNo,
                     (long)billing->startTime,
                     (long)billing->endTime,
                     billing->amount,
                     billing->state,
                     billing->ratePerHour,
                     billing->delFlag);
            found = 1;
            break;
        }
    }
    if (!found) return 0;

    fp = fopen(BILLING_FILE, "w");
    if (fp == NULL) return 0;
    for (int i = 0; i < total; i++)
        fputs(lines[i], fp);
    fclose(fp);
    return 1;
}

/* 读取所有有效消费记录，返回 malloc 数组，调用方负责 free */
Billing *readAllBillings(int *count)
{
    *count = 0;
    FILE *fp = fopen(BILLING_FILE, "r");
    if (fp == NULL) return NULL;

    int total = 0;
    char line[BILLING_LINE_LEN];
    while (fgets(line, sizeof(line), fp) != NULL) total++;
    rewind(fp);

    if (total == 0) { fclose(fp); return NULL; }

    Billing *arr = (Billing *)malloc(total * sizeof(Billing));
    if (arr == NULL) { fclose(fp); return NULL; }

    int i = 0;
    Billing b;
    while (fgets(line, sizeof(line), fp) != NULL && i < total) {
        if (parseBillingLine(line, &b) && b.delFlag == NOT_DELETED)
            arr[i++] = b;
    }
    *count = i;
    fclose(fp);
    return arr;
}
