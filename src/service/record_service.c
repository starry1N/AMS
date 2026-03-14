/*
 * record_service.c  —— 业务逻辑层：充值、退费、注销卡
 *
 * 依赖数据存储层：card_file.c（卡信息读写）、record_file.c（充值退费记录读写）
 * 依赖同层辅助：card_service.c（链表初始化/释放）
 */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/card.h"
#include "../../include/money.h"
#include "../../include/card_file.h"
#include "../../include/record_file.h"
#include "../../include/card_service.h"
#include "../../include/record_service.h"

/* ================================================================
 *  充值
 * ================================================================ */
int rechargeCard(const char *cardNo, const char *password,
                 double amount, Card *outCard)
{
    CardList list;
    getCard(&list);

    int result = 0;
    CardNode *p = list.head;
    while (p != NULL) {
        Card *card = &p->data;
        if (strcmp(card->cardNo, cardNo) == 0
            && strcmp(card->password, password) == 0) {

            if (card->state == CARD_STATE_CANCELLED) { result = -2; break; }

            card->money += amount;
            updateCard(card);

            Money record;
            memset(&record, 0, sizeof(Money));
            strncpy(record.cardNo, cardNo, sizeof(record.cardNo) - 1);
            record.amount  = amount;
            record.type    = MONEY_TYPE_RECHARGE;
            record.opTime  = time(NULL);
            record.delFlag = NOT_DELETED;
            saveRecord(&record);

            if (outCard != NULL) *outCard = *card;
            result = 1;
            break;
        }
        p = p->next;
    }
    freeCardList(&list);
    return result;
}

/* ================================================================
 *  退费
 * ================================================================ */
int refundCard(const char *cardNo, const char *password,
               double amount, Card *outCard)
{
    CardList list;
    getCard(&list);

    int result = 0;
    CardNode *p = list.head;
    while (p != NULL) {
        Card *card = &p->data;
        if (strcmp(card->cardNo, cardNo) == 0
            && strcmp(card->password, password) == 0) {

            if (card->state != CARD_STATE_NORMAL) { result = -2; break; }
            if (amount > card->money)             { result = -3; break; }

            card->money -= amount;
            updateCard(card);

            Money record;
            memset(&record, 0, sizeof(Money));
            strncpy(record.cardNo, cardNo, sizeof(record.cardNo) - 1);
            record.amount  = amount;
            record.type    = MONEY_TYPE_REFUND;
            record.opTime  = time(NULL);
            record.delFlag = NOT_DELETED;
            saveRecord(&record);

            if (outCard != NULL) *outCard = *card;
            result = 1;
            break;
        }
        p = p->next;
    }
    freeCardList(&list);
    return result;
}

/* ================================================================
 *  注销卡
 * ================================================================ */
int cancelCardService(const char *cardNo, const char *password,
                      double *refundAmount)
{
    CardList list;
    getCard(&list);

    int result = 0;
    CardNode *p = list.head;
    while (p != NULL) {
        Card *card = &p->data;
        if (strcmp(card->cardNo, cardNo) == 0
            && strcmp(card->password, password) == 0) {

            if (card->state == CARD_STATE_USING)      { result = -2; break; }
            if (card->state == CARD_STATE_CANCELLED)  { result = -3; break; }

            double refund = card->money;
            card->money   = 0.0;
            card->state   = CARD_STATE_CANCELLED;
            updateCard(card);

            /* 有余额时自动生成退款记录 */
            if (refund > 0.0) {
                Money record;
                memset(&record, 0, sizeof(Money));
                strncpy(record.cardNo, cardNo, sizeof(record.cardNo) - 1);
                record.amount  = refund;
                record.type    = MONEY_TYPE_REFUND;
                record.opTime  = time(NULL);
                record.delFlag = NOT_DELETED;
                saveRecord(&record);
            }

            if (refundAmount != NULL) *refundAmount = refund;
            result = 1;
            break;
        }
        p = p->next;
    }
    freeCardList(&list);
    return result;
}
