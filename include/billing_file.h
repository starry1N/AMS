#ifndef BILLING_FILE_H
#define BILLING_FILE_H

#include "billing.h"

int      saveBilling(Billing *billing);            /* 追加消费记录               */
int      updateBillingByCard(Billing *billing);    /* 更新该卡未结算的消费记录   */
Billing *readAllBillings(int *count);              /* 读取所有有效消费记录       */

#endif /* BILLING_FILE_H */
