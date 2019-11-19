# Process Financial Operations

This program accepts JSON lines with two possible operations described bellow and
outputs JSON lines with the account state and violations committed after each operation.

Any invalid input is ignored and reading terminates on `EOF`.

### Recognized Operations
#### Account
Type: `JSON`
Format: `{ "account": { "activeAccount": bool, "availableLimit": int } }`

Example:
```json
{ "account": { "activeAccount": true, "availableLimit": 1000 } }
```

#### Transaction
Type: `JSON`
Format: `{ "transaction": { "merchant": string, "amount": int, "time": ISO8601 format (yyyy-mm-ddThh:mm:ss.sssZ) } }`

Example:
```json
{ "transaction": { "merchant": "Burger King", "amount": 50, "time": "2019-02-13T10:00:00.000Z" } }
```

### Possible Violations

- `account-already-initialized`:
    returned when an account operation is provided after an account is already created;
- `insufficient-limit`:
    returned when the amount of the current transaction is higher than the available limit of the account;
- `account-not-active`:
    returned when the account is not active;
- `high-frequency-small-interval`:
    return when more than 3 transactions occur in a small interval;
- `doubled-transaction`:
    return when more than 2 equal transactions (same amount and merchant) occur in a small interval 
    
Only valid transactions are accounted for when evaluating violations.

## Design Choices
    
#### Handle previous transactions
If the transactions were received from oldest to newest, we would only need to store the last
3 transactions since there would never be more than 3 valid transactions in the last 2 minutes according
to the `high-frequency-small-interval` violation and we only require information from the last 2 minutes
to evaluate the validity of a transaction.

Since we can receive transactions in any possible order all previous valid transactions need to be stored,
but assuming that most transactions will be ordered, a more efficient storing solution is an ordered map
with the transaction's time as keys. For each new transaction, we can iterate through the previous transactions
in reverse order until the time difference is higher than 2 minutes, this way in the majority of cases we
only iterate through 3 transactions at most.

## Usage

First install the JSON parser `nlohmann/json`:

```shell script
$ brew tap nlohmann/json
$ brew install nlohmann-json
```

Add this project to the lib directory in your new project
and include the `process_operations.h` header file present in the include folder.

Call the `process_operations` function with your preferred
input and output streams or with the default, `stdin` and `stdout`.

```
mybank::process_operations(); // Uses std::cin and std::cout by default
```

### Running Unit and Integration Tests

```shell script
$ mkdir build
$ cd build
$ cmake ..
$ make
$ test/authorizer_tests
```


## Links

To both parse the input json and create the output json I used `nlohmann`'s library,
it provides an intuitive syntax in accordance to modern C++.
- github: https://github.com/nlohmann/json

For testing I used Catch2, it's a really simple to use, single header file, framework.
- github: https://github.com/catchorg/Catch2
