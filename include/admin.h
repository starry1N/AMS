#ifndef ADMIN_H
#define ADMIN_H

#include <time.h>

#define ADMIN_NAME_LEN      19
#define ADMIN_PASSWORD_LEN   9

/* 权限位 */
#define ADMIN_PERM_RECHARGE   (1 << 0)
#define ADMIN_PERM_REFUND     (1 << 1)
#define ADMIN_PERM_CANCEL     (1 << 2)
#define ADMIN_PERM_STATS      (1 << 3)
#define ADMIN_PERM_MANAGE     (1 << 4)
#define ADMIN_PERM_ALL        (ADMIN_PERM_RECHARGE | ADMIN_PERM_REFUND | ADMIN_PERM_CANCEL | ADMIN_PERM_STATS | ADMIN_PERM_MANAGE)

typedef struct {
    char userName[ADMIN_NAME_LEN];
    char password[ADMIN_PASSWORD_LEN];
    int  permissions;
    int  delFlag;
} Admin;

typedef struct AdminNode {
    Admin               data;
    struct AdminNode   *next;
} AdminNode;

typedef struct {
    AdminNode *head;
    int        count;
} AdminList;

#endif /* ADMIN_H */