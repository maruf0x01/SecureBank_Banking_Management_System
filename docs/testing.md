# SecureBank Testing

Run the automated tests with `./test.sh` on Linux or `./test.ps1` in Windows PowerShell.

## Automated suite (`tests/test_main.cpp`)

Covers `Account` and `FileManager`:

- Positive and invalid deposits
- Positive, negative, zero, and excessive withdrawals
- Transfer balance behavior
- Closed-account state preservation
- Account file save and load round trips
- Malformed, invalid, and duplicate account records
- Transaction file save and load round trips
- Invalid transaction records
- Customer-name length and phone-number validation

## Manual checklist for the GUI

Start from empty `data/` files (`./run_gui.sh`).

1. **Registration** - register a customer; sign in as that customer -> "awaiting activation".
   Invalid name/phone/username/password are rejected; duplicate usernames are rejected.
2. **Activation** - sign in as admin; the pending account is highlighted; *Activate account*; the customer can now sign in.
3. **Deposit approval** - customer requests a deposit; balance is unchanged; admin approves -> balance increases and a Deposit transaction appears. Approving twice is refused. Rejecting leaves the balance unchanged.
4. **Withdrawal** - wrong password is refused (3 wrong -> signed out); correct password withdraws and shows a confirmation pop-up; amount above balance is refused.
5. **Transfer** - wrong receiver name is refused; correct name asks for confirmation; both accounts show the transaction; self-transfer and closed accounts are refused.
6. **Admin tools** - edit, close/reopen, customer login set/reset/remove, history filters.
7. **Delete account** - refused while the balance is non-zero; wrong typed number refused; after deletion the login no longer works and deposit requests are gone.
8. **Sign out / reload** - the sidebar buttons are readable; signing out returns to the sign-in window.

The console version should also be compiled with `./run.sh` or `./run.ps1` before demonstration.
