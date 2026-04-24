/*
 * menu.c  —— 表示层：用户界面与交互
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/card.h"
#include "../../include/billing.h"
#include "../../include/rate.h"
#include "../../include/admin.h"
#include "../../include/tool.h"
#include "../../include/card_service.h"
#include "../../include/billing_service.h"
#include "../../include/record_service.h"
#include "../../include/admin_service.h"
#include "../../include/rate_service.h"
#include "../../include/menu.h"

static int readTimeInput(const char *prompt, time_t *outTime)
{
    char buffer[64];
    printf("%s", prompt);
    if (scanf(" %63[^\n]", buffer) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return 0;
    }
    clearInputBuf();
    if (!parseTimeString(buffer, outTime)) {
        printf("时间格式错误！请使用 YYYY-MM-DD HH:MM:SS\n");
        return 0;
    }
    return 1;
}

static void printBillingTable(const Billing *billings, int count)
{
    if (billings == NULL || count <= 0) {
        printf("暂无消费记录。\n");
        return;
    }

    printf("\n\t%-20s%-22s%-22s%-12s%-10s\n",
           "卡号", "上机时间", "下机时间", "消费金额", "状态");
    printf("\t----------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        char sBuf[32], eBuf[32];
        timeToString(billings[i].startTime, sBuf, sizeof(sBuf));
        timeToString(billings[i].endTime,   eBuf, sizeof(eBuf));
        printf("\t%-20s%-22s%-22s%-12.2f%-10s\n",
               billings[i].cardNo, sBuf, eBuf,
               billings[i].amount,
               billings[i].state == BILLING_STATE_SETTLED ? "已结算" : "未结算");
    }
}

static void printMonthlyRevenue(int year, const double monthlyRevenue[12])
{
    printf("\n===== %d 年月营业额 =====\n", year);
    printf("\t%-8s%-12s\n", "月份", "营业额");
    printf("\t----------------------\n");
    for (int i = 0; i < 12; i++) {
        printf("\t%-8d%-12.2f\n", i + 1, monthlyRevenue[i]);
    }
}

/* ================================================================
 *  主菜单输出
 * ================================================================ */
void outputMenu(void)
{
    printf("1025005072李宇轩计算机类2502\n");
    printf("\t============================================\n");
    printf("\t           计  费  管  理  系  统          \n");
    printf("\t============================================\n");
    printf("\t  1. 添加卡                                \n");
    printf("\t  2. 查询卡                                \n");
    printf("\t  3. 上机                                  \n");
    printf("\t  4. 下机                                  \n");
    printf("\t  5. 充值                                  \n");
    printf("\t  6. 退费                                  \n");
    printf("\t  7. 查询统计                              \n");
    printf("\t  8. 注销卡                                \n");
    printf("\t  9. 管理员管理                            \n");
    printf("\t  0. 退出                                  \n");
    printf("\t============================================\n");
    printf("\t请选择菜单项编号: ");
}

/* ================================================================
 *  1. 添加卡
 * ================================================================ */
