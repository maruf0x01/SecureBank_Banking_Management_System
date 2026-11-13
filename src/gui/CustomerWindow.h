#pragma once

#include <QMainWindow>

#include "BankService.h"

class QLabel;
class QTableWidget;
class QStackedWidget;
class QListWidget;
class QComboBox;

class CustomerWindow : public QMainWindow
{
    Q_OBJECT
public:
    CustomerWindow(BankService& bank, int accountNumber);

signals:
    void loggedOut();

private:
    BankService& bank;
    int accountNumber;
    int failedPasswords = 0;

    QStackedWidget* pages;
    QListWidget* sidebar;
    QLabel *welcomeLabel, *balanceLabel, *accountLabel, *statusLabel;
    QTableWidget* recentTable;
    QTableWidget* historyTable;
    QComboBox* historyType;
    QTableWidget* requestTable;
    QWidget* buildRequests();
    void doRequestDeposit();
    QLabel *profileName, *profileAccount, *profileType, *profilePhone, *profileStatus;

    QWidget* buildSidebar();
    QWidget* buildOverview();
    QWidget* buildHistory();
    QWidget* buildProfile();

    void refresh();
    void report(const Result& r);
    void doWithdraw();
    void doTransfer();
    void doChangePhone();
    void doChangePassword();
};
