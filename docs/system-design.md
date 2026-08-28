# SecureBank System Design

## Architecture

SecureBank is a menu-driven C++17 console application with three main layers:

- `Account`: owns account identity, status, and integer-paisa balance rules.
- `BankSystem`: controls authentication-facing workflows, account operations, financial operations, reports, and user input.
- `FileManager`: loads and saves validated account and transaction records.

`main.cpp` displays the menu and delegates operations to `BankSystem`.

## Workflow

1. The administrator signs in.
2. `BankSystem` loads accounts from `data/accounts.txt`.
3. The administrator selects an operation.
4. Input is validated before state changes.
5. Account-only changes are saved through a temporary file.
6. Financial changes save account and transaction snapshots together.
7. The updated state is available on the next run.

## Design Decisions

Money is stored internally as integer paisa to avoid floating-point rounding errors. Text files remain human-readable with two decimal places. Closed accounts are retained so their history is not lost and can be reopened by an administrator.
