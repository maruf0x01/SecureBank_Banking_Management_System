// ============================================================================
// FILE: BankService.cpp
// PURPOSE: Implements all business rules, input validations, password hashing,
//          deposit request workflow, loan workflow, and file persistence.
// ============================================================================
#include "BankService.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTextStream>

#include <algorithm>
#include <chrono>
#include <ctime>

BankService::BankService()
{
    reload();
}

// Reloads accounts, customer logins, deposit requests, and loans from the data/ files.
void BankService::reload()
{
    accountList = FileManager::loadAccounts(accountsFile);
    loadCredentials();
    loadDeposits();
    loadLoans();
}

const Account* BankService::find(int number) const
{
    for(const Account& a : accountList)
        if(a.getAccountNumber() == number)
            return &a;
    return nullptr;
}

Account* BankService::findMutable(int number)
{
    for(Account& a : accountList)
        if(a.getAccountNumber() == number)
            return &a;
    return nullptr;
}

bool BankService::saveAccounts()
{
    return FileManager::saveAccounts(accountsFile, accountList);
}

bool BankService::saveFinancial(const std::vector<Transaction>& txns)
{
    return FileManager::saveAccountsAndTransactions(accountsFile, transactionsFile, accountList, txns);
}

// Builds a new Transaction record with a unique ID and current timestamp.
Transaction BankService::makeTransaction(int account, const std::string& type, long long paisa, int related) const
{
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string timeText = std::ctime(&now);
    timeText.erase(std::remove(timeText.begin(), timeText.end(), '\n'), timeText.end());
    // The counter keeps IDs unique when several transactions happen within the same second.
    std::string id = "TXN" + std::to_string(now) + std::to_string(account) + "-" + std::to_string(++txnCounter);
    return {id, account, type, paisa, timeText, related};
}

// [PHONE VALIDATION] Must be 7 to 15 digits:
QString BankService::validatePhone(const QString& phone)
{
    static const QRegularExpression digits("^[0-9]{7,15}$");
    return digits.match(phone).hasMatch() ? QString() : "Phone number must be 7 to 15 digits only.";
}

// [NAME VALIDATION] 1 to 50 characters; letters, spaces, '.', '-', and ''' only:
QString BankService::validateName(const QString& name)
{
    static const QRegularExpression re("^[\\p{L}][\\p{L} .'-]*$");
    QString n = name.trimmed();
    if(n.isEmpty() || n.size() > 50)
        return "Name must be 1-50 characters.";
    if(!re.match(n).hasMatch())
        return "Name may contain only letters, spaces, '.', '-' and apostrophes.";
    return QString();
}

// Compares two customer names (case-insensitive, ignoring extra spaces).
bool BankService::namesMatch(const QString& a, const QString& b)
{
    return a.simplified().compare(b.simplified(), Qt::CaseInsensitive) == 0;
}

// Parses a decimal string into integer paisa and checks 0 < paisa <= MaxAmountPaisa.
bool BankService::parseAmount(const QString& text, long long& paisa)
{
    try
    {
        paisa = parseMoneyToPaisa(text.trimmed().toStdString());
        return paisa > 0 && paisa <= MaxAmountPaisa;
    }
    catch(...)
    {
        return false;
    }
}

// ---------- Account Operations ----------

// Creates a new Active account directly (used by Admin).
Result BankService::createAccount(const QString& name, const QString& phone, const QString& type)
{
    QString n = name.trimmed();
    if(QString e = validateName(n); !e.isEmpty())
        return {false, e};
    if(QString e = validatePhone(phone); !e.isEmpty())
        return {false, e};

    int next = 100001;
    for(const Account& a : accountList)
        next = std::max(next, a.getAccountNumber() + 1);

    accountList.emplace_back(next, n.toStdString(), phone.toStdString(), type.toStdString(), 0, "Active");
    if(!saveAccounts())
    {
        accountList.pop_back();
        return {false, "Unable to save account data."};
    }
    return {true, QString("Account %1 created for %2.").arg(next).arg(n), next};
}

