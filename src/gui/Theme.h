// ============================================================================
// FILE: Theme.h
// PURPOSE: Application-wide styling (colors, fonts, border radius) and small
//          reusable UI card/title builder functions.
//          Works just like CSS in web development.
// ============================================================================
#pragma once

#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

// Applies the global stylesheet and default font size to the entire application.
inline void applyTheme(QApplication& app)
{
    app.setStyle("Fusion");

    // [FONT SIZE] Global base font size (increase 10 to 12/14 to make all text bigger)
    QFont f = app.font();
    f.setPointSize(std::max(f.pointSize(), 10));
    app.setFont(f);

    // [STYLESHEET / CSS] Change colors, font sizes, or border-radius here:
    app.setStyleSheet(R"(
        /* Default text color across the app */
        QWidget { color: #1f2937; }

        /* Main window and dialog page background color */
        QMainWindow, QDialog, QStackedWidget > QWidget { background: #f3f4f6; }

        /* Page heading ("Dashboard", "Accounts", "Welcome, ...") */
        QLabel#pageTitle { font-size: 22px; font-weight: 700; }

        /* Muted gray subtitle / helper text */
        QLabel#subtle { color: #6b7280; }

        /* Big number inside dashboard statistic cards */
        QLabel#cardValue { font-size: 24px; font-weight: 700; color: #111827; }

        /* Small title above dashboard statistic cards */
        QLabel#cardTitle { color: #6b7280; font-size: 12px; }

        /* Big balance text inside customer's blue Hero Card */
        QLabel#balanceValue { font-size: 38px; font-weight: 700; color: white; }

        /* Light text placed on top of the blue Hero Card */
        QLabel#onBlue { color: #dbeafe; }

        /* White info card container (used on Dashboard and Profile page) */
        QFrame#card { background: white; border: 1px solid #e5e7eb; border-radius: 10px; }

        /* Customer Overview blue gradient balance card */
        QFrame#heroCard { background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #1d4ed8, stop:1 #4f46e5);
                          border-radius: 14px; }
        QFrame#heroCard QLabel { background: transparent; }

        /* Left dark navigation sidebar panel and bottom button area */
        QWidget#sidePanel { background: #111827; }
        QWidget#sideButtons { background: #111827; }

        /* Sidebar bottom buttons ("Reload data" and "Sign out") */
        QPushButton#sideButton { background: #1f2937; color: #f9fafb; border: 1px solid #4b5563; border-radius: 7px;
                                 padding: 8px 14px; font-weight: 600; }
        QPushButton#sideButton:hover { background: #374151; border-color: #9ca3af; }

        /* Red "Sign out" button in the sidebar */
        QPushButton#sideButton[logout="true"] { background: #b91c1c; border-color: #b91c1c; }
        QPushButton#sideButton[logout="true"]:hover { background: #dc2626; }

        /* Sidebar menu list items (Dashboard, Accounts, Transactions, etc.) */
        QListWidget#sidebar { background: #111827; border: none; padding-top: 8px; outline: 0; }
        QListWidget#sidebar::item { color: #d1d5db; padding: 12px 18px; margin: 2px 8px; border-radius: 8px; }
        QListWidget#sidebar::item:hover { background: #1f2937; }
        QListWidget#sidebar::item:selected { background: #2563eb; color: white; }

        /* Sidebar top brand logo text ("SecureBank") and role subtitle ("Administrator" / "Online banking") */
        QLabel#brand { color: white; font-size: 18px; font-weight: 700; padding: 16px 18px 4px 18px; background: #111827; }
        QLabel#brandSub { color: #9ca3af; padding: 0 18px 8px 18px; background: #111827; }

        /* Standard white button */
        QPushButton { background: white; border: 1px solid #d1d5db; border-radius: 7px; padding: 7px 14px; }
        QPushButton:hover { background: #f9fafb; border-color: #9ca3af; }
        QPushButton:disabled { color: #9ca3af; background: #f3f4f6; }

        /* Primary action button (Blue: "Sign in", "New account", "Send money", "Approve") */
        QPushButton[primary="true"] { background: #2563eb; border-color: #2563eb; color: white; font-weight: 600; }
        QPushButton[primary="true"]:hover { background: #1d4ed8; }
        QPushButton[primary="true"]:disabled { background: #93c5fd; border-color: #93c5fd; color: white; }

        /* Danger button (Red border/text: "Delete account", "Reject") */
        QPushButton[danger="true"] { color: #b91c1c; border-color: #fca5a5; }
        QPushButton[danger="true"]:hover { background: #fef2f2; }

        /* Text input boxes and dropdown menus */
        QLineEdit, QComboBox { background: white; border: 1px solid #d1d5db; border-radius: 7px; padding: 6px 9px; }
        QLineEdit:focus, QComboBox:focus { border: 1px solid #2563eb; }

        /* Data tables (Accounts, Transactions, Deposit requests) */
        QTableWidget { background: white; alternate-background-color: #f9fafb; border: 1px solid #e5e7eb;
                       border-radius: 8px; gridline-color: transparent; selection-background-color: #dbeafe;
                       selection-color: #111827; }
        QHeaderView::section { background: #f3f4f6; border: none; border-bottom: 1px solid #e5e7eb;
                               padding: 8px; font-weight: 600; color: #374151; }

        /* Bottom status bar and hover tooltips */
        QTabBar::tab { padding: 8px 18px; }
        QStatusBar { background: white; border-top: 1px solid #e5e7eb; }
        QToolTip { color: #111827; background: #fffbe6; border: 1px solid #d1d5db; }
    )");
}

// Helper: Wraps the bottom sidebar buttons ("Reload data", "Sign out") so they stay readable on the dark sidebar.
inline QWidget* makeSideButtons(const QList<QPushButton*>& buttons)
{
    auto* box = new QWidget;
    box->setObjectName("sideButtons");
    box->setAttribute(Qt::WA_StyledBackground, true);
    auto* l = new QVBoxLayout(box);
    l->setContentsMargins(14, 8, 14, 0);
    l->setSpacing(8);
    for(QPushButton* b : buttons)
    {
        b->setObjectName("sideButton");
        b->setCursor(Qt::PointingHandCursor);
        l->addWidget(b);
    }
    return box;
}

// Helper: Creates an empty white rounded card container.
inline QFrame* makeCard()
{
    auto* card = new QFrame;
    card->setObjectName("card");
    return card;
}

// Helper: Creates a dashboard statistic card with a small gray title and a large bold value label.
inline QFrame* makeStatCard(const QString& title, QLabel*& valueLabel)
{
    QFrame* card = makeCard();
    auto* layout = new QVBoxLayout(card);
    auto* t = new QLabel(title);       // Card heading text (e.g., "Total accounts")
    t->setObjectName("cardTitle");
    valueLabel = new QLabel("-");      // Card value text (updated dynamically later)
    valueLabel->setObjectName("cardValue");
    layout->addWidget(t);
    layout->addWidget(valueLabel);
    return card;
}

// Helper: Adds a big page title and a gray subtitle at the top of a page layout.
inline QLabel* makeTitle(const QString& text, const QString& subtitle, QVBoxLayout* into)
{
    auto* t = new QLabel(text);        // Main title text
    t->setObjectName("pageTitle");
    auto* s = new QLabel(subtitle);    // Subtitle description text
    s->setObjectName("subtle");
    into->addWidget(t);
    into->addWidget(s);
    return t;
}
