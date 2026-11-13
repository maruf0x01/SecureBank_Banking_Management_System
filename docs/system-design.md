# SecureBank System Design

## Architecture

SecureBank has two front-ends that share one storage layer.

- `Account` - account identity, status and integer-paisa balance rules.
- `FileManager` - loads and saves validated account and transaction records
  (atomic writes through temporary files).
- `BankSystem` + `main.cpp` - console front-end (admin only).
- `src/gui/` - Qt6 front-end:
  - `BankService` - all GUI business rules: account lifecycle, deposits,
    withdrawals, transfers, deposit requests, customer logins, validation.
  - `LoginDialog` - Customer/Admin sign-in and self-registration.
  - `AdminWindow`, `CustomerWindow` - role-specific main windows.
  - `Dialogs`, `Theme.h` - reusable validated dialogs, tables and styling.

`main_gui.cpp` loops: sign-in -> role window -> sign out -> sign-in, and exits
when the sign-in is cancelled or a window is closed with the window button.

## Roles and Access

| Role | Authentication | Capabilities |
|------|----------------|--------------|
| Admin | fixed `admin` / `admin123` | full account management, approvals, logins |
| Customer | username + salted SHA-256 password hash | own account only |

A customer can only act on the account bound to their login.

## Account Lifecycle

`Pending` (self-registered) -> `Active` (admin activates) <-> `Closed`.
Admin-created accounts start `Active`. An admin may delete an account whose
balance is zero; its login and deposit requests are deleted, transaction history
is kept.

## Money Workflows

- **Deposit (admin)**: credited immediately and logged.
- **Deposit (customer)**: creates a `Pending` request; on approval the request is
  marked `Approved` and the account is credited and logged. If crediting fails
  the request returns to `Pending`. A request can be decided only once.
- **Withdrawal (customer)**: amount + password; 3 wrong passwords sign out.
- **Transfer**: both accounts active, different, sufficient balance. Customers
  must also enter the receiver's name, which must match the account holder.
  Two transaction records are written (`Transfer Sent`, `Transfer Received`).

Financial changes save the account snapshot and new transactions together; any
failed save restores the previous in-memory state.

## Design Decisions

- Money is stored as integer paisa to avoid floating-point rounding errors;
  files stay human-readable with two decimals.
- Validation happens in the dialogs for immediate feedback and again in
  `BankService`, so the rules cannot be bypassed by a UI path.
- Closed accounts are retained so their history is not lost.
- Runtime data is excluded from version control because it contains personal
  data and password hashes.
