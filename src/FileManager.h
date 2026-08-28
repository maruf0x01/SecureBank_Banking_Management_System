#pragma once

#include <bits/stdc++.h>
#include "Account.h"
using namespace std;

struct Transaction
{
    string transactionId;
    int accountNumber;
    string type;
    double amount;
    string dateTime;
    int relatedAccountNumber;
};

class FileManager
{
public:
    static vector<Account> loadAccounts(const string& fileName);
    static bool saveAccounts(const string& fileName, const vector<Account>& accounts);
    static bool appendTransaction(const string& fileName, const Transaction& transaction);
    static vector<Transaction> loadTransactions(const string& fileName);
};
