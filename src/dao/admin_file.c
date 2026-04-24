/*
 * admin_file.c  —— 数据存储层：管理员账号文件读写
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/admin.h"
#include "../../include/admin_file.h"

#define ADMIN_FILE     "data/admin.txt"
#define ADMIN_LINE_LEN 128

static void writeAdminLine(FILE *fp, const Admin *admin)
{
    fprintf(fp, "%s|%s|%d|%d\n",
            admin->userName,
            admin->password,
            admin->permissions,
            admin->delFlag);
}

static int parseAdminLine(const char *line, Admin *admin)
{
    memset(admin, 0, sizeof(Admin));
    int n = sscanf(line, "%18[^|]|%8[^|]|%d|%d",
                   admin->userName,
                   admin->password,
                   &admin->permissions,
                   &admin->delFlag);
    return (n == 4) ? 1 : 0;
}

int saveAdmin(Admin *admin)
{
    FILE *fp = fopen(ADMIN_FILE, "a");
    if (fp == NULL) return 0;
    writeAdminLine(fp, admin);
    fclose(fp);
    return 1;
}

int updateAdmin(Admin *admin)
{
    FILE *fp = fopen(ADMIN_FILE, "r");
    if (fp == NULL) return 0;

    char lines[256][ADMIN_LINE_LEN];
    int total = 0;
    while (total < 256 && fgets(lines[total], ADMIN_LINE_LEN, fp) != NULL)
        total++;
    fclose(fp);

    int found = 0;
    char newLine[ADMIN_LINE_LEN];
    snprintf(newLine, sizeof(newLine), "%s|%s|%d|%d\n",
             admin->userName, admin->password,
             admin->permissions, admin->delFlag);

    for (int i = 0; i < total; i++) {
        Admin tmp;
        if (parseAdminLine(lines[i], &tmp)
            && strcmp(tmp.userName, admin->userName) == 0) {
            strncpy(lines[i], newLine, sizeof(lines[i]) - 1);
            lines[i][sizeof(lines[i]) - 1] = '\0';
            found = 1;
            break;
        }
    }
    if (!found) return 0;

    fp = fopen(ADMIN_FILE, "w");
    if (fp == NULL) return 0;
    for (int i = 0; i < total; i++)
        fputs(lines[i], fp);
    fclose(fp);
    return 1;
}

Admin *readAllAdmins(int *count)
{
    *count = 0;
    FILE *fp = fopen(ADMIN_FILE, "r");
    if (fp == NULL) return NULL;

    int total = 0;
    char line[ADMIN_LINE_LEN];
    while (fgets(line, sizeof(line), fp) != NULL) total++;
    rewind(fp);

    if (total == 0) { fclose(fp); return NULL; }

    Admin *arr = (Admin *)malloc(total * sizeof(Admin));
    if (arr == NULL) { fclose(fp); return NULL; }

    int i = 0;
    Admin admin;
    while (fgets(line, sizeof(line), fp) != NULL && i < total) {
        if (parseAdminLine(line, &admin) && admin.delFlag == 0)
            arr[i++] = admin;
    }
    *count = i;
    fclose(fp);
    return arr;
}