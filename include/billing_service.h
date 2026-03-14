#ifndef BILLING_SERVICE_H
#define BILLING_SERVICE_H

#include <time.h>
#include "card.h"
#include "billing.h"

/* -------- 上机 / 下机 -------- */
int loginCard(const char *cardNo, const char *password,
              double ratePerHour, Card *outCard);
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
               double *cost, time_t *outEndTime);
/*
 * 下机
 * 返回:  1  — 成功
 *        0  — 卡号或密码错误
 *       -2  — 卡未处于上机状态
 */

/* -------- 消费记录查询 / 统计 -------- */
Billing *queryBillings(int *count);   /* 获取所有有效消费记录（调用方 free） */
double   getTotalRevenue(void);       /* 统计总营业额                       */

#endif /* BILLING_SERVICE_H */
