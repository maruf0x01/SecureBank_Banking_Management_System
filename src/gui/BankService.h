#pragma once

#include <QString>
#include <vector>

#include "Account.h"
#include "FileManager.h"

struct DepositRequest
{
    int id = 0;
    int accountNumber = 0;
    long long amountPaisa = 0;
    QString requestedAt;
    QString status; // Pending, Approved, Rejected
    QString decidedAt;
};

struct Result
{
    bool ok = false;
    QString message;
    int value = 0;
};

// Shared business logic for the GUI (admin and customer windows).
class BankService
{
public:
    BankService();

    void reload();
    const std::vector<Account>& accounts() const { return accountList; }
    const Account* find(int accountNumber) const;

    Result createAccount(const QString& name, const QString& phone, const QString& type);
    // Self-service registration: creates a Pending account plus its login. An admin must activate it.
    Result registerAccount(const QString& name, const QString& phone, const QString& type,
                           const QString& username, const QString& password);
    Result updateAccount(int number, const QString& name, const QString& phone, const QString& type);
    // Permanently deletes an account (balance must be zero), its login and its deposit requests.
    // Transaction history is kept for auditing.
    Result deleteAccount(int number);
    Result setActive(int number, bool active);
    Result deposit(int number, long long paisa);
    Result withdraw(int number, long long paisa);
    // If expectedReceiverName is non-empty, the receiver's account holder name must match it.
    Result transfer(int from, int to, long long paisa, const QString& expectedReceiverName = QString());
    static QString validateName(const QString& name); // empty string = valid
    static bool namesMatch(const QString& a, const QString& b);
    static constexpr long long MaxAmountPaisa = 10000000000LL; // 100,000,000.00 per transaction

    // Newest first. accountNumber 0 = every account.
    std::vector<Transaction> history(int accountNumber = 0) const;

    // Deposit requests: customers request, an admin approves (money is credited) or rejects.
    Result requestDeposit(int accountNumber, long long paisa);
    Result approveDeposit(int requestId);
    Result rejectDeposit(int requestId);
    std::vector<DepositRequest> depositRequests(int accountNumber = 0) const; // newest first
    int pendingDepositCount() const;

    // Customer logins
    bool hasLogin(int accountNumber) const;
    QString loginName(int accountNumber) const;
    Result setCustomerLogin(int accountNumber, const QString& username, const QString& password);
    Result removeCustomerLogin(int accountNumber);
    int authenticate(const QString& username, const QString& password) const; // account no.; -1 bad login, -2 closed, -3 pending
    bool verifyPassword(int accountNumber, const QString& password) const;
    Result changePassword(int accountNumber, const QString& oldPassword, const QString& newPassword);

    static QString validatePhone(const QString& phone);
    static bool parseAmount(const QString& text, long long& paisa);

private:
    struct Credential
    {
        int accountNumber;
        QString username, salt, hash;
    };

    std::vector<Account> accountList;
    std::vector<Credential> credentials;
    std::vector<DepositRequest> deposits;
    const std::string accountsFile = "data/accounts.txt";
    const std::string transactionsFile = "data/transactions.txt";
    const QString customersFile = "data/customers.txt";
    const QString depositsFile = "data/deposit_requests.txt";
    void loadDeposits();
    bool saveDeposits() const;
    DepositRequest* findDeposit(int id);

    Account* findMutable(int number);
    Transaction makeTransaction(int account, const std::string& type, long long paisa, int related = 0) const;
    bool saveAccounts();
    bool saveFinancial(const std::vector<Transaction>& txns);
    void loadCredentials();
    bool saveCredentials() const;
    static QString hashPassword(const QString& salt, const QString& password);
    static QString makeSalt();
    static QString validatePassword(const QString& password);
};