void addCardMenu(void)
{
    printf("\n===== 添加卡 =====\n");

    Card card;
    memset(&card, 0, sizeof(Card));

    printf("请输入卡号  (最多18位): ");
    if (scanf("%18s", card.cardNo) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    {
        int ch = getchar();
        if (ch != '\n' && ch != EOF) {
            clearInputBuf();
            printf("卡号长度不能超过18位！\n");
            return;
        }
    }
    if (strlen(card.cardNo) == 0) {
        printf("卡号为空！\n");
        return;
    }
    printf("请输入密码  (最多 8位): ");
    if (scanf("%8s", card.password) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    {
        int ch = getchar();
        if (ch != '\n' && ch != EOF) {
            clearInputBuf();
            printf("密码长度不能超过8位！\n");
            return;
        }
    }
    if (strlen(card.password) == 0) {
        printf("密码为空！\n");
        return;
    }
    printf("请输入开卡金额        : ");
    if (scanf("%lf", &card.money) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }

    int ret = addCard(&card);
    if (ret == 1) {
        printf("\n\t%-20s%-12s%-12s%-10s\n",
               "卡号", "密码", "余额", "卡状态");
        printf("\t--------------------------------------------------\n");
        printf("\t%-20s%-12s%-12.2f%-10s\n",
               card.cardNo, card.password, card.money, "未上机");
        printf("添加卡成功！\n");
    } else if (ret == -1) {
        printf("添加卡失败！该卡号已存在。\n");
    } else {
        printf("添加卡失败！写入文件时出错。\n");
    }
}

/* ================================================================
 *  2. 查询卡
 * ================================================================ */
void queryCardMenu(void)
{
    printf("\n===== 查询卡 =====\n");
    printf("请输入卡号关键字: ");

    char keyword[CARD_NO_LEN];
    if (scanf("%18s", keyword) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    {
        int ch = getchar();
        if (ch != '\n' && ch != EOF) {
            clearInputBuf();
            printf("卡号长度不能超过18位！\n");
            return;
        }
    }
    if (strlen(keyword) == 0) {
        printf("卡号为空！\n");
        return;
    }

    CardList list;
    getCard(&list);
    
    /* 先尝试精确匹配 */
    Card *exactMatch = queryCard(&list, keyword);
    
    if (exactMatch != NULL) {
        /* 有精确匹配，仅显示该条记录 */
        const char *stateNames[] = {"未上机", "上机中", "已注销"};
        printf("\n\t%-20s%-10s%-12s%-12s%-12s%-22s\n",
               "卡号", "状态", "余额", "累计使用", "使用次数", "上次使用时间");
        printf("\t--------------------------------------------------------------------------------\n");
        char timeBuf[32];
        int si = (exactMatch->state >= 0 && exactMatch->state <= 2) ? exactMatch->state : 0;
        timeToString(exactMatch->lastUseTime, timeBuf, sizeof(timeBuf));
        printf("\t%-20s%-10s%-12.2f%-12.2f%-12d%-22s\n",
               exactMatch->cardNo, stateNames[si], exactMatch->money, exactMatch->totalMoney,
               exactMatch->useCount, timeBuf);
    } else {
        /* 精确匹配失败，进行模糊查询 */
        CardList result;
        queryCards(&list, keyword, &result);
        
        if (result.count == 0) {
            printf("未找到相关卡信息。\n");
        } else {
            const char *stateNames[] = {"未上机", "上机中", "已注销"};
            printf("\n\t%-20s%-10s%-12s%-12s%-12s%-22s\n",
                   "卡号", "状态", "余额", "累计使用", "使用次数", "上次使用时间");
            printf("\t--------------------------------------------------------------------------------\n");
            CardNode *p = result.head;
            while (p != NULL) {
                Card *c = &p->data;
                char timeBuf[32];
                int si = (c->state >= 0 && c->state <= 2) ? c->state : 0;
                timeToString(c->lastUseTime, timeBuf, sizeof(timeBuf));
                printf("\t%-20s%-10s%-12.2f%-12.2f%-12d%-22s\n",
                       c->cardNo, stateNames[si], c->money, c->totalMoney,
                       c->useCount, timeBuf);
                p = p->next;
            }
        }
        
        freeCardList(&result);
    }

    freeCardList(&list);
}

/* ================================================================
 *  3. 上机
 * ================================================================ */
void loginMenu(void)
{
    printf("\n===== 上机 =====\n");

    /* 第1步：展示所有可用计费方案 */
    int rateCount = 0;
    Rate *plans = getAllRatePlans(&rateCount);
    if (plans == NULL || rateCount == 0) {
        printf("暂无可用计费方案，请先在【计费标准】菜单中添加方案。\n");
        if (plans != NULL) free(plans);
        return;
    }

    printf("当前可用计费方案：\n");
    printf("\t%-6s%-34s%-14s\n", "编号", "方案名称", "费率(元/时)");
    printf("\t----------------------------------------------\n");
    for (int i = 0; i < rateCount; i++)
        printf("\t%-6d%-34s%-14.2f\n",
               plans[i].rateId, plans[i].name, plans[i].ratePerHour);

    /* 第2步：用户选择方案 */
    int rateId;
    printf("请输入要使用的方案编号: ");
    if (scanf("%d", &rateId) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        free(plans);
        return;
    }

    /* 查找选中方案的费率 */
    double selectedRate = 0.0;
    for (int i = 0; i < rateCount; i++) {
        if (plans[i].rateId == rateId) {
            selectedRate = plans[i].ratePerHour;
            break;
        }
    }
    free(plans);
    plans = NULL;

    if (selectedRate <= 0.0) {
        printf("方案编号不存在，请重新选择！\n");
        return;
    }

    /* 第3步：输入卡号和密码 */
    char cardNo[CARD_NO_LEN];
    char password[PASSWORD_LEN];
    printf("请输入卡号: ");
    if (scanf("%18s", cardNo) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(cardNo) == 0) {
        printf("卡号为空！\n");
        return;
    }
    printf("请输入密码: ");
    if (scanf("%8s", password) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(password) == 0) {
        printf("密码为空！\n");
        return;
    }

    LogonInfo logonInfo;
    int ret = loginCard(cardNo, password, selectedRate, &logonInfo);
    if (ret == 1) {
        char timeBuf[32];
        timeToString(logonInfo.logonTime, timeBuf, sizeof(timeBuf));
        printf("\n\t%-20s%-12s%-22s\n", "卡号", "余额", "上机时间");
        printf("\t--------------------------------------------------\n");
        printf("\t%-20s%-12.2f%-22s\n",
               logonInfo.cardNo, logonInfo.balance, timeBuf);
        printf("上机成功！\n");
    } else if (ret == -2) {
        printf("该卡正在使用或已注销！\n");
    } else if (ret == -3) {
        printf("余额不足！\n");
    } else if (ret == -4) {
        printf("上机失败！费率无效。\n");
    } else {
        printf("上机失败！卡号或密码错误。\n");
    }
}

/* ================================================================
 *  4. 下机
 * ================================================================ */
void logoutMenu(void)
{
    printf("\n===== 下机 =====\n");

    char cardNo[CARD_NO_LEN];
    char password[PASSWORD_LEN];
    printf("请输入卡号: ");
    if (scanf("%18s", cardNo) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(cardNo) == 0) {
        printf("卡号为空！\n");
        return;
    }
    printf("请输入密码: ");
    if (scanf("%8s", password) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(password) == 0) {
        printf("密码为空！\n");
        return;
    }

    SettleInfo settleInfo;
    int ret = logoutCard(cardNo, password, &settleInfo);
    if (ret == 1) {
        char logonBuf[32];
        char settleBuf[32];
        timeToString(settleInfo.logonTime, logonBuf, sizeof(logonBuf));
        timeToString(settleInfo.settleTime, settleBuf, sizeof(settleBuf));
        printf("\n\t%-20s%-12s%-12s%-22s%-22s\n",
               "卡号", "消费金额", "余额", "上机时间", "下机时间");
        printf("\t--------------------------------------------------------------------------------\n");
        printf("\t%-20s%-12.2f%-12.2f%-22s%-22s\n",
               settleInfo.cardNo, settleInfo.amount, settleInfo.balance,
               logonBuf, settleBuf);
        printf("下机成功！\n");
    } else if (ret == -2) {
        printf("下机失败！该卡未处于上机状态。\n");
    } else if (ret == -3) {
        printf("下机失败！余额不足，当前余额无法结算本次费用。\n");
        printf("至少还需充值 %.2f 元。\n", settleInfo.neededRecharge);
        printf("是否立即充值后重新下机？(1=是 / 0=否): ");

        int rechargeChoice = 0;
        if (scanf("%d", &rechargeChoice) != 1) {
            clearInputBuf();
            printf("输入无效！\n");
            return;
        }
        clearInputBuf();

        if (rechargeChoice == 1) {
            double rechargeAmount = 0.0;
            printf("请输入充值金额（建议不低于 %.2f 元）: ", settleInfo.neededRecharge);
            if (scanf("%lf", &rechargeAmount) != 1) {
                clearInputBuf();
                printf("输入无效！\n");
                return;
            }
            clearInputBuf();

            if (rechargeAmount <= 0.0) {
                printf("充值金额必须大于0！\n");
                return;
            }

            Card rechargeCardInfo;
            int rechargeRet = rechargeCard(cardNo, password, rechargeAmount, &rechargeCardInfo);
            if (rechargeRet == 1) {
                printf("充值成功，已自动重新尝试下机。\n");
                ret = logoutCard(cardNo, password, &settleInfo);
                if (ret == 1) {
                    char logonBuf[32];
                    char settleBuf[32];
                    timeToString(settleInfo.logonTime, logonBuf, sizeof(logonBuf));
                    timeToString(settleInfo.settleTime, settleBuf, sizeof(settleBuf));
                    printf("\n\t%-20s%-12s%-12s%-22s%-22s\n",
                           "卡号", "消费金额", "余额", "上机时间", "下机时间");
                    printf("\t--------------------------------------------------------------------------------\n");
                    printf("\t%-20s%-12.2f%-12.2f%-22s%-22s\n",
                           settleInfo.cardNo, settleInfo.amount, settleInfo.balance,
                           logonBuf, settleBuf);
                    printf("下机成功！\n");
                } else if (ret == -3) {
                    printf("充值后余额仍不足，请继续充值后再尝试下机。\n");
                } else if (ret == -2) {
                    printf("下机失败！该卡未处于上机状态。\n");
                } else {
                    printf("下机失败！卡号或密码错误。\n");
                }
            } else {
                printf("充值失败！卡号或密码错误，或该卡已注销。\n");
            }
        }
    } else {
        printf("下机失败！卡号或密码错误。\n");
    }
}

/* ================================================================
 *  5. 充值
 * ================================================================ */
void rechargeMenu(void)
{
    printf("\n===== 充值 =====\n");

    char   cardNo[CARD_NO_LEN];
    char   password[PASSWORD_LEN];
    double amount;

    printf("请输入卡号  : ");
    if (scanf("%18s", cardNo) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(cardNo) == 0) {
        printf("卡号为空！\n");
        return;
    }
    printf("请输入密码  : ");
    if (scanf("%8s", password) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(password) == 0) {
        printf("密码为空！\n");
        return;
    }
    printf("请输入充值金额: ");
    if (scanf("%lf", &amount) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }

    if (amount <= 0.0) {
        printf("充值金额必须大于0！\n");
        return;
    }

    Card card;
    int ret = rechargeCard(cardNo, password, amount, &card);
    if (ret == 1) {
        printf("\n\t%-20s%-12s%-12s\n", "卡号", "充值金额", "余额");
        printf("\t------------------------------------------\n");
        printf("\t%-20s%-12.2f%-12.2f\n", cardNo, amount, card.money);
        printf("充值成功！\n");
    } else if (ret == -2) {
        printf("充值失败！该卡已注销。\n");
    } else {
        printf("充值失败！卡号或密码错误。\n");
    }
}

/* ================================================================
 *  6. 退费
 * ================================================================ */
void refundMenu(void)
{
    printf("\n===== 退费 =====\n");

    char   cardNo[CARD_NO_LEN];
    char   password[PASSWORD_LEN];
    double amount;

    printf("请输入卡号  : ");
    if (scanf("%18s", cardNo) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(cardNo) == 0) {
        printf("卡号为空！\n");
        return;
    }
    printf("请输入密码  : ");
    if (scanf("%8s", password) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(password) == 0) {
        printf("密码为空！\n");
        return;
    }
    printf("请输入退费金额: ");
    if (scanf("%lf", &amount) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }

    if (amount <= 0.0) {
        printf("退费金额必须大于0！\n");
        return;
    }

    Card card;
    int ret = refundCard(cardNo, password, amount, &card);
    if (ret == 1) {
        printf("\n\t%-20s%-12s%-12s\n", "卡号", "退费金额", "余额");
        printf("\t------------------------------------------\n");
        printf("\t%-20s%-12.2f%-12.2f\n", cardNo, amount, card.money);
        printf("退费成功！\n");
    } else if (ret == -2) {
        printf("退费失败！该卡正在使用或已注销。\n");
    } else if (ret == -3) {
        printf("退费失败！退费金额超过余额。\n");
    } else {
        printf("退费失败！卡号或密码错误。\n");
    }
}

/* ================================================================
 *  7. 查询统计
 * ================================================================ */
void queryStatisticsMenu(void)
{
    printf("\n===== 查询统计 =====\n");
    printf("  1. 按卡号和时间段查询消费记录\n");
    printf("  2. 统计指定时间段总营业额\n");
    printf("  3. 统计一年中每个月营业额\n");
    printf("  4. 查询全部消费记录\n");
    printf("  5. 统计总营业额\n");
    printf("  0. 返回\n");
    printf("请选择: ");

    int choice;
    if (scanf("%d", &choice) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    clearInputBuf();

    if (choice == 1) {
        char cardNo[CARD_NO_LEN];
        time_t startTime = 0;
        time_t endTime = 0;
        int count = 0;

        printf("请输入卡号: ");
        if (scanf("%18s", cardNo) != 1) {
            clearInputBuf();
            printf("输入无效！\n");
            return;
        }
        clearInputBuf();
        if (strlen(cardNo) == 0) {
            printf("卡号为空！\n");
            return;
        }
        if (!readTimeInput("请输入开始时间(YYYY-MM-DD HH:MM:SS): ", &startTime)) return;
        if (!readTimeInput("请输入结束时间(YYYY-MM-DD HH:MM:SS): ", &endTime)) return;
        if (startTime > endTime) {
            printf("开始时间不能大于结束时间！\n");
            return;
        }

        Billing *billings = queryBillingsByCardAndTime(cardNo, startTime, endTime, &count);
        printBillingTable(billings, count);
        if (billings != NULL) free(billings);
    } else if (choice == 2) {
        time_t startTime = 0;
        time_t endTime = 0;

        if (!readTimeInput("请输入开始时间(YYYY-MM-DD HH:MM:SS): ", &startTime)) return;
        if (!readTimeInput("请输入结束时间(YYYY-MM-DD HH:MM:SS): ", &endTime)) return;
        if (startTime > endTime) {
            printf("开始时间不能大于结束时间！\n");
            return;
        }

        double total = getTotalRevenueByTime(startTime, endTime);
        printf("\n指定时间段总营业额：%.2f 元\n", total);
    } else if (choice == 3) {
        int year;
        printf("请输入统计年份(例如 2026): ");
        if (scanf("%d", &year) != 1) {
            clearInputBuf();
            printf("输入无效！\n");
            return;
        }
        clearInputBuf();

        double monthlyRevenue[12];
        getMonthlyRevenue(year, monthlyRevenue);
        printMonthlyRevenue(year, monthlyRevenue);
    } else if (choice == 4) {
        int count = 0;
        Billing *billings = queryBillings(&count);
        printBillingTable(billings, count);
        if (billings != NULL) free(billings);
    } else if (choice == 5) {
        double total = getTotalRevenue();
        printf("\n总营业额：%.2f 元\n", total);
    } else if (choice != 0) {
        printf("无效的选择！\n");
    }
}

/* ================================================================
 *  8. 注销卡
 * ================================================================ */
void cancelCardMenu(void)
{
    printf("\n===== 注销卡 =====\n");

    char cardNo[CARD_NO_LEN];
    char password[PASSWORD_LEN];
    printf("请输入卡号: ");
    if (scanf("%18s", cardNo) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(cardNo) == 0) {
        printf("卡号为空！\n");
        return;
    }
    printf("请输入密码: ");
    if (scanf("%8s", password) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(password) == 0) {
        printf("密码为空！\n");
        return;
    }

    double refundAmount = 0.0;
    int ret = cancelCardService(cardNo, password, &refundAmount);
    if (ret == 1) {
        printf("\n\t%-20s%-12s\n", "卡号", "退款金额");
        printf("\t--------------------------------\n");
        printf("\t%-20s%-12.2f\n", cardNo, refundAmount);
        printf("注销卡成功！\n");
    } else if (ret == -2) {
        printf("注销卡失败！该卡正在使用中，请先下机。\n");
    } else if (ret == -3) {
        printf("注销卡失败！该卡已注销。\n");
    } else {
        printf("注销卡失败！卡号或密码错误。\n");
    }
}

/* ================================================================
 *  9. 计费标准管理
 * ================================================================ */
void rateManagementMenu(void)
{
    int choice;
    do {
        printf("\n===== 计费标准管理 =====\n");
        printAllRates();
        printf("\n  1. 添加计费方案\n");
        printf("  2. 删除计费方案\n");
        printf("  3. 修改计费方案\n");
        printf("  0. 返回\n");
        printf("请选择: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuf();
            choice = -1;
        }

        if (choice == 1) {
            char name[RATE_NAME_LEN];
            double newRate;
            printf("请输入方案名称（最多32字符）: ");
            scanf(" %32[^\n]", name);
            printf("请输入费率（元/小时）: ");
            scanf("%lf", &newRate);
            if (newRate <= 0.0) {
                printf("费率必须大于 0！\n");
            } else {
                if (addRatePlan(name, newRate))
                    printf("计费方案「%s」（%.2f 元/时）已添加。\n", name, newRate);
                else
                    printf("添加失败！写入文件时出错。\n");
            }
        } else if (choice == 2) {
            int rateId;
            printf("请输入要删除的方案编号: ");
            if (scanf("%d", &rateId) != 1) {
                clearInputBuf();
                printf("输入无效！\n");
                continue;
            }
            if (deleteRatePlan(rateId))
                printf("方案编号 %d 已删除。\n", rateId);
            else
                printf("删除失败！未找到编号为 %d 的方案。\n", rateId);
        } else if (choice == 3) {
            int rateId;
            char name[RATE_NAME_LEN];
            double newRate;

            printf("请输入要修改的方案编号: ");
            if (scanf("%d", &rateId) != 1) {
                clearInputBuf();
                printf("输入无效！\n");
                continue;
            }
            clearInputBuf();

            printf("请输入新的方案名称（最多32字符）: ");
            if (scanf(" %32[^\n]", name) != 1) {
                clearInputBuf();
                printf("输入无效！\n");
                continue;
            }
            clearInputBuf();
            if (strlen(name) == 0) {
                printf("方案名称为空！\n");
                continue;
            }

            printf("请输入新的费率（元/小时）: ");
            if (scanf("%lf", &newRate) != 1) {
                clearInputBuf();
                printf("输入无效！\n");
                continue;
            }
            clearInputBuf();
            if (newRate <= 0.0) {
                printf("费率必须大于 0！\n");
                continue;
            }

            if (updateRatePlan(rateId, name, newRate))
                printf("方案编号 %d 已修改为「%s」（%.2f 元/时）。\n", rateId, name, newRate);
            else
                printf("修改失败！未找到编号为 %d 的方案。\n", rateId);
        }
        clearInputBuf();
    } while (choice != 0);
}

/* ================================================================
 *  9. 管理员管理
 * ================================================================ */
void adminMenu(void)
{
    printf("\n===== 管理员登录 =====\n");

    char userName[ADMIN_NAME_LEN];
    char password[ADMIN_PASSWORD_LEN];

    printf("请输入管理员账号: ");
    if (scanf("%18s", userName) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    clearInputBuf();
    if (strlen(userName) == 0) {
        printf("管理员账号为空！\n");
        return;
    }

    printf("请输入密码: ");
    if (scanf("%8s", password) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    clearInputBuf();
    if (strlen(password) == 0) {
        printf("密码为空！\n");
        return;
    }

    Admin currentAdmin;
    if (loginAdmin(userName, password, &currentAdmin) != 1) {
        printf("管理员登录失败！账号或密码错误。\n");
        return;
    }

    int choice;
    do {
        printf("\n===== 管理员管理 =====\n");
        printf("当前管理员: %s\n", currentAdmin.userName);
        printf("当前权限值: %d\n", currentAdmin.permissions);
        printf("  1. 添加管理员\n");
        printf("  2. 删除管理员\n");
        printf("  3. 计费标准管理\n");
        printf("  0. 返回\n");
        printf("请选择: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuf();
            choice = -1;
        } else {
            clearInputBuf();
        }

        if (choice == 1) {
            if ((currentAdmin.permissions & ADMIN_PERM_MANAGE) == 0) {
                printf("权限不足！\n");
                continue;
            }

            Admin newAdmin;
            memset(&newAdmin, 0, sizeof(newAdmin));

            printf("请输入新管理员账号: ");
            if (scanf("%18s", newAdmin.userName) != 1) {
                clearInputBuf();
                printf("输入无效！\n");
                continue;
            }
            clearInputBuf();
            if (strlen(newAdmin.userName) == 0) {
                printf("管理员账号为空！\n");
                continue;
            }

            printf("请输入新管理员密码: ");
            if (scanf("%8s", newAdmin.password) != 1) {
                clearInputBuf();
                printf("输入无效！\n");
                continue;
            }
            clearInputBuf();
            if (strlen(newAdmin.password) == 0) {
                printf("密码为空！\n");
                continue;
            }

            printf("请输入权限值(0~%d): ", ADMIN_PERM_ALL);
            if (scanf("%d", &newAdmin.permissions) != 1) {
                clearInputBuf();
                printf("输入无效！\n");
                continue;
            }
            clearInputBuf();
            if (newAdmin.permissions < 0 || newAdmin.permissions > ADMIN_PERM_ALL) {
                printf("权限值无效！\n");
                continue;
            }

            int addRet = addAdmin(&newAdmin);
            if (addRet == 1) {
                printf("管理员 %s 添加成功！\n", newAdmin.userName);
            } else if (addRet == -1) {
                printf("添加失败！管理员账号已存在。\n");
            } else {
                printf("添加失败！写入文件时出错。\n");
            }
        } else if (choice == 2) {
            if ((currentAdmin.permissions & ADMIN_PERM_MANAGE) == 0) {
                printf("权限不足！\n");
                continue;
            }

            char deleteName[ADMIN_NAME_LEN];
            printf("请输入要删除的管理员账号: ");
            if (scanf("%18s", deleteName) != 1) {
                clearInputBuf();
                printf("输入无效！\n");
                continue;
            }
            clearInputBuf();
            if (strlen(deleteName) == 0) {
                printf("管理员账号为空！\n");
                continue;
            }

            if (strcmp(deleteName, currentAdmin.userName) == 0) {
                printf("不能删除当前登录的管理员！\n");
                continue;
            }

            if (deleteAdmin(deleteName) == 1) {
                printf("管理员 %s 删除成功！\n", deleteName);
            } else {
                printf("删除失败！未找到该管理员账号。\n");
            }
        } else if (choice == 3) {
            if ((currentAdmin.permissions & ADMIN_PERM_MANAGE) == 0) {
                printf("权限不足！\n");
                continue;
            }
            rateManagementMenu();
        } else if (choice != 0) {
            printf("无效的选择！\n");
        }
    } while (choice != 0);
}
