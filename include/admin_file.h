#ifndef ADMIN_FILE_H
#define ADMIN_FILE_H

#include "admin.h"

int    saveAdmin(Admin *admin);
int    updateAdmin(Admin *admin);
Admin *readAllAdmins(int *count);

#endif /* ADMIN_FILE_H */