/*
 * card_service.c  —— 业务逻辑层：卡信息管理（添加/查询/链表管理）
 *
 * 上机/下机       -> billing_service.c
 * 充值/退费/注销  -> record_service.c
 */
#include <stdlib.h>
#include <string.h>
#include "../../include/card.h"
#include "../../include/card_file.h"
#include "../../include/card_service.h"

/* ================================================================
 *  链表管理
 * ================================================================ */

void initCardList(CardList *list)
{
    list->head  = NULL;
    list->count = 0;
}

void freeCardList(CardList *list)
{
    CardNode *p = list->head;
    while (p != NULL) {
        CardNode *next = p->next;
        free(p);
        p = next;
    }
    list->head  = NULL;
    list->count = 0;
}

/* 从文件读取全部有效卡记录到链表 */
void getCard(CardList *list)
{
    initCardList(list);
    readCard(list);
}

/* ================================================================
 *  卡的基本操作
 * ================================================================ */

/*
 * 添加卡
 * 返回:  1  — 成功
 *       -1  — 卡号已存在
 *        0  — 写文件失败
 */
int addCard(Card *card)
{
    /* 检查卡号唯一性 */
    CardList list;
    getCard(&list);
    CardNode *p = list.head;
    while (p != NULL) {
        if (strcmp(p->data.cardNo, card->cardNo) == 0) {
            freeCardList(&list);
            return -1;
        }
        p = p->next;
    }
    freeCardList(&list);

    /* 填充系统字段 */
    card->openTime    = time(NULL);
    card->endTime     = 0;
    card->lastUseTime = 0;
    card->state       = CARD_STATE_NORMAL;
    card->useCount    = 0;
    card->totalMoney  = 0.0;
    card->delFlag     = NOT_DELETED;

    return saveCard(card) ? 1 : 0;
}

/* 精确查询：在已加载的链表中按卡号查找，返回节点数据指针 */
Card *queryCard(CardList *list, const char *cardNo)
{
    CardNode *p = list->head;
    while (p != NULL) {
        if (strcmp(p->data.cardNo, cardNo) == 0)
            return &p->data;
        p = p->next;
    }
    return NULL;
}

/* 模糊查询：将含 keyword 的卡号对应节点复制到 result 链表 */
void queryCards(CardList *list, const char *keyword, CardList *result)
{
    initCardList(result);
    CardNode *p = list->head;
    while (p != NULL) {
        if (strstr(p->data.cardNo, keyword) != NULL) {
            CardNode *node = (CardNode *)malloc(sizeof(CardNode));
            if (node != NULL) {
                node->data = p->data;
                node->next = NULL;
                if (result->head == NULL) {
                    result->head = node;
                } else {
                    CardNode *q = result->head;
                    while (q->next != NULL) q = q->next;
                    q->next = node;
                }
                result->count++;
            }
        }
        p = p->next;
    }
}



