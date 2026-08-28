#pragma once

#include <bits/stdc++.h>
#include "Account.h"
using namespace std;

class BankSystem
{
private:
    vector<Account> accounts;
    string accountsFile;

    string readRequiredText(const string& prompt);
    string readAccountType();
    int readAccountNumber(const string& prompt);
    double readAmount(const string& prompt);
    int findAccountIndex(int accountNumber) const;
    void printAccount(const Account& account) const;
    bool saveData();

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
};
