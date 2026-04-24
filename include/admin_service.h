#ifndef ADMIN_SERVICE_H
#define ADMIN_SERVICE_H

#include "admin.h"

void  initAdminList(AdminList *list);
void  freeAdminList(AdminList *list);
void  getAdmin(AdminList *list);
Admin *queryAdmin(AdminList *list, const char *userName);

int   ensureDefaultAdmin(void);
int   loginAdmin(const char *userName, const char *password, Admin *outAdmin);
int   addAdmin(Admin *admin);
int   deleteAdmin(const char *userName);

#endif /* ADMIN_SERVICE_H */