// Self-registers a new Pending account + login (used by Customer from the login window).
Result BankService::registerAccount(const QString& name, const QString& phone, const QString& type,
                                    const QString& username, const QString& password)
{
    static const QRegularExpression userRe("^[A-Za-z0-9_.]{3,20}$");
    QString n = name.trimmed();
    if(QString e = validateName(n); !e.isEmpty())
        return {false, e};
    if(QString e = validatePhone(phone); !e.isEmpty())
        return {false, e};
    if(!userRe.match(username).hasMatch())
        return {false, "Username must be 3-20 letters, digits, '_' or '.'."};
    if(QString e = validatePassword(password); !e.isEmpty())
        return {false, e};
    for(const Credential& c : credentials)
        if(c.username.compare(username, Qt::CaseInsensitive) == 0)
            return {false, "That username is already taken."};

    int next = 100001;
    for(const Account& a : accountList)
        next = std::max(next, a.getAccountNumber() + 1);

    accountList.emplace_back(next, n.toStdString(), phone.toStdString(), type.toStdString(), 0, "Pending");
    if(!saveAccounts())
    {
        accountList.pop_back();
        return {false, "Unable to save account data."};
    }
    Result login = setCustomerLogin(next, username, password);
    if(!login.ok)
    {
        accountList.pop_back();
        saveAccounts();
        return login;
    }
    return {true, QString("Account %1 created. It must be activated by an administrator before you can sign in.").arg(next), next};
}

// Updates customer name, phone number, and account type.
Result BankService::updateAccount(int number, const QString& name, const QString& phone, const QString& type)
{
    Account* a = findMutable(number);
    if(!a)
        return {false, "Account not found."};
    QString n = name.trimmed();
    if(QString e = validateName(n); !e.isEmpty())
        return {false, e};
    if(QString e = validatePhone(phone); !e.isEmpty())
        return {false, e};

    Account backup = *a;
    a->setCustomerName(n.toStdString());
    a->setPhoneNumber(phone.toStdString());
    a->setAccountType(type.toStdString());
    if(!saveAccounts())
    {
        *a = backup;
        return {false, "Unable to save account data."};
    }
    return {true, "Account updated."};
}

// Permanently deletes an account (requires 0 balance and 0 unpaid loan), its login, deposits, and loans.
Result BankService::deleteAccount(int number)
{
    auto it = std::find_if(accountList.begin(), accountList.end(),
                           [&](const Account& a) { return a.getAccountNumber() == number; });
    if(it == accountList.end())
        return {false, "Account not found."};
    if(it->getBalancePaisa() != 0)
        return {false, "This account still has a balance of " +
                           QString::fromStdString(formatPaisa(it->getBalancePaisa())) +
                           ". Withdraw or transfer the money before deleting it."};
    long long debt = activeLoanDebt(number);
    if(debt > 0)
        return {false, "This account has an unpaid active loan of " +
                           QString::fromStdString(formatPaisa(debt)) +
                           ". The loan must be repaid before deleting the account."};

    std::vector<Account> accountsBackup = accountList;
    std::vector<Credential> credBackup = credentials;
    std::vector<DepositRequest> depBackup = deposits;
    std::vector<LoanRequest> loanBackup = loans;

    accountList.erase(it);
    credentials.erase(std::remove_if(credentials.begin(), credentials.end(),
                                     [&](const Credential& c) { return c.accountNumber == number; }),
                      credentials.end());
    deposits.erase(std::remove_if(deposits.begin(), deposits.end(),
                                  [&](const DepositRequest& d) { return d.accountNumber == number; }),
                   deposits.end());
    loans.erase(std::remove_if(loans.begin(), loans.end(),
                               [&](const LoanRequest& l) { return l.accountNumber == number; }),
                loans.end());

    if(!saveAccounts() || !saveCredentials() || !saveDeposits() || !saveLoans())
    {
        accountList = accountsBackup;
        credentials = credBackup;
        deposits = depBackup;
        loans = loanBackup;
        saveAccounts();
        saveCredentials();
        saveDeposits();
        saveLoans();
        return {false, "Unable to delete the account (could not save data)."};
    }
    return {true, QString("Account %1 deleted.").arg(number)};
}

