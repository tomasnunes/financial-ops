#ifndef MYBANK_PROCESS_OPERATIONS_H
#define MYBANK_PROCESS_OPERATIONS_H

#include <cstdint>
#include <ctime>
#include <iosfwd>
#include <iostream>
#include <functional>
#include <string>

namespace mybank
{

enum class Violation
{
    ACCOUNT_ALREADY_INITIALIZED,
    ACCOUNT_NOT_ACTIVE,
    DOUBLED_TRANSACTION,
    HIGH_FREQUENCY_SMALL_INTERVAL,
    INSUFFICIENT_LIMIT
};

struct account
{
    bool activeAccount;
    int64_t availableLimit;
};

struct transaction
{
    int64_t amount;
    std::string merchant;
    std::string timeIso8601;
    time_t timeInMillis;
};

void process_operations(
        std::istream & = std::cin,
        std::ostream & = std::cout);

auto get_new_account(
        std::istream & = std::cin,
        std::ostream & = std::cout)
        -> std::optional<mybank::account>;

void process_transactions(
        mybank::account &,
        std::istream & = std::cin,
        std::ostream & = std::cout);

} //namespace mybank

#endif //MYBANK_PROCESS_OPERATIONS_H
