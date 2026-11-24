// ============================================================================
// FILE: BankService.h
// PURPOSE: Shared business logic and validation layer for the GUI.
//          Both AdminWindow and CustomerWindow call BankService to read/write
//          accounts, transactions, customer logins, deposit requests, and loans.
// ============================================================================
#pragma once

#include <QString>
#include <QStringList>
#include <vector>

#include "Account.h"
#include "FileManager.h"

// Represents a customer deposit request stored in data/deposit_requests.txt
struct DepositRequest
{
    int id = 0;
    int accountNumber = 0;
    long long amountPaisa = 0;
    QString requestedAt;
    QString status; // "Pending", "Approved", or "Rejected"
    QString decidedAt;
};

// Represents a customer loan record stored in data/loans.txt
struct LoanRequest
{
    int id = 0;
    int accountNumber = 0;
    long long amountPaisa = 0;    // Original requested loan amount in paisa
    long long remainingPaisa = 0; // Remaining unpaid amount in paisa
    QString purpose;              // e.g. "Personal", "Education", "Business", "Medical", "Home"
    QString requestedAt;
    QString status;               // "Pending", "Active", "Rejected", or "Paid"
    QString decidedAt;
};

// Standard return type for all operations (ok = success/failure, message = user feedback, value = created ID)
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

    // Reloads all accounts, credentials, deposit requests, and loans from the data/ files
    void reload();
    const std::vector<Account>& accounts() const { return accountList; }
    const Account* find(int accountNumber) const;

    // Account management operations
    Result createAccount(const QString& name, const QString& phone, const QString& type);
    // Self-service registration: creates a Pending account plus its login and 3 security question answers.
    Result registerAccount(const QString& name, const QString& phone, const QString& type,
                           const QString& username, const QString& password,
                           const QString& ans1 = QString(), const QString& ans2 = QString(),
                           const QString& ans3 = QString());
    Result updateAccount(int number, const QString& name, const QString& phone, const QString& type);
    // Permanently deletes an account (balance and unpaid loan must be zero), its login, deposits, and loans.
    // Transaction history is kept for auditing.
    Result deleteAccount(int number);
    Result setActive(int number, bool active);

    // Money operations (amounts are in integer paisa: 100 paisa = 1.00 unit)
    Result deposit(int number, long long paisa);
    Result withdraw(int number, long long paisa);
    // If expectedReceiverName is non-empty, the receiver's account holder name must match it.
    Result transfer(int from, int to, long long paisa, const QString& expectedReceiverName = QString());

    // Input validation helpers
    static QString validateName(const QString& name); // empty string = valid
    static bool namesMatch(const QString& a, const QString& b);
    // [MAX TRANSACTION LIMIT] 10,000,000,000 paisa = 100,000,000.00 per transaction
    static constexpr long long MaxAmountPaisa = 10000000000LL;

    // Transaction history (newest first; accountNumber 0 = every account)
    std::vector<Transaction> history(int accountNumber = 0) const;

    // Deposit requests: customers request, an admin approves (money is credited) or rejects.
    Result requestDeposit(int accountNumber, long long paisa);
    Result approveDeposit(int requestId);
    Result rejectDeposit(int requestId);
    std::vector<DepositRequest> depositRequests(int accountNumber = 0) const; // newest first
    int pendingDepositCount() const;

    // Loans: customers apply, admin approves (credits account) or rejects, customer repays from balance.
    Result requestLoan(int accountNumber, long long paisa, const QString& purpose);
    Result approveLoan(int loanId);
    Result rejectLoan(int loanId);
    Result repayLoan(int accountNumber, int loanId, long long paisa);
    std::vector<LoanRequest> loanRequests(int accountNumber = 0) const; // newest first
    int pendingLoanCount() const;
    long long activeLoanDebt(int accountNumber = 0) const; // 0 = total across bank, >0 = specific account

    // Customer login management, security questions & authentication
    static QStringList securityQuestions(); // Returns the 3 standard security questions
    bool hasLogin(int accountNumber) const;
    bool hasSecurityAnswers(int accountNumber) const;
    QString loginName(int accountNumber) const;
    Result setCustomerLogin(int accountNumber, const QString& username, const QString& password,
                            const QString& ans1 = QString(), const QString& ans2 = QString(),
                            const QString& ans3 = QString());
    Result setSecurityAnswers(int accountNumber, const QString& ans1, const QString& ans2, const QString& ans3);
    Result resetPasswordWithSecurity(const QString& username, const QString& ans1, const QString& ans2,
                                     const QString& ans3, const QString& newPassword);
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
        QString ans1Hash, ans2Hash, ans3Hash; // Salted SHA-256 hashes of the 3 security question answers
    };

    std::vector<Account> accountList;
    std::vector<Credential> credentials;
    std::vector<DepositRequest> deposits;
    std::vector<LoanRequest> loans;
    const std::string accountsFile = "data/accounts.txt";
    const std::string transactionsFile = "data/transactions.txt";
    const QString customersFile = "data/customers.txt";
    const QString depositsFile = "data/deposit_requests.txt";
    const QString loansFile = "data/loans.txt";
    void loadDeposits();
    bool saveDeposits() const;
    DepositRequest* findDeposit(int id);
    void loadLoans();
    bool saveLoans() const;
    LoanRequest* findLoan(int id);

    mutable int txnCounter = 0;
    Account* findMutable(int number);
    Transaction makeTransaction(int account, const std::string& type, long long paisa, int related = 0) const;
    bool saveAccounts();
    bool saveFinancial(const std::vector<Transaction>& txns);
    void loadCredentials();
    bool saveCredentials() const;
    static QString hashPassword(const QString& salt, const QString& password);
    static QString normalizeAnswer(const QString& answer);
    static QString makeSalt();
    static QString validatePassword(const QString& password);
};
