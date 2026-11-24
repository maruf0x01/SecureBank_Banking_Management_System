// ============================================================================
// FILE: CustomerWindow.h
// PURPOSE: Header for the Customer main window.
//          Contains 5 pages switched via the left sidebar:
//          0 = Overview, 1 = Transactions, 2 = Deposit requests, 3 = Loans, 4 = Profile & security.
// ============================================================================
#pragma once

#include <QMainWindow>

#include "BankService.h"

class QLabel;
class QTableWidget;
class QStackedWidget;
class QListWidget;
class QComboBox;
class QPushButton;

class CustomerWindow : public QMainWindow
{
    Q_OBJECT
public:
    CustomerWindow(BankService& bank, int accountNumber);

signals:
    void loggedOut(); // Emitted when customer clicks "Sign out" or fails password 3 times

private:
    BankService& bank;
    int accountNumber;       // The logged-in customer's account number
    int failedPasswords = 0; // Counts wrong password entries during withdrawals (3 = sign out)

    QStackedWidget* pages;
    QListWidget* sidebar;

    // --- Page 0: Overview widgets ---
    QLabel *welcomeLabel, *balanceLabel, *accountLabel, *statusLabel;
    QTableWidget* recentTable;

    // --- Page 1: Transactions widgets ---
    QTableWidget* historyTable;
    QComboBox* historyType;

    // --- Page 2: Deposit requests widgets ---
    QTableWidget* requestTable;
    QWidget* buildRequests();
    void doRequestDeposit();

    // --- Page 3: Loans widgets ---
    QTableWidget* loanTable;
    QPushButton* repayLoanBtn;
    QLabel* loanSummaryLabel;
    QWidget* buildLoans();
    int selectedLoanId() const;
    void doRequestLoan();
    void doRepayLoan();

    // --- Page 4: Profile & security widgets ---
    QLabel *profileName, *profileAccount, *profileType, *profilePhone, *profileStatus;

    // UI page builders
    QWidget* buildSidebar();
    QWidget* buildOverview();
    QWidget* buildHistory();
    QWidget* buildProfile();

    // Refresh & action handlers
    void refresh();
    void report(const Result& r);
    void doWithdraw();
    void doTransfer();
    void doChangePhone();
    void doChangePassword();
};
