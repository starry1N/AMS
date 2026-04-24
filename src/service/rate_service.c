/*
 * rate_service.c  —— 业务逻辑层：计费方案管理
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/rate.h"
#include "../../include/rate_file.h"
#include "../../include/rate_service.h"

/* 添加计费方案。成功返回1，失败返回0 */
int addRatePlan(const char *name, double ratePerHour)
{
    Rate rate;
    memset(&rate, 0, sizeof(Rate));
    rate.rateId      = getNextRateId();
    strncpy(rate.name, name, RATE_NAME_LEN - 1);
    rate.ratePerHour = ratePerHour;
    rate.updateTime  = time(NULL);
    rate.delFlag     = 0;
    return appendRate(&rate);
}

/* 删除（软删除）计费方案。成功返回1，未找到返回0 */
int deleteRatePlan(int rateId)
{
    return updateRateDelFlag(rateId, 1);
}

/* 修改计费方案。成功返回1，未找到返回0 */
int updateRatePlan(int rateId, const char *name, double ratePerHour)
{
    return updateRatePlanFile(rateId, name, ratePerHour);
}

/* 获取所有有效计费方案，调用方负责 free */
Rate *getAllRatePlans(int *count)
{
    return readAllRates(count);
}

/* 按 ID 获取费率；未找到时返回 DEFAULT_RATE */
double getRateById(int rateId)
{
    int count = 0;
    Rate *plans = readAllRates(&count);
    double result = DEFAULT_RATE;
    for (int i = 0; i < count; i++) {
        if (plans[i].rateId == rateId) {
            result = plans[i].ratePerHour;
            break;
        }
    }
    if (plans != NULL) free(plans);
    return result;
}

/* 打印所有有效计费方案 */
void printAllRates(void)
{
    int count = 0;
    Rate *plans = readAllRates(&count);
    if (plans == NULL || count == 0) {
        printf("\t暂无计费方案，请先添加。\n");
    } else {
        printf("\n\t%-6s%-34s%-14s%-22s\n", "编号", "方案名称", "费率(元/时)", "更新时间");
        printf("\t----------------------------------------------------------------------\n");
        for (int i = 0; i < count; i++) {
            char buf[32];
            struct tm *t = localtime(&plans[i].updateTime);
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
            printf("\t%-6d%-34s%-14.2f%-22s\n",
                   plans[i].rateId, plans[i].name,
                   plans[i].ratePerHour, buf);
        }
    }
    if (plans != NULL) free(plans);
}
