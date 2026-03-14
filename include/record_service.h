#ifndef RECORD_SERVICE_H
#define RECORD_SERVICE_H

#include "card.h"
#include "money.h"

int rechargeCard(const char *cardNo, const char *password,
                 double amount, Card *outCard);
/*
 * 充值
 * 返回:  1  — 成功
 *        0  — 卡号或密码错误
 *       -2  — 卡已注销
 */

int refundCard(const char *cardNo, const char *password,
               double amount, Card *outCard);
/*
 * 退费
 * 返回:  1  — 成功
 *        0  — 卡号或密码错误
 *       -2  — 卡正在使用或已注销
 *       -3  — 退费金额超过余额
 */

int cancelCardService(const char *cardNo, const char *password,
                      double *refundAmount);
/*
 * 注销卡
 * 返回:  1  — 成功
 *        0  — 卡号或密码错误
 *       -2  — 卡正在上机中
 *       -3  — 卡已注销
 */

#endif /* RECORD_SERVICE_H */
