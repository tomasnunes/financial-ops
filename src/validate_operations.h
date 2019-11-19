#ifndef PROCESS_OPERATIONS_VALIDATE_OPERATIONS_H
#define PROCESS_OPERATIONS_VALIDATE_OPERATIONS_H

namespace mybank {

void validate_active_account(
        const account &,
        std::vector<Violation> &);

void validate_sufficient_limit(
        const account &,
        const transaction &,
        std::vector<Violation> &);

void validate_transactions_small_interval(
        const std::map<time_t, mybank::transaction> &,
        const transaction &,
        std::vector<Violation> &);

} //namespace mybank

#endif //PROCESS_OPERATIONS_VALIDATE_OPERATIONS_H
