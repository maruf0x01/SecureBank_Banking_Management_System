#pragma once

#include <bits/stdc++.h>
using namespace std;

class Account
{
private:
    int accountNumber;
    string customerName;
    string phoneNumber;
    string accountType;
    long long balancePaisa;
    string status;

public:
    Account();
    Account(int accountNumber, const string& customerName, const string& phoneNumber,
            const string& accountType, long long balancePaisa, const string& status);

    int getAccountNumber() const;
    string getCustomerName() const;
    string getPhoneNumber() const;
    string getAccountType() const;
    long long getBalancePaisa() const;
    string getStatus() const;

    void setCustomerName(const string& customerName);
    void setPhoneNumber(const string& phoneNumber);
    void setAccountType(const string& accountType);
    void setStatus(const string& status);
    bool deposit(long long amountPaisa);
    bool withdraw(long long amountPaisa);
};
