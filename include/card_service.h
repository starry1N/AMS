#ifndef CARD_SERVICE_H
#define CARD_SERVICE_H

#include "card.h"

/* -------- 链表辅助（供各 service 层共用）-------- */
void  initCardList(CardList *list);           /* 初始化链表           */
void  freeCardList(CardList *list);           /* 释放链表所有节点内存 */
void  getCard(CardList *list);                /* 从文件读取所有卡到链表 */

/* -------- 卡的基本操作 -------- */
int   addCard(Card *card);
/* 返回值: 1成功 | -1卡号已存在 | 0失败 */

Card *queryCard(CardList *list, const char *cardNo);
/* 精确查询，返回链表中节点数据指针，未找到返回NULL */

void  queryCards(CardList *list, const char *keyword, CardList *result);
/* 模糊查询，结果存入result链表（caller需freeCardList） */

/* 上机/下机  → billing_service.h */
/* 充值/退费  → record_service.h  */
/* 注销卡     → record_service.h  */

#endif /* CARD_SERVICE_H */
