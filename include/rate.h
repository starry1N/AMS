#ifndef RATE_H
#define RATE_H

#include <time.h>

#define DEFAULT_RATE   1.5   /* 默认计费标准: 元/小时 */
#define RATE_NAME_LEN  33    /* 方案名称最大长度 32 + \0 */

/* 计费方案结构体 */
typedef struct {
    int    rateId;              /* 方案编号（自增）        */
    char   name[RATE_NAME_LEN]; /* 方案名称                */
    double ratePerHour;         /* 每小时费率              */
    time_t updateTime;          /* 上次更新时间            */
    int    delFlag;             /* 删除标识 0=正常/1=删除  */
} Rate;

#endif /* RATE_H */
