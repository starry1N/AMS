#ifndef MONEY_H
#define MONEY_H

#include <time.h>

/* 操作类型 */
#define MONEY_TYPE_RECHARGE 1  /* 充值 */
#define MONEY_TYPE_REFUND   2  /* 退费 */

/* 充值/退费记录结构体 */
typedef struct {
    char   cardNo[19];  /* 卡号       */
    double amount;      /* 金额       */
    int    type;        /* 操作类型   */
    time_t opTime;      /* 操作时间   */
    int    delFlag;     /* 删除标识   */
} Money;

#endif /* MONEY_H */
