#ifndef BILLING_H
#define BILLING_H

#include <time.h>

/* 消费状态 */
#define BILLING_STATE_UNSETTLED 0  /* 未结算 */
#define BILLING_STATE_SETTLED   1  /* 已结算 */

/* 消费记录结构体 */
typedef struct {
    char   cardNo[19];   /* 卡号             */
    time_t startTime;    /* 上机时间         */
    time_t endTime;      /* 下机时间         */
    double amount;       /* 消费金额         */
    int    state;        /* 消费状态         */
    double ratePerHour;  /* 本次计费费率     */
    int    delFlag;      /* 删除标识         */
} Billing;

/* 计费信息链表结点 */
typedef struct BillingNode {
    Billing              data;
    struct BillingNode  *next;
} BillingNode;

/* 计费信息链表 */
typedef struct {
    BillingNode *head;
    int          count;
} BillingList;

/* 上机信息 */
typedef struct {
    char   cardNo[19];
    double balance;
    time_t logonTime;
    double ratePerHour;
} LogonInfo;

/* 下机结算信息 */
typedef struct {
    char   cardNo[19];
    double amount;
    double balance;
    double neededRecharge;
    time_t logonTime;
    time_t settleTime;
} SettleInfo;

#endif /* BILLING_H */
