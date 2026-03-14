/*
 * billing_service.c  —— 业务逻辑层：上下机操作与消费记录管理
 *
 * 依赖数据存储层：card_file.c（卡信息读写）、billing_file.c（消费记录读写）
 * 依赖同层辅助：card_service.c（链表初始化/释放）
 */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/card.h"
#include "../../include/billing.h"
#include "../../include/rate.h"
#include "../../include/card_file.h"
#include "../../include/billing_file.h"
#include "../../include/card_service.h"
#include "../../include/billing_service.h"

/* ================================================================
 *  上机
 * ================================================================ */
int loginCard(const char *cardNo, const char *password,
              double ratePerHour, Card *outCard)
{
    if (ratePerHour <= 0.0) return -4;  /* 费率必须 > 0 */

    CardList list;
    getCard(&list);

    int result = 0;
    CardNode *p = list.head;
    while (p != NULL) {
        Card *card = &p->data;
        if (strcmp(card->cardNo, cardNo) == 0
            && strcmp(card->password, password) == 0) {

            if (card->state != CARD_STATE_NORMAL) { result = -2; break; }
            if (card->money <= 0.0)               { result = -3; break; }

            card->state       = CARD_STATE_USING;
            card->lastUseTime = time(NULL);
            card->useCount++;
            card->currentRate = ratePerHour;  /* 记录本次选用的费率 */
            updateCard(card);

            /* 新建未结算消费记录，存储选用的费率 */
            Billing billing;
            memset(&billing, 0, sizeof(Billing));
            strncpy(billing.cardNo, cardNo, sizeof(billing.cardNo) - 1);
            billing.startTime   = card->lastUseTime;
            billing.endTime     = 0;
            billing.amount      = 0.0;
            billing.state       = BILLING_STATE_UNSETTLED;
            billing.ratePerHour = ratePerHour;
            billing.delFlag     = NOT_DELETED;
            saveBilling(&billing);

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
 *  下机（按当前计费标准计算费用）
 * ================================================================ */
int logoutCard(const char *cardNo, const char *password,
               double *cost, time_t *outEndTime)
{
    CardList list;
    getCard(&list);

    int result = 0;
    CardNode *p = list.head;
    while (p != NULL) {
        Card *card = &p->data;
        if (strcmp(card->cardNo, cardNo) == 0
            && strcmp(card->password, password) == 0) {

            if (card->state != CARD_STATE_USING) { result = -2; break; }

            time_t now   = time(NULL);
            double hours = difftime(now, card->lastUseTime) / 3600.0;
            double rate  = card->currentRate;  /* 使用上机时选定的费率 */
            if (rate <= 0.0) rate = DEFAULT_RATE;  /* 兼容旧数据 */
            double fee   = hours * rate;
            if (fee > card->money) fee = card->money;

            card->money      -= fee;
            card->totalMoney += fee;
            card->state       = CARD_STATE_NORMAL;
            updateCard(card);

            /* 将未结算消费记录更新为已结算，并记录实际费率 */
            Billing billing;
            memset(&billing, 0, sizeof(Billing));
            strncpy(billing.cardNo, cardNo, sizeof(billing.cardNo) - 1);
            billing.startTime   = card->lastUseTime;
            billing.endTime     = now;
            billing.amount      = fee;
            billing.state       = BILLING_STATE_SETTLED;
            billing.ratePerHour = rate;
            billing.delFlag     = NOT_DELETED;
            updateBillingByCard(&billing);

            if (cost       != NULL) *cost       = fee;
            if (outEndTime != NULL) *outEndTime = now;
            result = 1;
            break;
        }
        p = p->next;
    }
    freeCardList(&list);
    return result;
}

/* ================================================================
 *  消费记录查询 / 统计
 * ================================================================ */

/* 返回所有有效消费记录数组，调用方负责 free */
Billing *queryBillings(int *count)
{
    return readAllBillings(count);
}

/* 统计总营业额 */
double getTotalRevenue(void)
{
    int count = 0;
    Billing *billings = readAllBillings(&count);
    double total = 0.0;
    for (int i = 0; i < count; i++)
        total += billings[i].amount;
    if (billings != NULL) free(billings);
    return total;
}
