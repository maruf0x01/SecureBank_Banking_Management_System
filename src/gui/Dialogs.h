// ============================================================================
// FILE: Dialogs.h
// PURPOSE: Declarations for reusable UI helpers (buttons, tables, money format)
//          and all pop-up form dialogs (New/Edit Account, Deposit/Withdraw
//          amount, Transfer Money, Loan Application/Repayment, Set Login,
//          Change Password, Security Questions, Forgot Password, Registration).
// ============================================================================
#pragma once

#include <QString>
#include <QStringList>
#include <vector>

#include "BankService.h"

class QWidget;
class QTableWidget;
class QPushButton;

// Formats integer paisa into a decimal money string (e.g., 125050 -> "1250.50")
QString money(long long paisa);

// Creates a styled, read-only table widget with the given column headers
QTableWidget* makeTable(const QStringList& headers);

// Creates a styled QPushButton (primary = Blue button, danger = Red button)
QPushButton* makeButton(const QString& text, bool primary = false, bool danger = false);

// Fills a transaction table. Accounts are shown as "Holder name (account number)".
void fillHistory(QTableWidget* table, const BankService& bank, const std::vector<Transaction>& txns,
                 bool showAccountColumn);

// Pop-up dialog to create or edit an account (Name, Phone, Account Type)
bool askAccountDetails(QWidget* parent, const QString& title, QString& name, QString& phone,
                       QString& type, bool editing);

// Pop-up dialog to enter an amount (and optionally a password for customer withdrawals)
bool askAmount(QWidget* parent, const QString& title, const QString& summary, long long& paisa,
               QString* password = nullptr);

// Pop-up dialog to transfer money. If fixedFrom > 0 (customer), sender is locked and receiver name is verified.
bool askTransfer(QWidget* parent, const BankService& bank, int fixedFrom, int presetFrom,
                 int& from, int& to, long long& paisa, QString& receiverName);

// Pop-up dialog for a customer to apply for a loan (Purpose dropdown + Amount)
bool askLoanRequest(QWidget* parent, long long& paisa, QString& purpose);

// Pop-up dialog for a customer to repay an active loan (partially or in full)
bool askLoanRepayment(QWidget* parent, int loanId, long long remainingPaisa, long long balancePaisa,
                      long long& paisa);

// Pop-up dialog for admin to set or reset a customer's login username and password
bool askCredentials(QWidget* parent, const QString& title, const QString& info, QString& username,
                    QString& password, bool lockUsername);

// Pop-up dialog for a customer to change their password
bool askChangePassword(QWidget* parent, QString& oldPassword, QString& newPassword);

// Pop-up dialog for a customer to set/update their 3 security question answers
bool askSecurityQuestions(QWidget* parent, QString& ans1, QString& ans2, QString& ans3);

// Pop-up dialog on the login screen to reset a forgotten password using the 3 security questions
bool askForgotPassword(QWidget* parent, BankService& bank, QString& usernameOut);

// Pop-up dialog for a new customer to self-register a bank account (includes 3 security questions)
bool askRegistration(QWidget* parent, QString& name, QString& phone, QString& type,
                     QString& username, QString& password,
                     QString& ans1, QString& ans2, QString& ans3);
