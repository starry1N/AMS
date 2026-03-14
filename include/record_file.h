#ifndef RECORD_FILE_H
#define RECORD_FILE_H

#include "money.h"

int    saveRecord(Money *record);        /* 追加充值/退费记录       */
Money *readAllRecords(int *count);       /* 读取所有有效充值退费记录 */

#endif /* RECORD_FILE_H */
