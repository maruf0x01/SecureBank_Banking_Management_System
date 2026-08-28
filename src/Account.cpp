#include "Account.h"

Account::Account()
    : accountNumber(0), balance(0.0), status("Active")
{
}

Account::Account(int accountNumber, const string& customerName, const string& phoneNumber,
                 const string& accountType, double balance, const string& status)
    : accountNumber(accountNumber), customerName(customerName), phoneNumber(phoneNumber),
      accountType(accountType), balance(balance), status(status)
{
}

int Account::getAccountNumber() const
{
    return accountNumber;
}

string Account::getCustomerName() const
{
    return customerName;
}

string Account::getPhoneNumber() const
{
    return phoneNumber;
}

string Account::getAccountType() const
{
    return accountType;
}

double Account::getBalance() const
{
    return balance;
}

string Account::getStatus() const
{
    return status;
}

void Account::setCustomerName(const string& customerName)
{
    this->customerName = customerName;
}

void Account::setPhoneNumber(const string& phoneNumber)
{
    this->phoneNumber = phoneNumber;
}

void Account::setAccountType(const string& accountType)
{
    this->accountType = accountType;
}

void Account::setStatus(const string& status)
{
    this->status = status;
}
