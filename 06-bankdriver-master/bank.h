#ifndef _BANK_H
#define _BANK_H



typedef struct Bank {
  unsigned int numberBranches;
  int workerCntdwn;
  struct       Branch  *branches;
  struct       Report  *report;
  sem_t sem_checkReport;
  sem_t sem_newDay;
  sem_t sem_transferLock;

} Bank;

#include "account.h"

int Bank_Balance(Bank *bank, AccountAmount *balance);

Bank *Bank_Init(int numBranches, int numAccounts, AccountAmount initAmount,
                AccountAmount reportingAmount,
                int numWorkers);

int Bank_Validate(Bank *bank);
int Bank_Compare(Bank *bank1, Bank *bank2);



#endif /* _BANK_H */
