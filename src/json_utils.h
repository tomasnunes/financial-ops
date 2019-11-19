#ifndef PROCESS_OPERATIONS_JSON_UTILS_H
#define PROCESS_OPERATIONS_JSON_UTILS_H

#include <deque>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mybank
{

NLOHMANN_JSON_SERIALIZE_ENUM(Violation, {
    { Violation::ACCOUNT_ALREADY_INITIALIZED, "account-already-initialized" },
    { Violation::ACCOUNT_NOT_ACTIVE, "account-not-active" },
    { Violation::DOUBLED_TRANSACTION, "doubled-transaction" },
    { Violation::HIGH_FREQUENCY_SMALL_INTERVAL, "high-frequency-small-interval" },
    { Violation::INSUFFICIENT_LIMIT, "insufficient-limit" }
})

void to_json(json &, const account &);
void from_json(const json &, account &);

void to_json(json &, const transaction &);
void from_json(const json &, transaction &);

auto build_output_json(
        const mybank::account &account,
        const std::vector<mybank::Violation> &violations)
        -> json;

auto is_valid_json_account(const json &) -> bool;
auto is_valid_json_transaction(const json &) -> bool;

} // namespace mybank

#endif // PROCESS_OPERATIONS_JSON_UTILS_H
