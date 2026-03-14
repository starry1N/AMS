#ifndef RATE_FILE_H
#define RATE_FILE_H

#include "rate.h"

int    appendRate(Rate *rate);                     /* 追加一条计费方案，成功返回1     */
int    updateRateDelFlag(int rateId, int delFlag); /* 更新删除标识，成功返回1         */
Rate  *readAllRates(int *count);                   /* 读取所有有效方案（调用方 free） */
int    getNextRateId(void);                        /* 获取下一个可用方案ID            */

#endif /* RATE_FILE_H */
