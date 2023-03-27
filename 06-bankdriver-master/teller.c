#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <semaphore.h>
#include "teller.h"
#include "account.h"
#include "error.h"
#include "debug.h"
#include "account.c"
/*
 * deposit money into an account
 */
int Teller_DoDeposit(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoDeposit(account 0x%" PRIx64 " amount %" PRId64 ")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL)
  {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
  int bId = AccountNum_GetBranchID(accountNum);
  sem_wait(&(account->sem_account));
  sem_wait(&(bank->branches[bId].sem_branch));

  Account_Adjust(bank, account, amount, 1);

  sem_post(&(account->sem_account));
  sem_post(&(bank->branches[bId].sem_branch));
  return ERROR_SUCCESS;
}

/*
 * withdraw money from an account
 */
int Teller_DoWithdraw(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoWithdraw(account 0x%" PRIx64 " amount %" PRId64 ")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL)
  {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  int bId = AccountNum_GetBranchID(accountNum);
  sem_wait(&(account->sem_account));
  sem_wait(&(bank->branches[bId].sem_branch));

  if (amount > Account_Balance(account))
  {
    sem_post(&(account->sem_account));
    sem_post(&(bank->branches[bId].sem_branch));
    return ERROR_INSUFFICIENT_FUNDS;
  }

  Account_Adjust(bank, account, -amount, 1);
  sem_post(&(account->sem_account));
  sem_post(&(bank->branches[bId].sem_branch));
  return ERROR_SUCCESS;
}

/*
 * do a tranfer from one account to another account
 */
int Teller_DoTransfer(Bank *bank, AccountNumber srcAccountNum,
                      AccountNumber dstAccountNum,
                      AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoTransfer(src 0x%" PRIx64 ", dst 0x%" PRIx64
                ", amount %" PRId64 ")\n",
                srcAccountNum, dstAccountNum, amount));

  Account *srcAccount = Account_LookupByNumber(bank, srcAccountNum);
  if (srcAccount == NULL)
  {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  Account *dstAccount = Account_LookupByNumber(bank, dstAccountNum);
  if (dstAccount == NULL)
  {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  if(srcAccount==dstAccount){
    return ERROR_SUCCESS;
  }
  /*
   * If we are doing a transfer within the branch, we tell the Account module to
   * not bother updating the branch balance since the net change for the
   * branch is 0.
   */
  int updateBranch = !Account_IsSameBranch(srcAccountNum, dstAccountNum);
  int srcbId, dstbId;
  srcbId = AccountNum_GetBranchID(srcAccountNum);
  dstbId = AccountNum_GetBranchID(dstAccountNum);
  if (!updateBranch)
  {
    if (srcAccount->accountNumber < dstAccount->accountNumber)
    {
      sem_wait(&(srcAccount->sem_account));
      sem_wait(&(dstAccount->sem_account));
    }
    else
    {
      sem_wait(&(dstAccount->sem_account));
      sem_wait(&(srcAccount->sem_account));
    }
  }
  else
  {

    if (srcbId < dstbId)
    {
      sem_wait(&(srcAccount->sem_account));
      sem_wait(&(dstAccount->sem_account));
      sem_wait(&(bank->branches[srcbId].sem_branch));
      sem_wait(&(bank->branches[dstbId].sem_branch));
    }
    else
    {
      sem_wait(&(dstAccount->sem_account));
      sem_wait(&(srcAccount->sem_account));
      sem_wait(&(bank->branches[dstbId].sem_branch));
      sem_wait(&(bank->branches[srcbId].sem_branch));
    }
  }
  if (amount > Account_Balance(srcAccount))
  {
    if (!updateBranch)
    {
      sem_post(&(srcAccount->sem_account));
      sem_post(&(dstAccount->sem_account));
    }
    else
    {
      sem_post(&(srcAccount->sem_account));
      sem_post(&(dstAccount->sem_account));
      sem_post(&(bank->branches[srcbId].sem_branch));
      sem_post(&(bank->branches[dstbId].sem_branch));
    }
    return ERROR_INSUFFICIENT_FUNDS;
  }

  Account_Adjust(bank, srcAccount, -amount, updateBranch);
  Account_Adjust(bank, dstAccount, amount, updateBranch);

  if (!updateBranch)
    {
      sem_post(&(srcAccount->sem_account));
      sem_post(&(dstAccount->sem_account));
    }
    else
    {
      sem_post(&(srcAccount->sem_account));
      sem_post(&(dstAccount->sem_account));
      sem_post(&(bank->branches[srcbId].sem_branch));
      sem_post(&(bank->branches[dstbId].sem_branch));
    }
  return ERROR_SUCCESS;
}
