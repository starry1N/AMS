#ifndef MENU_H
#define MENU_H

void outputMenu(void);            /* 输出主菜单     */
void addCardMenu(void);           /* 1. 添加卡      */
void queryCardMenu(void);         /* 2. 查询卡      */
void loginMenu(void);             /* 3. 上机        */
void logoutMenu(void);            /* 4. 下机        */
void rechargeMenu(void);          /* 5. 充值        */
void refundMenu(void);            /* 6. 退费        */
void queryStatisticsMenu(void);   /* 7. 查询统计    */
void cancelCardMenu(void);        /* 8. 注销卡      */
void adminMenu(void);             /* 9. 管理员管理  */
void rateManagementMenu(void);    /* 计费标准（保留功能，当前未挂到主菜单） */

#endif /* MENU_H */
