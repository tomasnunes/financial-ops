#include <cmath>
#include <deque>
#include <map>
#include <vector>

#include "process_operations/process_operations.h"
#include "validate_operations.h"
#include "json_utils.h"

void mybank::process_operations(std::istream &in, std::ostream &out)
{
    auto account{ get_new_account(in, out) };

    if (account.has_value())
    {
        process_transactions(account.value(), in, out);
    }
}

auto mybank::get_new_account(std::istream &in, std::ostream &out) -> std::optional<mybank::account>
{
    for (std::string inputLine; std::getline(in, inputLine);)
    {
        if (!json::accept(inputLine))
        {
            continue;
        }

        const auto inputJson = json::parse(inputLine);

        if (is_valid_json_account(inputJson))
        {
            const auto newAccount{ inputJson["account"].get<mybank::account>() };
            const auto outputJson = mybank::build_output_json(newAccount, {});
            out << outputJson << '\n';
            return std::optional<mybank::account>{newAccount };
        }
    }

    return std::nullopt;
}

void mybank::process_transactions(mybank::account &account, std::istream &in, std::ostream &out)
{
    std::vector<mybank::Violation> violations{};
    std::map<time_t, mybank::transaction> validTransactions{};

    for (std::string inputLine; std::getline(in, inputLine);)
    {
        if (!json::accept(inputLine))
        {
            continue;
        }

        violations.clear();
        const auto inputJson = json::parse(inputLine);

        if (is_valid_json_account(inputJson))
        {
            violations.push_back(mybank::Violation::ACCOUNT_ALREADY_INITIALIZED);
        }
        else if (is_valid_json_transaction(inputJson))
        {
            const auto transaction{ inputJson["transaction"].get<mybank::transaction>() };

            validate_active_account(account, violations);
            validate_sufficient_limit(account, transaction, violations);
            validate_transactions_small_interval(validTransactions, transaction, violations);

            if (violations.empty())
            {
                account.availableLimit -= transaction.amount;
                validTransactions.emplace(transaction.timeInMillis, transaction);
            }
        }

        const auto outputJson = mybank::build_output_json(account, violations);
        out << outputJson << '\n';
    }
}

void mybank::validate_active_account(
        const account &account,
        std::vector<Violation> &violations)
{
    if (!account.activeAccount)
    {
        violations.push_back(mybank::Violation::ACCOUNT_NOT_ACTIVE);
    }
}

void mybank::validate_sufficient_limit(
        const account &account,
        const transaction &transaction,
        std::vector<Violation> &violations)
{
    if (account.availableLimit < transaction.amount)
    {
        violations.push_back(mybank::Violation::INSUFFICIENT_LIMIT);
    }
}

void mybank::validate_transactions_small_interval(
        const std::map<time_t, mybank::transaction> &validTransactions,
        const transaction &transaction,
        std::vector<Violation> &violations)
{
    constexpr auto smallInterval{ 2*60*1000 }; // 2 minutes in milliseconds

    double timediff;
    auto maxTransactionsSmallInterval{ 0 };
    auto maxEqualTransactionsSmallInterval{ 0 };
    std::deque<mybank::transaction> transactionsSmallInterval{};
    for (auto crit{ validTransactions.crbegin() };
         crit != validTransactions.crend() &&
         (timediff = difftime(transaction.timeInMillis, crit->first)) < smallInterval;
         ++crit)
    {
        if (fabs(timediff) < smallInterval)
        {
            transactionsSmallInterval.push_back(crit->second);
            while (difftime(transactionsSmallInterval.front().timeInMillis, crit->first) > smallInterval)
            {
                transactionsSmallInterval.pop_front();
            }

            auto currTransactionsSmallInterval{ 0 };
            auto currEqualTransactionsSmallInterval{ 0 };
            for (const auto &t : transactionsSmallInterval)
            {
                ++currTransactionsSmallInterval;

                if (t.merchant == transaction.merchant && t.amount == transaction.amount)
                {
                    ++currEqualTransactionsSmallInterval;
                }
            }

            if (currTransactionsSmallInterval > maxTransactionsSmallInterval)
            {
                maxTransactionsSmallInterval = currTransactionsSmallInterval;
            }

            if (currEqualTransactionsSmallInterval > maxEqualTransactionsSmallInterval)
            {
                maxEqualTransactionsSmallInterval = currEqualTransactionsSmallInterval;
            }
        }
    }

    if (maxEqualTransactionsSmallInterval > 1)
    {
        violations.push_back(mybank::Violation::DOUBLED_TRANSACTION);
    }

    if (maxTransactionsSmallInterval > 2)
    {
        violations.push_back(mybank::Violation::HIGH_FREQUENCY_SMALL_INTERVAL);
    }
}

void mybank::to_json(json &j, const account &a)
{
    j = json{
            { "activeAccount", a.activeAccount },
            { "availableLimit", a.availableLimit }
    };
}

void mybank::from_json(const json& j, account &a)
{
    j.at("activeAccount").get_to(a.activeAccount);
    j.at("availableLimit").get_to(a.availableLimit);
}

void mybank::to_json(json &j, const transaction &t)
{
    j = json{
        { "amount", t.amount },
        { "merchant", t.merchant },
        { "time", t.timeIso8601 }
    };
}

void mybank::from_json(const json& j, transaction &t)
{
    j.at("amount").get_to(t.amount);
    j.at("merchant").get_to(t.merchant);
    j.at("time").get_to(t.timeIso8601);

    tm time{};
    memset(&time, 0, sizeof(tm));
    strptime(t.timeIso8601.c_str(), "%Y-%m-%dT%H:%M:%SZ", &time);
    const auto milliseconds{ (t.timeIso8601.length() > 20) ? std::strtol(&t.timeIso8601[20], nullptr, 10) : 0 };
    t.timeInMillis = mktime(&time)*1000 + milliseconds;
}

auto mybank::build_output_json(
        const mybank::account &account,
        const std::vector<mybank::Violation> &violations)
        -> json
{
    return json{
        { "account", account },
        { "violations", violations }
    };
}

auto mybank::is_valid_json_account(const json &j) -> bool
{
    return (j.is_object() &&
            j.find("account") != j.end() &&
            j["account"].find("activeAccount") != j["account"].end() &&
            j["account"]["activeAccount"].is_boolean() &&
            j["account"].find("availableLimit") != j["account"].end() &&
            j["account"]["availableLimit"].is_number_integer());
}

auto mybank::is_valid_json_transaction(const json &j) -> bool
{
    return (j.is_object() &&
            j.find("transaction") != j.end() &&
            j["transaction"].find("merchant") != j["transaction"].end() &&
            j["transaction"].find("amount") != j["transaction"].end() &&
            j["transaction"]["amount"].is_number_integer() &&
            j["transaction"].find("time") != j["transaction"].end());
}
