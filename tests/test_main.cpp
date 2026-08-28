#include <bits/stdc++.h>
#include <cassert>
#include "Account.h"
#include "FileManager.h"
using namespace std;

void testAccountBalanceRules()
{
    Account account(100001, "Test Customer", "01700000000", "Savings", 10000, "Active");

    assert(account.deposit(5000));
    assert(account.getBalancePaisa() == 15000);
    assert(!account.deposit(0));
    assert(!account.deposit(-1000));
    assert(account.withdraw(7500));
    assert(account.getBalancePaisa() == 7500);
    assert(!account.withdraw(10000));
    assert(!account.withdraw(0));
    assert(!account.withdraw(-1000));
}

void testTransferAndClosedAccountRules()
{
    Account sender(100001, "Sender", "01700000000", "Savings", 10000, "Active");
    Account receiver(100002, "Receiver", "01800000000", "Current", 0, "Active");
    Account closed(100003, "Closed", "01900000000", "Student", 5000, "Closed");

    assert(sender.withdraw(2500));
    assert(receiver.deposit(2500));
    assert(sender.getBalancePaisa() == 7500);
    assert(receiver.getBalancePaisa() == 2500);
    assert(!closed.deposit(0));
    assert(closed.getStatus() == "Closed");
    assert(sender.getAccountNumber() != receiver.getAccountNumber());
}

void testAccountFileRoundTrip()
{
    const string fileName = "/tmp/securebank-test-accounts.txt";
    vector<Account> expected = {
        Account(100001, "Test Customer", "01700000000", "Savings", 12550, "Active"),
        Account(100002, "Closed Customer", "01800000000", "Current", 0, "Closed")
    };

    assert(FileManager::saveAccounts(fileName, expected));
    vector<Account> actual = FileManager::loadAccounts(fileName);
    assert(actual.size() == 2);
    assert(actual[0].getAccountNumber() == 100001);
    assert(actual[0].getCustomerName() == "Test Customer");
    assert(actual[0].getBalancePaisa() == 12550);
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
         << "100003|Wrong Status|01900000000|Student|10.00|Unknown\n"
         << "100001|Duplicate|01900000000|Savings|10.00|Active\n";
    file.close();

    vector<Account> accounts = FileManager::loadAccounts(fileName);
    assert(accounts.size() == 1);
    assert(accounts[0].getAccountNumber() == 100001);

    remove(fileName.c_str());
}

void testInvalidTransactionLinesAreIgnored()
{
    const string fileName = "/tmp/securebank-test-invalid-transactions.txt";
    ofstream file(fileName);
    file << "TXN001|100001|Deposit|10.00|2026-08-29 12:00:00|0\n"
         << "broken|100001|Deposit|-5.00|2026-08-29 12:00:00|0\n"
         << "broken|0|Deposit|5.00|2026-08-29 12:00:00|0\n";
    file.close();

    vector<Transaction> transactions = FileManager::loadTransactions(fileName);
    assert(transactions.size() == 1);
    assert(transactions[0].amountPaisa == 1000);

    remove(fileName.c_str());
}

void testTransactionFileRoundTrip()
{
    const string fileName = "/tmp/securebank-test-transactions.txt";
    Transaction expected{"TXN001", 100001, "Transfer Sent", 2500,
                        "2026-08-29 12:00:00", 100002};

    assert(FileManager::appendTransaction(fileName, expected));
    vector<Transaction> transactions = FileManager::loadTransactions(fileName);
    assert(transactions.size() == 1);
    assert(transactions[0].transactionId == "TXN001");
    assert(transactions[0].accountNumber == 100001);
    assert(transactions[0].type == "Transfer Sent");
    assert(transactions[0].amountPaisa == 2500);
    assert(transactions[0].relatedAccountNumber == 100002);

    remove(fileName.c_str());
}

int main()
{
    testAccountBalanceRules();
    testTransferAndClosedAccountRules();
    testAccountFileRoundTrip();
    testMalformedAccountLinesAreIgnored();
    testInvalidTransactionLinesAreIgnored();
    testTransactionFileRoundTrip();
    cout << "All tests passed.\n";
    return 0;
}
