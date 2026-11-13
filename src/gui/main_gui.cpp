#include <QApplication>
#include <QDir>
#include <QEventLoop>

#include "AdminWindow.h"
#include "BankService.h"
#include "CustomerWindow.h"
#include "LoginDialog.h"
#include "Theme.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SecureBank");
    app.setQuitOnLastWindowClosed(false);
    applyTheme(app);

    // Locate the project's data/ directory (cwd, or relative to the executable).
    QDir dir(QDir::currentPath());
    if(!dir.exists("data"))
    {
        QDir appDir(QApplication::applicationDirPath());
        if(appDir.cd("..") && appDir.exists("data"))
            dir = appDir;
    }
    dir.mkpath("data");
    QDir::setCurrent(dir.absolutePath());

    BankService bank;

    while(true)
    {
        LoginDialog login(bank);
        if(login.exec() != QDialog::Accepted)
            break;

        bank.reload();
        QMainWindow* window = nullptr;
        bool signedOut = false;
        if(login.role() == LoginDialog::Admin)
        {
            auto* w = new AdminWindow(bank);
            QObject::connect(w, &AdminWindow::loggedOut, [&] { signedOut = true; });
            window = w;
        }
        else
        {
            auto* w = new CustomerWindow(bank, login.accountNumber());
            QObject::connect(w, &CustomerWindow::loggedOut, [&] { signedOut = true; });
            window = w;
        }

        window->setAttribute(Qt::WA_DeleteOnClose);
        QEventLoop loop;
        QObject::connect(window, &QObject::destroyed, &loop, &QEventLoop::quit);
        window->show();
        loop.exec();

        if(!signedOut) // window closed with the X button: exit the application
            break;
    }
    return 0;
}
