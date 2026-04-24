#ifndef RATE_SERVICE_H
#define RATE_SERVICE_H

#include "rate.h"

int    addRatePlan(const char *name, double ratePerHour); /* 添加计费方案，成功返回1         */
int    deleteRatePlan(int rateId);                        /* 软删除计费方案，成功返回1       */
int    updateRatePlan(int rateId, const char *name, double ratePerHour); /* 修改计费方案，成功返回1 */
Rate  *getAllRatePlans(int *count);                        /* 获取所有有效方案（调用方 free） */
double getRateById(int rateId);                           /* 按ID获取费率，未找到返DEFAULT_RATE */
void   printAllRates(void);                               /* 打印所有有效计费方案列表        */

#endif /* RATE_SERVICE_H */
