#include <stdio.h>
#include <stdlib.h>
#include "../include/tool.h"
#include "../include/menu.h"

int main(void)
{
    initDataDir();

    int choice;
    do {
        outputMenu();
        if (scanf("%d", &choice) != 1) {
            clearInputBuf();
            choice = -1;
        }

        switch (choice) {
            case 1:  addCardMenu();           break;
            case 2:  queryCardMenu();          break;
            case 3:  loginMenu();              break;
            case 4:  logoutMenu();             break;
            case 5:  rechargeMenu();           break;
            case 6:  refundMenu();             break;
            case 7:  queryStatisticsMenu();    break;
            case 8:  cancelCardMenu();         break;
            case 9:  adminMenu();              break;
            case 0:  printf("\n退出！\n");
                     system("pause");
                     break;
            default: printf("\n输入的菜单序号错误！\n"); break;
        }
        clearInputBuf();  /* 清除本次输入后的残留字符 */

    } while (choice != 0);
    system("pause");
    return 0;
}
