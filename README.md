# SecureBank Banking Management System

A C++ banking management system for managing customer accounts and financial transactions.

## Current Scope

- Admin-only access
- Account creation and management
- Deposits and withdrawals
- Fund transfers
- Transaction history
- Text-file data storage

Customer login will be considered as a future enhancement.

## Project Structure

```text
SecureBank_Banking_Management_System/
|-- src/
|   |-- main.cpp
|   |-- Account.cpp
|   |-- Account.h
|   |-- BankSystem.cpp
|   |-- BankSystem.h
|   |-- FileManager.cpp
|   `-- FileManager.h
|-- data/
|   |-- accounts.txt
|   `-- transactions.txt
|-- tests/
|-- docs/
`-- README.md
```

## Build and Run

### Linux

Make the script executable once:

```bash
chmod +x run.sh
```

Build and run the application:

```bash
./run.sh
```

### Windows PowerShell

Build and run the application:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\run.ps1
```

Each script compiles all C++ source files in `src/` with C++17 and strict
warning flags, then runs the executable from the generated `build/` directory.

## Current Features

- Admin-only access with three login attempts
- Create, view, search, update, and close bank accounts
- Persistent account records stored in `data/accounts.txt`
- Safe temporary-file replacement during account saves
- Input validation for account numbers and text-file delimiters
- Deposits and withdrawals for active accounts
- Insufficient-balance protection for withdrawals

Fund transfers, transaction history, automated tests, and additional
documentation will be added in later milestones.
