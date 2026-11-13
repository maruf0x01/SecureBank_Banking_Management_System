# SecureBank Data Format

## Accounts

File: `data/accounts.txt`

Each line contains:

```text
account_number|customer_name|phone_number|account_type|balance|status
100001|Example Customer|01700000000|Savings|1250.50|Active
```

The balance is stored as a readable decimal value in the file and converted to integer paisa in memory. Valid statuses are `Active`, `Closed` and `Pending` (self-registered, awaiting admin activation). The pipe character is reserved as the field delimiter.

Customer names may contain up to 50 characters (letters, spaces, `.`, `-`, `'` in the GUI). Phone numbers must contain 7 to 15 digits.

## Transactions

File: `data/transactions.txt`

```text
transaction_id|account_number|type|amount|date_time|related_account_number
TXN001|100001|Deposit|100.00|Fri Aug 29 12:00:00 2026|0
```

Transfer operations create two records: `Transfer Sent` for the sender and `Transfer Received` for the receiver. A related account number is recorded for transfers and `0` for deposits and withdrawals.

Malformed records are ignored during loading instead of terminating the application.

## Customer Logins

File: `data/customers.txt`

```text
account_number|username|salt|sha256_hash
100001|alice|3f9a...|b1c2...
```

The hash is `SHA-256(salt + password)` in hex. Usernames are unique and
case-insensitive. Deleting an account removes its login.

## Deposit Requests

File: `data/deposit_requests.txt`

```text
id|account_number|amount_paisa|requested_at|status|decided_at
1|100001|25000|Fri Nov 13 02:28:28 2026|Pending|
```

`amount_paisa` is an integer number of paisa. Valid statuses are `Pending`,
`Approved` and `Rejected`. `decided_at` is empty while pending.

All files are git-ignored and created on first use.
