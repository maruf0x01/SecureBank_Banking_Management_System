#include <bits/stdc++.h>
#include <cassert>
#include "Account.h"
#include "FileManager.h"
using namespace std;

void testAccountBalanceRules()
{
    Account account(100001, "Test Customer", "01700000000", "Savings", 100.0, "Active");

    assert(account.deposit(50.0));
    assert(account.getBalance() == 150.0);
    assert(!account.deposit(0.0));
    assert(!account.deposit(-10.0));
    assert(!account.deposit(numeric_limits<double>::infinity()));
    assert(account.withdraw(75.0));
    assert(account.getBalance() == 75.0);
    assert(!account.withdraw(100.0));
    assert(!account.withdraw(0.0));
    assert(!account.withdraw(-10.0));
}

void testAccountFileRoundTrip()
{
    const string fileName = "/tmp/securebank-test-accounts.txt";
    vector<Account> expected = {
        Account(100001, "Test Customer", "01700000000", "Savings", 125.50, "Active"),
        Account(100002, "Closed Customer", "01800000000", "Current", 0.0, "Closed")
    };

    assert(FileManager::saveAccounts(fileName, expected));
    vector<Account> actual = FileManager::loadAccounts(fileName);
    assert(actual.size() == 2);
    assert(actual[0].getAccountNumber() == 100001);
    assert(actual[0].getCustomerName() == "Test Customer");
    assert(actual[0].getBalance() == 125.50);
    assert(actual[1].getStatus() == "Closed");

    remove(fileName.c_str());
}

void testMalformedAccountLinesAreIgnored()
{
    const string fileName = "/tmp/securebank-test-malformed-accounts.txt";
    ofstream file(fileName);
    file << "100001|Valid Customer|01700000000|Savings|50.00|Active\n"
         << "invalid|Broken|01800000000|Current|10.00|Active\n"
         << "100002|Negative Balance|01800000000|Current|-5.00|Active\n"
         << "100003|Wrong Status|01900000000|Student|10.00|Unknown\n";
    file.close();

    vector<Account> accounts = FileManager::loadAccounts(fileName);
    assert(accounts.size() == 1);
    assert(accounts[0].getAccountNumber() == 100001);

    remove(fileName.c_str());
}

void testTransactionFileRoundTrip()
{
    const string fileName = "/tmp/securebank-test-transactions.txt";
    Transaction expected{"TXN001", 100001, "Transfer Sent", 25.0,
                        "2026-08-29 12:00:00", 100002};

    assert(FileManager::appendTransaction(fileName, expected));
    vector<Transaction> transactions = FileManager::loadTransactions(fileName);
    assert(transactions.size() == 1);
    assert(transactions[0].transactionId == "TXN001");
    assert(transactions[0].accountNumber == 100001);
    assert(transactions[0].type == "Transfer Sent");
    assert(transactions[0].amount == 25.0);
    assert(transactions[0].relatedAccountNumber == 100002);

    remove(fileName.c_str());
}

int main()
{
    testAccountBalanceRules();
    testAccountFileRoundTrip();
    testMalformedAccountLinesAreIgnored();
    testTransactionFileRoundTrip();
    cout << "All tests passed.\n";
    return 0;
}
