#pragma once

#include <QString>
#include <QStringList>
#include <vector>

#include "BankService.h"

class QWidget;
class QTableWidget;
class QPushButton;

QString money(long long paisa);
QTableWidget* makeTable(const QStringList& headers);
QPushButton* makeButton(const QString& text, bool primary = false, bool danger = false);
// Fills a transaction table. Accounts are shown as "Holder name (account number)".
void fillHistory(QTableWidget* table, const BankService& bank, const std::vector<Transaction>& txns,
                 bool showAccountColumn);

bool askAccountDetails(QWidget* parent, const QString& title, QString& name, QString& phone,
                       QString& type, bool editing);
// If password is non-null, the dialog also asks for the user's password (returned in *password).
bool askAmount(QWidget* parent, const QString& title, const QString& summary, long long& paisa,
               QString* password = nullptr);
// fixedFrom > 0: sender is locked (customer). Otherwise admin picks the sender.
bool askTransfer(QWidget* parent, const BankService& bank, int fixedFrom, int presetFrom,
                 int& from, int& to, long long& paisa, QString& receiverName);
bool askCredentials(QWidget* parent, const QString& title, const QString& info, QString& username,
                    QString& password, bool lockUsername);
bool askChangePassword(QWidget* parent, QString& oldPassword, QString& newPassword);
bool askRegistration(QWidget* parent, QString& name, QString& phone, QString& type,
                     QString& username, QString& password);
