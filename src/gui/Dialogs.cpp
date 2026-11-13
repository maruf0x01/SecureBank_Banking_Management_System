#include "Dialogs.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QTableWidget>
#include <QVBoxLayout>

QString money(long long paisa)
{
    return QString::fromStdString(formatPaisa(paisa));
}

static QTableWidgetItem* cell(const QString& text, bool right = false)
{
    auto* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    if(right)
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return item;
}

QTableWidget* makeTable(const QStringList& headers)
{
    auto* t = new QTableWidget(0, headers.size());
    t->setHorizontalHeaderLabels(headers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setAlternatingRowColors(true);
    t->setShowGrid(false);
    t->verticalHeader()->setVisible(false);
    t->verticalHeader()->setDefaultSectionSize(34);
    t->horizontalHeader()->setStretchLastSection(true);
    t->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    return t;
}

QPushButton* makeButton(const QString& text, bool primary, bool danger)
{
    auto* b = new QPushButton(text);
    b->setCursor(Qt::PointingHandCursor);
    if(primary)
        b->setProperty("primary", true);
    if(danger)
        b->setProperty("danger", true);
    return b;
}

static QString holderLabel(const BankService& bank, int accountNumber)
{
    const Account* a = bank.find(accountNumber);
    return QString("%1 (%2)").arg(a ? QString::fromStdString(a->getCustomerName()) : "Deleted account")
        .arg(accountNumber);
}

void fillHistory(QTableWidget* table, const BankService& bank, const std::vector<Transaction>& txns,
                 bool showAccountColumn)
{
    table->setRowCount(0);
    table->setColumnCount(showAccountColumn ? 6 : 5);
    QStringList headers = {"Date", "Type", "Amount", "Other party", "Transaction ID"};
    if(showAccountColumn)
        headers.insert(0, "Account holder");
    table->setHorizontalHeaderLabels(headers);

    for(const Transaction& t : txns)
    {
        int row = table->rowCount();
        table->insertRow(row);
        int c = 0;
        if(showAccountColumn)
            table->setItem(row, c++, cell(holderLabel(bank, t.accountNumber)));
        table->setItem(row, c++, cell(QString::fromStdString(t.dateTime)));
        table->setItem(row, c++, cell(QString::fromStdString(t.type)));
        bool incoming = t.type == "Deposit" || t.type == "Transfer Received";
        auto* amount = cell((incoming ? "+ " : "- ") + money(t.amountPaisa), true);
        amount->setForeground(incoming ? QColor("#15803d") : QColor("#b91c1c"));
        table->setItem(row, c++, amount);
        table->setItem(row, c++, cell(t.relatedAccountNumber ? holderLabel(bank, t.relatedAccountNumber) : "-"));
        table->setItem(row, c++, cell(QString::fromStdString(t.transactionId)));
    }
}

static QLabel* errorLabel()
{
    auto* l = new QLabel;
    l->setStyleSheet("color: #b91c1c;");
    l->setWordWrap(true);
    return l;
}

static QDialogButtonBox* okCancel(QDialog& d, const QString& okText)
{
    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    box->button(QDialogButtonBox::Ok)->setText(okText);
    box->button(QDialogButtonBox::Ok)->setProperty("primary", true);
    QObject::connect(box, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    return box;
}

bool askAccountDetails(QWidget* parent, const QString& title, QString& name, QString& phone,
                       QString& type, bool editing)
{
    QDialog d(parent);
    d.setWindowTitle(title);
    d.setMinimumWidth(380);
    auto* nameEdit = new QLineEdit(name);
    nameEdit->setMaxLength(50);
    nameEdit->setPlaceholderText("Full name");
    auto* phoneEdit = new QLineEdit(phone);
    phoneEdit->setMaxLength(15);
    phoneEdit->setPlaceholderText("e.g. 01712345678");
    phoneEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]*"), &d));
    auto* typeBox = new QComboBox;
    typeBox->addItems({"Savings", "Current", "Student", "Business"});
    if(!type.isEmpty())
        typeBox->setCurrentText(type);
    auto* error = errorLabel();

    auto* form = new QFormLayout;
    form->addRow("Customer name", nameEdit);
    form->addRow("Phone number", phoneEdit);
    form->addRow("Account type", typeBox);
    auto* box = okCancel(d, editing ? "Save changes" : "Create account");
    auto* layout = new QVBoxLayout(&d);
    layout->addLayout(form);
    layout->addWidget(error);
    layout->addWidget(box);

    QObject::connect(box, &QDialogButtonBox::accepted, &d, [&]() {
        QString n = nameEdit->text().trimmed();
        if(QString e = BankService::validateName(n); !e.isEmpty())
            error->setText(e);
        else if(QString e = BankService::validatePhone(phoneEdit->text()); !e.isEmpty())
            error->setText(e);
        else
            d.accept();
    });
    if(d.exec() != QDialog::Accepted)
        return false;
    name = nameEdit->text().trimmed();
    phone = phoneEdit->text();
    type = typeBox->currentText();
    return true;
}

bool askAmount(QWidget* parent, const QString& title, const QString& summary, long long& paisa,
               QString* password)
{
    QDialog d(parent);
    d.setWindowTitle(title);
    d.setMinimumWidth(360);
    auto* info = new QLabel(summary);
    info->setWordWrap(true);
    auto* amountEdit = new QLineEdit;
    amountEdit->setPlaceholderText("0.00");
    amountEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{0,12}([.][0-9]{0,2})?"), &d));
    auto* error = errorLabel();
    auto* box = okCancel(d, title);
    auto* layout = new QVBoxLayout(&d);
    layout->addWidget(info);
    layout->addWidget(new QLabel("Amount"));
    layout->addWidget(amountEdit);
    QLineEdit* passEdit = nullptr;
    if(password)
    {
        passEdit = new QLineEdit;
        passEdit->setEchoMode(QLineEdit::Password);
        passEdit->setMaxLength(64);
        passEdit->setPlaceholderText("Enter your password to confirm");
        layout->addWidget(new QLabel("Password"));
        layout->addWidget(passEdit);
    }
    layout->addWidget(error);
    layout->addWidget(box);
    QObject::connect(box, &QDialogButtonBox::accepted, &d, [&]() {
        if(!BankService::parseAmount(amountEdit->text(), paisa))
            error->setText("Enter a positive amount with up to two decimals.");
        else if(passEdit && passEdit->text().isEmpty())
            error->setText("Enter your password to confirm.");
        else
            d.accept();
    });
    if(d.exec() != QDialog::Accepted)
        return false;
    if(passEdit)
        *password = passEdit->text();
    return true;
}

bool askTransfer(QWidget* parent, const BankService& bank, int fixedFrom, int presetFrom,
                 int& from, int& to, long long& paisa, QString& receiverName)
{
    const bool verifyName = fixedFrom > 0; // customers must confirm the receiver's name
    QDialog d(parent);
    d.setWindowTitle("Transfer money");
    d.setMinimumWidth(420);

    QComboBox* fromBox = nullptr;
    QLabel* fromInfo = new QLabel;
    if(fixedFrom > 0)
    {
        const Account* a = bank.find(fixedFrom);
        fromInfo->setText(QString("<b>%1</b> (%2)<br>Available balance: <b>%3</b>")
                              .arg(QString::fromStdString(a->getCustomerName()))
                              .arg(fixedFrom).arg(money(a->getBalancePaisa())));
    }
    else
    {
        fromBox = new QComboBox;
        for(const Account& a : bank.accounts())
            if(a.getStatus() == "Active")
                fromBox->addItem(QString("%1 - %2 (%3)").arg(a.getAccountNumber())
                                     .arg(QString::fromStdString(a.getCustomerName()))
                                     .arg(money(a.getBalancePaisa())), a.getAccountNumber());
        int idx = fromBox->findData(presetFrom);
        if(idx >= 0)
            fromBox->setCurrentIndex(idx);
    }

    auto* toEdit = new QLineEdit;
    toEdit->setPlaceholderText("Receiver account number");
    toEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{0,9}"), &d));
    auto* receiverInfo = new QLabel;
    receiverInfo->setObjectName("subtle");
    auto* nameEdit = new QLineEdit;
    nameEdit->setMaxLength(50);
    nameEdit->setPlaceholderText("Name of the account holder you intend to pay");
    auto* amountEdit = new QLineEdit;
    amountEdit->setPlaceholderText("0.00");
    amountEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{0,12}([.][0-9]{0,2})?"), &d));
    auto* error = errorLabel();

    QObject::connect(toEdit, &QLineEdit::textChanged, &d, [&]() {
        const Account* a = bank.find(toEdit->text().toInt());
        if(toEdit->text().isEmpty())
            receiverInfo->clear();
        else if(!a)
            receiverInfo->setText("No account with this number");
        else if(verifyName)
            receiverInfo->setText("Account found. Enter the holder's name below to verify.");
        else
            receiverInfo->setText("Receiver: " + QString::fromStdString(a->getCustomerName()));
    });

    auto* form = new QFormLayout;
    if(fromBox)
        form->addRow("From", fromBox);
    else
        form->addRow("From", fromInfo);
    form->addRow("To account", toEdit);
    form->addRow("", receiverInfo);
    if(verifyName)
        form->addRow("Receiver name", nameEdit);
    form->addRow("Amount", amountEdit);
    auto* box = okCancel(d, "Continue");
    auto* layout = new QVBoxLayout(&d);
    layout->addLayout(form);
    layout->addWidget(error);
    layout->addWidget(box);

    QObject::connect(box, &QDialogButtonBox::accepted, &d, [&]() {
        int f = fixedFrom > 0 ? fixedFrom : fromBox->currentData().toInt();
        int t = toEdit->text().toInt();
        long long amt = 0;
        const Account* receiver = bank.find(t);
        if(f <= 0)
            error->setText("No active sender account available.");
        else if(!receiver)
            error->setText("Receiver account was not found.");
        else if(t == f)
            error->setText("Sender and receiver must be different.");
        else if(verifyName && nameEdit->text().trimmed().isEmpty())
            error->setText("Enter the receiver's name so the account can be verified.");
        else if(verifyName && !BankService::namesMatch(nameEdit->text(), QString::fromStdString(receiver->getCustomerName())))
            error->setText("The account number and name do not match. Check the details and try again.");
        else if(receiver->getStatus() != "Active")
            error->setText("Receiver account is not active.");
        else if(!BankService::parseAmount(amountEdit->text(), amt))
            error->setText("Enter a positive amount with up to two decimals.");
        else if(amt > bank.find(f)->getBalancePaisa())
            error->setText("Insufficient balance. Available: " + money(bank.find(f)->getBalancePaisa()));
        else if(QMessageBox::question(&d, "Confirm transfer",
                    QString("Send <b>%1</b> to <b>%2</b> (account %3)?<br>This cannot be undone.").arg(money(amt))
                        .arg(QString::fromStdString(receiver->getCustomerName())).arg(t)) == QMessageBox::Yes)
        {
            from = f;
            to = t;
            receiverName = verifyName ? nameEdit->text().trimmed() : QString();
            paisa = amt;
            d.accept();
        }
    });
    return d.exec() == QDialog::Accepted;
}

bool askCredentials(QWidget* parent, const QString& title, const QString& info, QString& username,
                    QString& password, bool lockUsername)
{
    QDialog d(parent);
    d.setWindowTitle(title);
    d.setMinimumWidth(380);
    auto* infoLabel = new QLabel(info);
    infoLabel->setWordWrap(true);
    auto* userEdit = new QLineEdit(username);
    userEdit->setReadOnly(lockUsername);
    userEdit->setMaxLength(20);
    userEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[A-Za-z0-9_.]*"), &d));
    auto* passEdit = new QLineEdit;
    passEdit->setMaxLength(64);
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setPlaceholderText("At least 6 characters");
    auto* error = errorLabel();
    auto* form = new QFormLayout;
    form->addRow("Username", userEdit);
    form->addRow("Password", passEdit);
    auto* box = okCancel(d, "Save login");
    auto* layout = new QVBoxLayout(&d);
    layout->addWidget(infoLabel);
    layout->addLayout(form);
    layout->addWidget(error);
    layout->addWidget(box);
    QObject::connect(box, &QDialogButtonBox::accepted, &d, [&]() {
        static const QRegularExpression userRe("^[A-Za-z0-9_.]{3,20}$");
        if(!userRe.match(userEdit->text()).hasMatch())
            error->setText("Username must be 3-20 letters, digits, '_' or '.'.");
        else if(passEdit->text().size() < 6 || passEdit->text().size() > 64 || passEdit->text().contains(' ') ||
                passEdit->text().contains('|'))
            error->setText("Password must be 6-64 characters with no spaces or '|'.");
        else
            d.accept();
    });
    if(d.exec() != QDialog::Accepted)
        return false;
    username = userEdit->text();
    password = passEdit->text();
    return true;
}

bool askChangePassword(QWidget* parent, QString& oldPassword, QString& newPassword)
{
    QDialog d(parent);
    d.setWindowTitle("Change password");
    d.setMinimumWidth(380);
    auto make = [] { auto* e = new QLineEdit; e->setEchoMode(QLineEdit::Password); e->setMaxLength(64); return e; };
    auto* oldEdit = make();
    auto* newEdit = make();
    auto* confirmEdit = make();
    auto* error = errorLabel();
    auto* form = new QFormLayout;
    form->addRow("Current password", oldEdit);
    form->addRow("New password", newEdit);
    form->addRow("Confirm new password", confirmEdit);
    auto* box = okCancel(d, "Change password");
    auto* layout = new QVBoxLayout(&d);
    layout->addLayout(form);
    layout->addWidget(error);
    layout->addWidget(box);
    QObject::connect(box, &QDialogButtonBox::accepted, &d, [&]() {
        if(oldEdit->text().isEmpty())
            error->setText("Enter your current password.");
        else if(newEdit->text().size() < 6 || newEdit->text().size() > 64 || newEdit->text().contains(' ') ||
                newEdit->text().contains('|'))
            error->setText("New password must be 6-64 characters with no spaces or '|'.");
        else if(newEdit->text() == oldEdit->text())
            error->setText("The new password must be different from the current one.");
        else if(newEdit->text() != confirmEdit->text())
            error->setText("New passwords do not match.");
        else
            d.accept();
    });
    if(d.exec() != QDialog::Accepted)
        return false;
    oldPassword = oldEdit->text();
    newPassword = newEdit->text();
    return true;
}

bool askRegistration(QWidget* parent, QString& name, QString& phone, QString& type,
                     QString& username, QString& password)
{
    QDialog d(parent);
    d.setWindowTitle("Create a bank account");
    d.setMinimumWidth(420);
    auto* intro = new QLabel("Fill in your details. Your account will be reviewed and must be "
                             "activated by a bank administrator before you can sign in.");
    intro->setWordWrap(true);
    intro->setObjectName("subtle");
    auto* nameEdit = new QLineEdit;
    nameEdit->setMaxLength(50);
    auto* phoneEdit = new QLineEdit;
    phoneEdit->setMaxLength(15);
    phoneEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]*"), &d));
    auto* typeBox = new QComboBox;
    typeBox->addItems({"Savings", "Current", "Student", "Business"});
    auto* userEdit = new QLineEdit;
    userEdit->setPlaceholderText("3-20 letters, digits, _ or .");
    userEdit->setMaxLength(20);
    userEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[A-Za-z0-9_.]*"), &d));
    auto* passEdit = new QLineEdit;
    passEdit->setMaxLength(64);
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setPlaceholderText("At least 6 characters");
    auto* confirmEdit = new QLineEdit;
    confirmEdit->setEchoMode(QLineEdit::Password);
    auto* error = errorLabel();

    auto* form = new QFormLayout;
    form->addRow("Full name", nameEdit);
    form->addRow("Phone number", phoneEdit);
    form->addRow("Account type", typeBox);
    form->addRow("Username", userEdit);
    form->addRow("Password", passEdit);
    form->addRow("Confirm password", confirmEdit);
    auto* box = okCancel(d, "Create account");
    auto* layout = new QVBoxLayout(&d);
    layout->addWidget(intro);
    layout->addLayout(form);
    layout->addWidget(error);
    layout->addWidget(box);

    QObject::connect(box, &QDialogButtonBox::accepted, &d, [&]() {
        static const QRegularExpression userRe("^[A-Za-z0-9_.]{3,20}$");
        QString n = nameEdit->text().trimmed();
        if(QString e = BankService::validateName(n); !e.isEmpty())
            error->setText(e);
        else if(QString e = BankService::validatePhone(phoneEdit->text()); !e.isEmpty())
            error->setText(e);
        else if(!userRe.match(userEdit->text()).hasMatch())
            error->setText("Username must be 3-20 letters, digits, '_' or '.'.");
        else if(passEdit->text().size() < 6 || passEdit->text().size() > 64 || passEdit->text().contains(' ') ||
                passEdit->text().contains('|'))
            error->setText("Password must be 6-64 characters with no spaces or '|'.");
        else if(passEdit->text() != confirmEdit->text())
            error->setText("Passwords do not match.");
        else
            d.accept();
    });
    if(d.exec() != QDialog::Accepted)
        return false;
    name = nameEdit->text().trimmed();
    phone = phoneEdit->text();
    type = typeBox->currentText();
    username = userEdit->text();
    password = passEdit->text();
    return true;
}
