/*
 * card_file.c  —— 数据访问层：文件读写操作
 *
 * 文件格式：文本，每行一条卡记录，字段用 '|' 分隔：
 *   cardNo|password|money|state|openTime|endTime|lastUseTime|useCount|totalMoney|currentRate|delFlag
 * 兼容旧格式（10字段，无 currentRate），currentRate 默认为 0
 * 文件路径：相对于可执行文件所在目录（即项目根目录 计费管理系统/）
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/card.h"
#include "../../include/card_file.h"

/* 消费记录文件操作 -> billing_file.c */
/* 充退费记录文件操作 -> record_file.c  */

#define CARD_FILE "data/card.txt"

/* ================================================================
 *  内部辅助：将一条 Card 写成一行文本（不含换行）
 * ================================================================ */
static void writeCardLine(FILE *fp, const Card *card)
{
    fprintf(fp, "%s|%s|%.2f|%d|%ld|%ld|%ld|%d|%.2f|%.4f|%d\n",
            card->cardNo,
            card->password,
            card->money,
            card->state,
            (long)card->openTime,
            (long)card->endTime,
            (long)card->lastUseTime,
            card->useCount,
            card->totalMoney,
            card->currentRate,
            card->delFlag);
}

/* 内部辅助：解析一行文本填充到 Card，成功返回1 */
static int parseCardLine(const char *line, Card *card)
{
    memset(card, 0, sizeof(Card));
    long openT = 0, endT = 0, lastT = 0;
    /* 先尝试 11 字段（包含 currentRate） */
    int n = sscanf(line,
        "%18[^|]|%8[^|]|%lf|%d|%ld|%ld|%ld|%d|%lf|%lf|%d",
        card->cardNo,
        card->password,
        &card->money,
        &card->state,
        &openT,
        &endT,
        &lastT,
        &card->useCount,
        &card->totalMoney,
        &card->currentRate,
        &card->delFlag);
    if (n == 11) {
        card->openTime    = (time_t)openT;
        card->endTime     = (time_t)endT;
        card->lastUseTime = (time_t)lastT;
        return 1;
    }
    /* 兼容旧格式（10 字段，无 currentRate） */
    n = sscanf(line,
        "%18[^|]|%8[^|]|%lf|%d|%ld|%ld|%ld|%d|%lf|%d",
        card->cardNo,
        card->password,
        &card->money,
        &card->state,
        &openT,
        &endT,
        &lastT,
        &card->useCount,
        &card->totalMoney,
        &card->delFlag);
    card->openTime    = (time_t)openT;
    card->endTime     = (time_t)endT;
    card->lastUseTime = (time_t)lastT;
    card->currentRate = 0.0;  /* 旧数据默认值 */
    return (n == 10) ? 1 : 0;
}

/* ================================================================
 *  卡信息文件操作
 * ================================================================ */

/* 追加一条卡记录（文本行）。成功返回1，失败返回0 */
int saveCard(Card *card)
{
    FILE *fp = fopen(CARD_FILE, "a");
    if (fp == NULL) return 0;
    writeCardLine(fp, card);
    fclose(fp);
    return 1;
}

/* 根据卡号更新对应行（读全部 -> 改 -> 覆盖写回）。成功返回1，未找到返回0 */
int updateCard(Card *card)
{
    FILE *fp = fopen(CARD_FILE, "r");
    if (fp == NULL) return 0;

    /* 读取所有行到内存 */
    char  lines[1024][256];
    int   total = 0;
    while (total < 1024 && fgets(lines[total], sizeof(lines[0]), fp) != NULL)
        total++;
    fclose(fp);

    /* 找到匹配行并替换 */
    int found = 0;
    char newLine[256];
    /* 构造新行（sprintf 不含 \n，后面统一加） */
    snprintf(newLine, sizeof(newLine),
             "%s|%s|%.2f|%d|%ld|%ld|%ld|%d|%.2f|%.4f|%d\n",
             card->cardNo, card->password,
             card->money, card->state,
             (long)card->openTime, (long)card->endTime,
             (long)card->lastUseTime, card->useCount,
             card->totalMoney, card->currentRate, card->delFlag);

    for (int i = 0; i < total; i++) {
        Card tmp;
        if (parseCardLine(lines[i], &tmp)
            && strcmp(tmp.cardNo, card->cardNo) == 0) {
            strncpy(lines[i], newLine, sizeof(lines[i]) - 1);
            lines[i][sizeof(lines[i]) - 1] = '\0';
            found = 1;
            break;
        }
    }
    if (!found) return 0;

    /* 覆盖写回 */
    fp = fopen(CARD_FILE, "w");
    if (fp == NULL) return 0;
    for (int i = 0; i < total; i++)
        fputs(lines[i], fp);
    fclose(fp);
    return 1;
}

/* 返回文件中的有效记录数 */
int getCardCount(void)
{
    FILE *fp = fopen(CARD_FILE, "r");
    if (fp == NULL) return 0;
    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL)
        count++;
    fclose(fp);
    return count;
}

/* 读取所有有效（delFlag==NOT_DELETED）卡记录到链表 */
void readCard(CardList *list)
{
    FILE *fp = fopen(CARD_FILE, "r");
    if (fp == NULL) return;

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        Card card;
        if (!parseCardLine(line, &card)) continue;
        if (card.delFlag != NOT_DELETED)  continue;

        CardNode *node = (CardNode *)malloc(sizeof(CardNode));
        if (node == NULL) continue;
        node->data = card;
        node->next = NULL;
        /* 尾插法 */
        if (list->head == NULL) {
            list->head = node;
        } else {
            CardNode *p = list->head;
            while (p->next != NULL) p = p->next;
            p->next = node;
        }
        list->count++;
    }
    fclose(fp);
}

