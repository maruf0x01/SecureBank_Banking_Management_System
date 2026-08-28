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
    double balance;
    string status;

public:
    Account();
    Account(int accountNumber, const string& customerName, const string& phoneNumber,
            const string& accountType, double balance, const string& status);

    int getAccountNumber() const;
    string getCustomerName() const;
    string getPhoneNumber() const;
    string getAccountType() const;
    double getBalance() const;
    string getStatus() const;

    void setCustomerName(const string& customerName);
    void setPhoneNumber(const string& phoneNumber);
    void setAccountType(const string& accountType);
    void setStatus(const string& status);
    bool deposit(double amount);
    bool withdraw(double amount);
};
