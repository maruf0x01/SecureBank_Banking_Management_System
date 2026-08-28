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
|   `-- main.cpp
`-- README.md
```

## Build and Run

Compile the application with:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o securebank
```

Run it with:

```bash
./securebank
```

The current version provides the main menu. Features will be implemented incrementally.
