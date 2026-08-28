#pragma once

#include <bits/stdc++.h>
#include "Account.h"
#include "FileManager.h"
using namespace std;

class BankSystem
{
private:
    vector<Account> accounts;
    string accountsFile;
    string transactionsFile;

    string readRequiredText(const string& prompt);
    string readOptionalText(const string& prompt);
    string readAccountType(bool allowSkip = false);
    int readAccountNumber(const string& prompt) const;
    long long readAmount(const string& prompt);
    int findAccountIndex(int accountNumber) const;
    void printAccount(const Account& account) const;
    bool saveData();
    Transaction createTransaction(int accountNumber, const string& type, long long amountPaisa,
                                  int relatedAccountNumber = 0) const;
    bool saveFinancialChanges(const vector<Transaction>& newTransactions);

public:
    explicit BankSystem(const string& accountsFile = "data/accounts.txt");

    void createAccount();
    void viewAccounts() const;
    void searchAccount() const;
    void updateAccount();
    void closeAccount();
    void reopenAccount();
    void depositMoney();
    void withdrawMoney();
    void transferMoney();
    void transactionHistory() const;
    void showReports() const;
};
