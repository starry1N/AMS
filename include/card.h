#ifndef CARD_H
#define CARD_H

#include <time.h>

#define CARD_NO_LEN      19   /* 卡号最大长度 18 + \0 */
#define PASSWORD_LEN      9   /* 密码最大长度  8 + \0 */
/* 卡状态 */
#define CARD_STATE_NORMAL    0  /* 未上机 */
#define CARD_STATE_USING     1  /* 上机中 */
#define CARD_STATE_CANCELLED 2  /* 已注销 */

/* 注：计费标准由 rate_service.c 管理，不在此定义 */

/* 删除标识 */
#define NOT_DELETED 0
#define DELETED     1

/* 卡信息结构体 */
typedef struct {
    char   cardNo[CARD_NO_LEN];     /* 卡号         */
    char   password[PASSWORD_LEN];  /* 密码         */
    double money;                   /* 余额         */
    int    state;                   /* 卡状态       */
    time_t openTime;                /* 开卡时间     */
    time_t endTime;                 /* 截止时间     */
    time_t lastUseTime;             /* 最后使用时间 */
    int    useCount;                /* 使用次数               */
    double totalMoney;              /* 累积消费金额           */
    double currentRate;             /* 本次上机费率（元/小时）*/
    int    delFlag;                 /* 删除标识               */
} Card;

/* 链表节点 */
typedef struct CardNode {
    Card              data;
    struct CardNode  *next;
} CardNode;

/* 链表头结构 */
typedef struct {
    CardNode *head;
    int       count;
} CardList;

#endif /* CARD_H */
