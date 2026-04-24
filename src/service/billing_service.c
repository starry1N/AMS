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
 *  计费信息链表管理
 * ================================================================ */

void initBillingList(BillingList *list)
{
    list->head  = NULL;
    list->count = 0;
}

void freeBillingList(BillingList *list)
{
    BillingNode *p = list->head;
    while (p != NULL) {
        BillingNode *next = p->next;
        free(p);
        p = next;
    }
    list->head  = NULL;
    list->count = 0;
}

void getBilling(BillingList *list)
{
    initBillingList(list);

    int count = 0;
    Billing *billings = readAllBillings(&count);
    if (billings == NULL || count == 0) {
        if (billings != NULL) free(billings);
        return;
    }

    BillingNode *tail = NULL;
    for (int i = 0; i < count; i++) {
        BillingNode *node = (BillingNode *)malloc(sizeof(BillingNode));
        if (node == NULL) continue;
        node->data = billings[i];
        node->next = NULL;

        if (list->head == NULL) {
            list->head = node;
        } else {
            tail->next = node;
        }
        tail = node;
        list->count++;
    }

    free(billings);
}

Billing *queryBilling(BillingList *list, const char *cardNo)
{
    BillingNode *p = list->head;
    while (p != NULL) {
        if (strcmp(p->data.cardNo, cardNo) == 0)
            return &p->data;
        p = p->next;
    }
    return NULL;
}

/* ================================================================
 *  上机
 * ================================================================ */
int loginCard(const char *cardNo, const char *password,
              double ratePerHour, LogonInfo *outInfo)
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

            if (outInfo != NULL) {
                strncpy(outInfo->cardNo, card->cardNo, sizeof(outInfo->cardNo) - 1);
                outInfo->cardNo[sizeof(outInfo->cardNo) - 1] = '\0';
                outInfo->balance = card->money;
                outInfo->logonTime = card->lastUseTime;
                outInfo->ratePerHour = ratePerHour;
            }
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
               SettleInfo *outInfo)
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
            double initialBalance = card->money;
            double hours = difftime(now, card->lastUseTime) / 3600.0;
            double rate  = card->currentRate;  /* 使用上机时选定的费率 */
            if (rate <= 0.0) rate = DEFAULT_RATE;  /* 兼容旧数据 */
            double fee   = hours * rate;
            if (fee > initialBalance) {
                if (outInfo != NULL) {
                    strncpy(outInfo->cardNo, card->cardNo, sizeof(outInfo->cardNo) - 1);
                    outInfo->cardNo[sizeof(outInfo->cardNo) - 1] = '\0';
                    outInfo->amount = fee;
                    outInfo->balance = initialBalance;
                    outInfo->neededRecharge = fee - initialBalance;
                    outInfo->logonTime = card->lastUseTime;
                    outInfo->settleTime = now;
                }
                result = -3;
                break;
            }

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

            if (outInfo != NULL) {
                strncpy(outInfo->cardNo, card->cardNo, sizeof(outInfo->cardNo) - 1);
                outInfo->cardNo[sizeof(outInfo->cardNo) - 1] = '\0';
                outInfo->amount = fee;
                outInfo->balance = card->money;
                outInfo->neededRecharge = 0.0;
                outInfo->logonTime = card->lastUseTime;
                outInfo->settleTime = now;
            }
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

static int billingRecordInRange(const Billing *billing, time_t startTime, time_t endTime)
{
    time_t recordTime = billing->endTime != 0 ? billing->endTime : billing->startTime;
    return recordTime >= startTime && recordTime <= endTime;
}

Billing *queryBillingsByCardAndTime(const char *cardNo,
                                    time_t startTime,
                                    time_t endTime,
                                    int *count)
{
    *count = 0;
    int total = 0;
    Billing *billings = readAllBillings(&total);
    if (billings == NULL || total == 0) {
        if (billings != NULL) free(billings);
        return NULL;
    }

    Billing *result = (Billing *)malloc(total * sizeof(Billing));
    if (result == NULL) {
        free(billings);
        return NULL;
    }

    for (int i = 0; i < total; i++) {
        if (strcmp(billings[i].cardNo, cardNo) == 0
            && billingRecordInRange(&billings[i], startTime, endTime)) {
            result[*count] = billings[i];
            (*count)++;
        }
    }

    free(billings);
    if (*count == 0) {
        free(result);
        return NULL;
    }
    return result;
}

double getTotalRevenueByTime(time_t startTime, time_t endTime)
{
    int total = 0;
    Billing *billings = readAllBillings(&total);
    double revenue = 0.0;
    for (int i = 0; i < total; i++) {
        if (billingRecordInRange(&billings[i], startTime, endTime))
            revenue += billings[i].amount;
    }
    if (billings != NULL) free(billings);
    return revenue;
}

void getMonthlyRevenue(int year, double monthlyRevenue[12])
{
    for (int i = 0; i < 12; i++) monthlyRevenue[i] = 0.0;

    int total = 0;
    Billing *billings = readAllBillings(&total);
    if (billings == NULL || total == 0) {
        if (billings != NULL) free(billings);
        return;
    }

    for (int i = 0; i < total; i++) {
        if (billings[i].endTime == 0) continue;
        struct tm *tmInfo = localtime(&billings[i].endTime);
        if (tmInfo == NULL) continue;
        if (tmInfo->tm_year + 1900 == year) {
            int month = tmInfo->tm_mon;
            if (month >= 0 && month < 12)
                monthlyRevenue[month] += billings[i].amount;
        }
    }

    free(billings);
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