// Activates a Pending account, reopens a Closed account, or closes an Active account.
Result BankService::setActive(int number, bool active)
{
    Account* a = findMutable(number);
    if(!a)
        return {false, "Account not found."};
    std::string wanted = active ? "Active" : "Closed";
    if(a->getStatus() == wanted)
        return {false, active ? "Account is already active." : "Account is already closed."};
    if(!active && activeLoanDebt(number) > 0)
        return {false, "Cannot close this account while it has an unpaid loan of " +
                           QString::fromStdString(formatPaisa(activeLoanDebt(number))) + "."};
    bool wasPending = a->getStatus() == "Pending";
    std::string old = a->getStatus();
    a->setStatus(wanted);
    if(!saveAccounts())
    {
        a->setStatus(old);
        return {false, "Unable to save account data."};
    }
    return {true, active ? (wasPending ? "Account activated." : "Account reopened.") : "Account closed."};
}

// Immediate deposit (used by Admin and by approveDeposit).
Result BankService::deposit(int number, long long paisa)
{
    if(paisa <= 0 || paisa > MaxAmountPaisa)
        return {false, "Invalid amount."};
    Account* a = findMutable(number);
    if(!a)
        return {false, "Account not found."};
    if(a->getStatus() != "Active")
        return {false, "Transaction denied. Account is not active."};
    if(!a->deposit(paisa))
        return {false, "Deposit failed (invalid amount or balance overflow)."};
    if(!saveFinancial({makeTransaction(number, "Deposit", paisa)}))
    {
        a->withdraw(paisa);
        return {false, "Unable to save account data."};
    }
    return {true, "Deposit successful. New balance: " + QString::fromStdString(formatPaisa(a->getBalancePaisa()))};
}

// Immediate withdrawal from an Active account.
Result BankService::withdraw(int number, long long paisa)
{
    if(paisa <= 0 || paisa > MaxAmountPaisa)
        return {false, "Invalid amount."};
    Account* a = findMutable(number);
    if(!a)
        return {false, "Account not found."};
    if(a->getStatus() != "Active")
        return {false, "Transaction denied. Account is not active."};
    if(!a->withdraw(paisa))
        return {false, "Withdrawal denied. Insufficient balance."};
    if(!saveFinancial({makeTransaction(number, "Withdrawal", paisa)}))
    {
        a->deposit(paisa);
        return {false, "Unable to save account data."};
    }
    return {true, "Withdrawal successful. New balance: " + QString::fromStdString(formatPaisa(a->getBalancePaisa()))};
}

// Transfers money between two Active accounts (verifies receiver name if expectedReceiverName is provided).
Result BankService::transfer(int from, int to, long long paisa, const QString& expectedReceiverName)
{
    if(paisa <= 0 || paisa > MaxAmountPaisa)
        return {false, "Invalid transfer amount."};
    if(from == to)
        return {false, "Sender and receiver accounts must be different."};
    Account* s = findMutable(from);
    Account* r = findMutable(to);
    if(!s || !r)
        return {false, "Receiver account was not found."};
    if(!expectedReceiverName.isEmpty() &&
       !namesMatch(expectedReceiverName, QString::fromStdString(r->getCustomerName())))
        return {false, "The account number and account holder name do not match. Transfer cancelled."};
    if(s->getStatus() != "Active" || r->getStatus() != "Active")
        return {false, "Both accounts must be active."};
    if(!s->withdraw(paisa))
        return {false, "Insufficient balance in sender account."};
    if(!r->deposit(paisa))
    {
        s->deposit(paisa);
        return {false, "Transfer failed (receiver balance overflow)."};
    }
    if(!saveFinancial({makeTransaction(from, "Transfer Sent", paisa, to),
                       makeTransaction(to, "Transfer Received", paisa, from)}))
    {
        s->deposit(paisa);
        r->withdraw(paisa);
        return {false, "Unable to save account data. Transfer cancelled."};
    }
    return {true, "Transfer successful."};
}

// Returns transactions in newest-first order (accountNumber == 0 returns all transactions).
std::vector<Transaction> BankService::history(int accountNumber) const
{
    std::vector<Transaction> all = FileManager::loadTransactions(transactionsFile);
    std::vector<Transaction> out;
    for(auto it = all.rbegin(); it != all.rend(); ++it)
        if(accountNumber == 0 || it->accountNumber == accountNumber)
            out.push_back(*it);
    return out;
}

// ---------- Customer Logins (salted SHA-256 hashes in data/customers.txt) ----------

