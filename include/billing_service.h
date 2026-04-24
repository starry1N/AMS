#ifndef BILLING_SERVICE_H
#define BILLING_SERVICE_H

#include <time.h>
#include "card.h"
#include "billing.h"

/* -------- 上机 / 下机 -------- */
int loginCard(const char *cardNo, const char *password,
              double ratePerHour, LogonInfo *outInfo);
/*
 * 上机
 * ratePerHour: 本次选用的计费费率（元/小时）
 * 返回:  1  — 成功
 *        0  — 卡号或密码错误
 *       -2  — 卡状态不正确（上机中或已注销）
 *       -3  — 余额不足
 *       -4  — 无效费率（<=0）
 */

int logoutCard(const char *cardNo, const char *password,
               SettleInfo *outInfo);
/*
 * 下机
 * 返回:  1  — 成功
 *        0  — 卡号或密码错误
 *       -2  — 卡未处于上机状态
 *       -3  — 余额不足
 */

/* -------- 消费记录查询 / 统计 -------- */
void     initBillingList(BillingList *list);   /* 初始化计费信息链表 */
void     freeBillingList(BillingList *list);   /* 释放计费信息链表   */
void     getBilling(BillingList *list);        /* 从文件读取计费信息到链表 */
Billing *queryBilling(BillingList *list, const char *cardNo); /* 按卡号查找计费信息 */
Billing *queryBillings(int *count);   /* 获取所有有效消费记录（调用方 free） */
Billing *queryBillingsByCardAndTime(const char *cardNo,
                                    time_t startTime,
                                    time_t endTime,
                                    int *count);
double   getTotalRevenueByTime(time_t startTime, time_t endTime);
void     getMonthlyRevenue(int year, double monthlyRevenue[12]);
double   getTotalRevenue(void);       /* 统计总营业额                       */

#endif /* BILLING_SERVICE_H */
