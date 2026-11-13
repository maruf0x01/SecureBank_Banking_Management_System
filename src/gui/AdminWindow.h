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
    void loggedOut();

private:
    BankService& bank;

    QStackedWidget* pages;
    QListWidget* sidebar;

    // dashboard
    QLabel *totalAccountsValue, *activeValue, *balanceValue, *loginsValue;
    QLabel *depositsValue, *withdrawalsValue, *transfersValue;
    QTableWidget* recentTable;
    QLabel* typeBreakdown;

    // accounts
    QLineEdit* searchEdit;
    QComboBox* statusFilter;
    QTableWidget* accountTable;
    QAction *newAct, *editAct, *depositAct, *withdrawAct, *transferAct, *toggleAct, *loginAct,
        *removeLoginAct, *deleteAct, *historyAct;

    // deposit requests
    QTableWidget* requestTable;
    QComboBox* requestFilter;
    QPushButton *approveBtn, *rejectBtn;
    QWidget* buildRequests();
    void refreshRequests();
    int selectedRequest() const;
    void doApprove();
    void doReject();

    // transactions
    QLineEdit* historyAccount;
    QComboBox* historyType;
    QTableWidget* historyTable;

    QWidget* buildSidebar();
    QWidget* buildDashboard();
    QWidget* buildAccounts();
    QWidget* buildTransactions();

    int selectedAccount() const;
    void refreshAll();
    void refreshDashboard();
    void refreshAccounts();
    void refreshHistory();
    void updateActions();
    void report(const Result& r);
    void showAccountMenu(const QPoint& pos);

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
