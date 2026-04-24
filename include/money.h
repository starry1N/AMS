#ifndef MONEY_H
#define MONEY_H

#include <time.h>

/* 状态 */
#define MONEY_TYPE_RECHARGE 0  /* 充值 */
#define MONEY_TYPE_REFUND   1  /* 退费 */

/* 充值/退费记录结构体 */
typedef struct {
    char   cardNo[19];  /* 卡号       */
    double amount;      /* 金额       */
    int    type;        /* 状态       */
    time_t opTime;      /* 操作时间   */
    int    delFlag;     /* 删除标识   */
} Money;

#endif /* MONEY_H */