QString BankService::makeSalt()
{
    QByteArray bytes;
    for(int i = 0; i < 16; i++)
        bytes.append(static_cast<char>(QRandomGenerator::system()->bounded(256)));
    return bytes.toHex();
}

QString BankService::hashPassword(const QString& salt, const QString& password)
{
    return QCryptographicHash::hash((salt + password).toUtf8(), QCryptographicHash::Sha256).toHex();
}

// [PASSWORD VALIDATION] 6 to 64 characters, no spaces or '|':
QString BankService::validatePassword(const QString& password)
{
    if(password.size() < 6)
        return "Password must be at least 6 characters.";
    if(password.size() > 64)
        return "Password cannot be longer than 64 characters.";
    if(password.contains('|') || password.contains('\n') || password.contains(' '))
        return "Password cannot contain spaces or '|'.";
    return QString();
}

void BankService::loadCredentials()
{
    credentials.clear();
    QFile file(customersFile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    while(!in.atEnd())
    {
        QStringList p = in.readLine().split('|');
        bool ok = false;
        if(p.size() != 4)
            continue;
        int number = p[0].toInt(&ok);
        if(ok && number > 0)
            credentials.push_back({number, p[1], p[2], p[3]});
    }
}

bool BankService::saveCredentials() const
{
    QSaveFile file(customersFile);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream out(&file);
    for(const Credential& c : credentials)
        out << c.accountNumber << '|' << c.username << '|' << c.salt << '|' << c.hash << '\n';
    out.flush();
    return file.commit();
}

bool BankService::hasLogin(int accountNumber) const
{
    return !loginName(accountNumber).isEmpty();
}

QString BankService::loginName(int accountNumber) const
{
    for(const Credential& c : credentials)
        if(c.accountNumber == accountNumber)
            return c.username;
    return QString();
}

Result BankService::setCustomerLogin(int accountNumber, const QString& username, const QString& password)
{
    static const QRegularExpression userRe("^[A-Za-z0-9_.]{3,20}$");
    if(!find(accountNumber))
        return {false, "Account not found."};
    if(!userRe.match(username).hasMatch())
        return {false, "Username must be 3-20 letters, digits, '_' or '.'."};
    if(QString e = validatePassword(password); !e.isEmpty())
        return {false, e};
    for(const Credential& c : credentials)
        if(c.accountNumber != accountNumber && c.username.compare(username, Qt::CaseInsensitive) == 0)
            return {false, "That username is already taken."};

    std::vector<Credential> backup = credentials;
    QString salt = makeSalt();
    Credential cred{accountNumber, username, salt, hashPassword(salt, password)};
    auto it = std::find_if(credentials.begin(), credentials.end(),
                           [&](const Credential& c) { return c.accountNumber == accountNumber; });
    if(it != credentials.end())
        *it = cred;
    else
        credentials.push_back(cred);
    if(!saveCredentials())
    {
        credentials = backup;
        return {false, "Unable to save customer login."};
    }
    return {true, QString("Customer login saved for account %1 (username: %2).").arg(accountNumber).arg(username)};
}

Result BankService::removeCustomerLogin(int accountNumber)
{
    std::vector<Credential> backup = credentials;
    credentials.erase(std::remove_if(credentials.begin(), credentials.end(),
                                     [&](const Credential& c) { return c.accountNumber == accountNumber; }),
                      credentials.end());
    if(credentials.size() == backup.size())
        return {false, "This account has no customer login."};
    if(!saveCredentials())
    {
        credentials = backup;
        return {false, "Unable to save customer logins."};
    }
    return {true, "Customer login removed."};
}

int BankService::authenticate(const QString& username, const QString& password) const
{
    for(const Credential& c : credentials)
        if(c.username.compare(username, Qt::CaseInsensitive) == 0)
        {
            if(hashPassword(c.salt, password) != c.hash)
                return -1;
            const Account* a = find(c.accountNumber);
            if(!a)
                return -1;
            if(a->getStatus() == "Pending")
                return -3;
            return a->getStatus() == "Active" ? c.accountNumber : -2;
        }
    return -1;
}

bool BankService::verifyPassword(int accountNumber, const QString& password) const
{
    for(const Credential& c : credentials)
        if(c.accountNumber == accountNumber)
            return hashPassword(c.salt, password) == c.hash;
    return false;
}

Result BankService::changePassword(int accountNumber, const QString& oldPassword, const QString& newPassword)
{
    for(Credential& c : credentials)
        if(c.accountNumber == accountNumber)
        {
            if(hashPassword(c.salt, oldPassword) != c.hash)
                return {false, "Current password is incorrect."};
            if(QString e = validatePassword(newPassword); !e.isEmpty())
                return {false, e};
            Credential backup = c;
            c.salt = makeSalt();
            c.hash = hashPassword(c.salt, newPassword);
            if(!saveCredentials())
            {
                c = backup;
                return {false, "Unable to save password."};
            }
            return {true, "Password changed."};
        }
    return {false, "No login found for this account."};
}

// ---------- Deposit Requests (stored in data/deposit_requests.txt) ----------

static QString nowText()
{
    return QDateTime::currentDateTime().toString("ddd MMM d hh:mm:ss yyyy");
}

void BankService::loadDeposits()
{
    deposits.clear();
    QFile file(depositsFile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    while(!in.atEnd())
    {
        QStringList p = in.readLine().split('|');
        if(p.size() != 6)
            continue;
        bool ok1, ok2, ok3;
        DepositRequest d;
        d.id = p[0].toInt(&ok1);
        d.accountNumber = p[1].toInt(&ok2);
        d.amountPaisa = p[2].toLongLong(&ok3);
        d.requestedAt = p[3];
        d.status = p[4];
        d.decidedAt = p[5];
        if(ok1 && ok2 && ok3 && d.id > 0 && d.amountPaisa > 0 &&
           (d.status == "Pending" || d.status == "Approved" || d.status == "Rejected"))
            deposits.push_back(d);
    }
}

bool BankService::saveDeposits() const
{
    QSaveFile file(depositsFile);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream out(&file);
    for(const DepositRequest& d : deposits)
        out << d.id << '|' << d.accountNumber << '|' << d.amountPaisa << '|' << d.requestedAt << '|'
            << d.status << '|' << d.decidedAt << '\n';
    out.flush();
    return file.commit();
}

DepositRequest* BankService::findDeposit(int id)
{
    for(DepositRequest& d : deposits)
        if(d.id == id)
            return &d;
    return nullptr;
}

Result BankService::requestDeposit(int accountNumber, long long paisa)
{
    const Account* a = find(accountNumber);
    if(!a)
        return {false, "Account not found."};
    if(a->getStatus() != "Active")
        return {false, "Your account is not active."};
    if(paisa <= 0 || paisa > MaxAmountPaisa)
        return {false, "Invalid amount."};
    int pending = 0, nextId = 1;
    for(const DepositRequest& d : deposits)
    {
        nextId = std::max(nextId, d.id + 1);
        if(d.accountNumber == accountNumber && d.status == "Pending")
            pending++;
    }
    if(pending >= 10)
        return {false, "You already have 10 pending deposit requests. Please wait for approval."};

    deposits.push_back({nextId, accountNumber, paisa, nowText(), "Pending", ""});
    if(!saveDeposits())
    {
        deposits.pop_back();
        return {false, "Unable to save the deposit request."};
    }
    return {true, "Deposit request submitted. It will be credited once an administrator approves it.", nextId};
}

Result BankService::approveDeposit(int requestId)
{
    DepositRequest* d = findDeposit(requestId);
    if(!d)
        return {false, "Deposit request not found."};
    if(d->status != "Pending")
        return {false, "This request has already been " + d->status.toLower() + "."};
    const Account* a = find(d->accountNumber);
    if(!a || a->getStatus() != "Active")
        return {false, "The account is not active, so the deposit cannot be approved."};

    DepositRequest backup = *d;
    d->status = "Approved";
    d->decidedAt = nowText();
    if(!saveDeposits())
    {
        *d = backup;
        return {false, "Unable to save the deposit request."};
    }
    Result r = deposit(d->accountNumber, d->amountPaisa);
    if(!r.ok)
    {
        *d = backup;
        saveDeposits();
        return r;
    }
    return {true, QString("Deposit of %1 approved and credited to account %2.")
                      .arg(QString::fromStdString(formatPaisa(backup.amountPaisa))).arg(backup.accountNumber)};
}

Result BankService::rejectDeposit(int requestId)
{
    DepositRequest* d = findDeposit(requestId);
    if(!d)
        return {false, "Deposit request not found."};
    if(d->status != "Pending")
        return {false, "This request has already been " + d->status.toLower() + "."};
    DepositRequest backup = *d;
    d->status = "Rejected";
    d->decidedAt = nowText();
    if(!saveDeposits())
    {
        *d = backup;
        return {false, "Unable to save the deposit request."};
    }
    return {true, "Deposit request rejected."};
}

std::vector<DepositRequest> BankService::depositRequests(int accountNumber) const
{
    std::vector<DepositRequest> out;
    for(auto it = deposits.rbegin(); it != deposits.rend(); ++it)
        if(accountNumber == 0 || it->accountNumber == accountNumber)
            out.push_back(*it);
    return out;
}

int BankService::pendingDepositCount() const
{
    int n = 0;
    for(const DepositRequest& d : deposits)
        if(d.status == "Pending")
            n++;
    return n;
}

// ---------- Loans (stored in data/loans.txt) ----------

void BankService::loadLoans()
{
    loans.clear();
    QFile file(loansFile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    while(!in.atEnd())
    {
        QStringList p = in.readLine().split('|');
        if(p.size() != 8)
            continue;
        bool ok1, ok2, ok3, ok4;
        LoanRequest l;
        l.id = p[0].toInt(&ok1);
        l.accountNumber = p[1].toInt(&ok2);
        l.amountPaisa = p[2].toLongLong(&ok3);
        l.remainingPaisa = p[3].toLongLong(&ok4);
        l.purpose = p[4];
        l.requestedAt = p[5];
        l.status = p[6];
        l.decidedAt = p[7];
        if(ok1 && ok2 && ok3 && ok4 && l.id > 0 && l.amountPaisa > 0 && l.remainingPaisa >= 0 &&
           (l.status == "Pending" || l.status == "Active" || l.status == "Rejected" || l.status == "Paid"))
            loans.push_back(l);
    }
}

bool BankService::saveLoans() const
{
    QSaveFile file(loansFile);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream out(&file);
    for(const LoanRequest& l : loans)
        out << l.id << '|' << l.accountNumber << '|' << l.amountPaisa << '|' << l.remainingPaisa << '|'
            << l.purpose << '|' << l.requestedAt << '|' << l.status << '|' << l.decidedAt << '\n';
    out.flush();
    return file.commit();
}

LoanRequest* BankService::findLoan(int id)
{
    for(LoanRequest& l : loans)
        if(l.id == id)
            return &l;
    return nullptr;
}

// Customer submits a new loan application (status starts as "Pending").
Result BankService::requestLoan(int accountNumber, long long paisa, const QString& purpose)
{
    const Account* a = find(accountNumber);
    if(!a)
        return {false, "Account not found."};
    if(a->getStatus() != "Active")
        return {false, "Your account is not active."};
    if(paisa <= 0 || paisa > MaxAmountPaisa)
        return {false, "Invalid loan amount."};
    QString cleanPurpose = purpose.trimmed();
    if(cleanPurpose.isEmpty() || cleanPurpose.size() > 40 || cleanPurpose.contains('|'))
        return {false, "Please select a valid loan purpose."};

    int openCount = 0, nextId = 1;
    for(const LoanRequest& l : loans)
    {
        nextId = std::max(nextId, l.id + 1);
        if(l.accountNumber == accountNumber && (l.status == "Pending" || l.status == "Active"))
            openCount++;
    }
    if(openCount >= 5)
        return {false, "You already have 5 pending or active loans. Please repay existing loans first."};

    loans.push_back({nextId, accountNumber, paisa, paisa, cleanPurpose, nowText(), "Pending", ""});
    if(!saveLoans())
    {
        loans.pop_back();
        return {false, "Unable to save the loan request."};
    }
    return {true, "Loan application submitted. It will be credited once an administrator approves it.", nextId};
}

// Admin approves a Pending loan: status becomes "Active" and the money is credited to the customer's balance.
Result BankService::approveLoan(int loanId)
{
    LoanRequest* l = findLoan(loanId);
    if(!l)
        return {false, "Loan request not found."};
    if(l->status != "Pending")
        return {false, "This loan request has already been " + l->status.toLower() + "."};
    Account* a = findMutable(l->accountNumber);
    if(!a || a->getStatus() != "Active")
        return {false, "The account is not active, so the loan cannot be approved."};

    LoanRequest backup = *l;
    l->status = "Active";
    l->remainingPaisa = l->amountPaisa;
    l->decidedAt = nowText();
    if(!saveLoans())
    {
        *l = backup;
        return {false, "Unable to save the loan status."};
    }
    if(!a->deposit(l->amountPaisa))
    {
        *l = backup;
        saveLoans();
        return {false, "Loan approval failed (balance overflow)."};
    }
    if(!saveFinancial({makeTransaction(l->accountNumber, "Loan Disbursed", l->amountPaisa)}))
    {
        a->withdraw(l->amountPaisa);
        *l = backup;
        saveLoans();
        return {false, "Unable to save account data."};
    }
    return {true, QString("Loan of %1 approved and credited to account %2.")
                      .arg(QString::fromStdString(formatPaisa(backup.amountPaisa))).arg(backup.accountNumber)};
}

// Admin rejects a Pending loan application.
Result BankService::rejectLoan(int loanId)
{
    LoanRequest* l = findLoan(loanId);
    if(!l)
        return {false, "Loan request not found."};
    if(l->status != "Pending")
        return {false, "This loan request has already been " + l->status.toLower() + "."};
    LoanRequest backup = *l;
    l->status = "Rejected";
    l->remainingPaisa = 0;
    l->decidedAt = nowText();
    if(!saveLoans())
    {
        *l = backup;
        return {false, "Unable to save the loan status."};
    }
    return {true, "Loan request rejected."};
}

// Customer repays part or all of an Active loan from their account balance.
Result BankService::repayLoan(int accountNumber, int loanId, long long paisa)
{
    if(paisa <= 0 || paisa > MaxAmountPaisa)
        return {false, "Invalid repayment amount."};
    Account* a = findMutable(accountNumber);
    if(!a)
        return {false, "Account not found."};
    if(a->getStatus() != "Active")
        return {false, "Your account is not active."};
    LoanRequest* l = findLoan(loanId);
    if(!l || l->accountNumber != accountNumber)
        return {false, "Loan not found."};
    if(l->status != "Active")
        return {false, "Only active loans can be repaid."};
    if(paisa > l->remainingPaisa)
        return {false, "Repayment amount exceeds the remaining loan balance of " +
                           QString::fromStdString(formatPaisa(l->remainingPaisa)) + "."};
    if(paisa > a->getBalancePaisa())
        return {false, "Insufficient account balance to make this repayment."};

    LoanRequest loanBackup = *l;
    if(!a->withdraw(paisa))
        return {false, "Insufficient account balance."};

    l->remainingPaisa -= paisa;
    if(l->remainingPaisa == 0)
        l->status = "Paid";

    if(!saveLoans())
    {
        a->deposit(paisa);
        *l = loanBackup;
        return {false, "Unable to save loan repayment."};
    }
    if(!saveFinancial({makeTransaction(accountNumber, "Loan Repayment", paisa)}))
    {
        a->deposit(paisa);
        *l = loanBackup;
        saveLoans();
        return {false, "Unable to save account transaction."};
    }

    if(l->remainingPaisa == 0)
        return {true, "Loan fully repaid! Remaining loan balance is 0.00."};
    return {true, "Loan repayment successful. Remaining loan due: " +
                      QString::fromStdString(formatPaisa(l->remainingPaisa))};
}

std::vector<LoanRequest> BankService::loanRequests(int accountNumber) const
{
    std::vector<LoanRequest> out;
    for(auto it = loans.rbegin(); it != loans.rend(); ++it)
        if(accountNumber == 0 || it->accountNumber == accountNumber)
            out.push_back(*it);
    return out;
}

int BankService::pendingLoanCount() const
{
    int n = 0;
    for(const LoanRequest& l : loans)
        if(l.status == "Pending")
            n++;
    return n;
}

long long BankService::activeLoanDebt(int accountNumber) const
{
    long long total = 0;
    for(const LoanRequest& l : loans)
        if(l.status == "Active" && (accountNumber == 0 || l.accountNumber == accountNumber))
            total += l.remainingPaisa;
    return total;
}
