#include "Account.h"

Account::Account()
    : accountNumber(0), balancePaisa(0), status("Active")
{
}

Account::Account(int accountNumber, const string& customerName, const string& phoneNumber,
                                 const string& accountType, long long balancePaisa, const string& status)
    : accountNumber(accountNumber), customerName(customerName), phoneNumber(phoneNumber),
            accountType(accountType), balancePaisa(balancePaisa), status(status)
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

long long Account::getBalancePaisa() const
{
    return balancePaisa;
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

bool Account::deposit(long long amountPaisa)
{
    if(amountPaisa <= 0 || balancePaisa > LLONG_MAX - amountPaisa)
        return false;

    balancePaisa += amountPaisa;
    return true;
}

bool Account::withdraw(long long amountPaisa)
{
    if(amountPaisa <= 0 || amountPaisa > balancePaisa)
        return false;

    balancePaisa -= amountPaisa;
    return true;
}
