// ============================================================================
// FILE: AdminWindow.h
// PURPOSE: Header for the Administrator main window.
//          Contains 4 pages switched via the left sidebar:
//          0 = Dashboard, 1 = Accounts, 2 = Transactions, 3 = Deposit requests.
// ============================================================================
#pragma once

#include <QMainWindow>

#include "BankService.h"

class QTableWidget;
class QLineEdit;
class QComboBox;
class QLabel;
class QAction;
class QStackedWidget;
class QPushButton;
class QListWidget;

class AdminWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit AdminWindow(BankService& bank);

signals:
    void loggedOut(); // Emitted when admin clicks "Sign out" so main_gui.cpp re-opens LoginDialog

private:
    BankService& bank;

    QStackedWidget* pages; // Holds the 4 pages (Dashboard, Accounts, Transactions, Deposit requests)
    QListWidget* sidebar;  // Left navigation menu list

    // --- Page 0: Dashboard widgets ---
    QLabel *totalAccountsValue, *activeValue, *balanceValue, *loginsValue;
    QLabel *depositsValue, *withdrawalsValue, *transfersValue;
    QTableWidget* recentTable;
    QLabel* typeBreakdown;

    // --- Page 1: Accounts widgets & actions ---
    QLineEdit* searchEdit;
    QComboBox* statusFilter;
    QTableWidget* accountTable;
    QAction *newAct, *editAct, *depositAct, *withdrawAct, *transferAct, *toggleAct, *loginAct,
        *removeLoginAct, *deleteAct, *historyAct;

    // --- Page 3: Deposit requests widgets & actions ---
    QTableWidget* requestTable;
    QComboBox* requestFilter;
    QPushButton *approveBtn, *rejectBtn;
    QWidget* buildRequests();
    void refreshRequests();
    int selectedRequest() const;
    void doApprove();
    void doReject();

    // --- Page 2: Transactions widgets ---
    QLineEdit* historyAccount;
    QComboBox* historyType;
    QTableWidget* historyTable;

    // UI page builders
    QWidget* buildSidebar();
    QWidget* buildDashboard();
    QWidget* buildAccounts();
    QWidget* buildTransactions();

    // Data refresh & selection helpers
    int selectedAccount() const;
    void refreshAll();
    void refreshDashboard();
    void refreshAccounts();
    void refreshHistory();
    void updateActions();
    void report(const Result& r);
    void showAccountMenu(const QPoint& pos);

    // Button click handlers for the Accounts page
    void doNew();
    void doEdit();
    void doDeposit();
    void doWithdraw();
    void doTransfer();
    void doToggle();
    void doDelete();
    void doLogin();
    void doRemoveLogin();
    void doHistory();
    void doReload();
};
