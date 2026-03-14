#ifndef CARD_FILE_H
#define CARD_FILE_H

#include "card.h"

/* -------- 卡文件操作（数据存储层）-------- */
int  saveCard(Card *card);        /* 追加一条卡信息到文件     */
int  updateCard(Card *card);      /* 按卡号更新文件中的卡信息 */
int  getCardCount(void);          /* 获取文件中的卡记录数量   */
void readCard(CardList *list);    /* 读取文件中的卡信息到链表 */

/* 消费记录操作 → billing_file.h */
/* 充退费记录操作 → record_file.h */
/* 计费标准操作   → rate_file.h   */

#endif /* CARD_FILE_H */
