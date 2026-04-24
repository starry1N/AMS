/*
 * admin_service.c  —— 业务逻辑层：管理员管理
 */
#include <stdlib.h>
#include <string.h>
#include "../../include/admin.h"
#include "../../include/admin_file.h"
#include "../../include/admin_service.h"

void initAdminList(AdminList *list)
{
    list->head = NULL;
    list->count = 0;
}

void freeAdminList(AdminList *list)
{
    AdminNode *p = list->head;
    while (p != NULL) {
        AdminNode *next = p->next;
        free(p);
        p = next;
    }
    list->head = NULL;
    list->count = 0;
}

void getAdmin(AdminList *list)
{
    initAdminList(list);

    int count = 0;
    Admin *admins = readAllAdmins(&count);
    if (admins == NULL || count == 0) {
        if (admins != NULL) free(admins);
        return;
    }

    AdminNode *tail = NULL;
    for (int i = 0; i < count; i++) {
        AdminNode *node = (AdminNode *)malloc(sizeof(AdminNode));
        if (node == NULL) continue;
        node->data = admins[i];
        node->next = NULL;

        if (list->head == NULL) {
            list->head = node;
        } else {
            tail->next = node;
        }
        tail = node;
        list->count++;
    }

    free(admins);
}

Admin *queryAdmin(AdminList *list, const char *userName)
{
    AdminNode *p = list->head;
    while (p != NULL) {
        if (strcmp(p->data.userName, userName) == 0)
            return &p->data;
        p = p->next;
    }
    return NULL;
}

int ensureDefaultAdmin(void)
{
    int count = 0;
    Admin *admins = readAllAdmins(&count);
    if (admins != NULL) free(admins);
    if (count > 0) return 1;

    Admin admin;
    memset(&admin, 0, sizeof(admin));
    strncpy(admin.userName, "admin", sizeof(admin.userName) - 1);
    strncpy(admin.password, "admin123", sizeof(admin.password) - 1);
    admin.permissions = ADMIN_PERM_ALL;
    admin.delFlag = 0;
    return saveAdmin(&admin);
}

int loginAdmin(const char *userName, const char *password, Admin *outAdmin)
{
    ensureDefaultAdmin();

    AdminList list;
    getAdmin(&list);
    Admin *admin = queryAdmin(&list, userName);
    if (admin != NULL && strcmp(admin->password, password) == 0) {
        if (outAdmin != NULL) *outAdmin = *admin;
        freeAdminList(&list);
        return 1;
    }
    freeAdminList(&list);
    return 0;
}

int addAdmin(Admin *admin)
{
    ensureDefaultAdmin();

    AdminList list;
    getAdmin(&list);
    Admin *exist = queryAdmin(&list, admin->userName);
    if (exist != NULL) {
        freeAdminList(&list);
        return -1;
    }
    freeAdminList(&list);

    admin->delFlag = 0;
    return saveAdmin(admin) ? 1 : 0;
}

int deleteAdmin(const char *userName)
{
    ensureDefaultAdmin();

    AdminList list;
    getAdmin(&list);
    Admin *admin = queryAdmin(&list, userName);
    if (admin == NULL) {
        freeAdminList(&list);
        return 0;
    }

    Admin tmp = *admin;
    freeAdminList(&list);
    tmp.delFlag = 1;
    return updateAdmin(&tmp);
}