#define CATCH_CONFIG_MAIN

#include <sstream>

#include "catch.hpp"

#include "../include/process_operations/process_operations.h"

TEST_CASE( "Test get_new_account", "[get_new_account]" )
{
    SECTION( "with active account" )
    {
        constexpr auto inputAccountActive{ R"({"account":{"activeAccount":true,"availableLimit":100}})" };
        constexpr auto outputAccountActive{ "{\"account\":{\"activeAccount\":true,\"availableLimit\":100},\"violations\":[]}\n" };

        std::istringstream input{ inputAccountActive };
        std::ostringstream output;

        const auto a{ mybank::get_new_account(input, output) };

        REQUIRE( a.has_value() );
        REQUIRE( a.value().activeAccount );
        REQUIRE( a.value().availableLimit == 100 );
        REQUIRE( output.str() == outputAccountActive );
    }

    SECTION( "with inactive account" )
    {
        constexpr auto inputAccountNotActive{ R"({"account":{"activeAccount":false,"availableLimit":100}})" };
        constexpr auto outputAccountNotActive{ "{\"account\":{\"activeAccount\":false,\"availableLimit\":100},\"violations\":[]}\n" };

        std::istringstream input{ inputAccountNotActive };
        std::ostringstream output;

        const auto a{ mybank::get_new_account(input, output) };

        REQUIRE( a.has_value() );
        REQUIRE( !a.value().activeAccount );
        REQUIRE( output.str() == outputAccountNotActive );
    }

    SECTION( "without account" )
    {
        constexpr auto inputWithoutAccount{ R"({"transaction":{"merchant":"Burger King","amount":20,"time":"2019-02-13T10:00:00.000Z"}})" };

        std::istringstream input{ inputWithoutAccount };
        std::ostringstream output;

        const auto a{mybank::get_new_account(input, output) };

        REQUIRE( !a.has_value() );
        REQUIRE( output.str().empty() );
    }
}

TEST_CASE( "Test process_transactions", "[process_transactions]" )
{
    SECTION( "with account input, then 'account-already-initialized' violation is returned" )
    {
        constexpr auto inputAccount{ R"({"account":{"activeAccount":true,"availableLimit":100}})" };
        constexpr auto outputViolationAccountAlreadyInitialized{ "{\"account\":{\"activeAccount\":true,\"availableLimit\":100},\"violations\":[\"account-already-initialized\"]}\n" };

        mybank::account account{ true, 100 };

        std::istringstream input{ inputAccount };
        std::ostringstream output;

        mybank::process_transactions(account, input, output);

        REQUIRE( account.activeAccount );
        REQUIRE( account.availableLimit == 100 );
        REQUIRE( output.str() == outputViolationAccountAlreadyInitialized );
    }

    SECTION( "with account not active, then 'account-not-active' violation is returned" )
    {
        constexpr auto inputTransaction{ R"({"transaction":{"merchant":"Burger King","amount":20,"time":"2019-02-13T10:00:00.000Z"}})" };
        constexpr auto outputViolationCardNotActive{ "{\"account\":{\"activeAccount\":false,\"availableLimit\":100},\"violations\":[\"account-not-active\"]}\n" };

        mybank::account account_not_active{ false, 100 };

        std::istringstream input{ inputTransaction };
        std::ostringstream output;

        mybank::process_transactions(account_not_active, input, output);

        REQUIRE( !account_not_active.activeAccount );
        REQUIRE( account_not_active.availableLimit == 100 );
        REQUIRE( output.str() == outputViolationCardNotActive );
    }

    SECTION( "with over-limit amount, then 'insufficient-limit' is returned" )
    {
        constexpr auto inputTransactionOverLimit{ R"({"transaction":{"merchant":"Habbib's","amount":120,"time":"2019-02-13T10:00:00.000Z"}})" };
        constexpr auto outputViolationInsufficientLimit{ "{\"account\":{\"activeAccount\":true,\"availableLimit\":100},\"violations\":[\"insufficient-limit\"]}\n" };

        mybank::account account{ true, 100 };

        std::istringstream input{ inputTransactionOverLimit };
        std::ostringstream output;

        mybank::process_transactions(account, input, output);

        REQUIRE( account.activeAccount );
        REQUIRE( account.availableLimit == 100 );
        REQUIRE( output.str() == outputViolationInsufficientLimit );
    }

    SECTION( "with 4 distinct transactions in 2 minutes interval, then 'high-frequency-small-interval' is returned" )
    {
        constexpr auto inputHighFrequencySmallIntervalTransactions{
            R"({"transaction":{"merchant":"Burger King","amount":10,"time":"2019-02-13T10:00:00.000Z"}}
               {"transaction":{"merchant":"Burger King","amount":15,"time":"2019-02-13T10:00:30.000Z"}}
               {"transaction":{"merchant":"Burger King","amount":20,"time":"2019-02-13T10:01:00.000Z"}}
               {"transaction":{"merchant":"Burger King","amount":25,"time":"2019-02-13T10:01:59.000Z"}}
               {"transaction":{"merchant":"Burger King","amount":25,"time":"2019-02-13T10:02:00.000Z"}})"
        };
        constexpr auto outputViolationHighFrequencySmallInterval{
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":90},\"violations\":[]}\n"
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":75},\"violations\":[]}\n"
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":55},\"violations\":[]}\n"
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":55},\"violations\":[\"high-frequency-small-interval\"]}\n"
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":30},\"violations\":[]}\n"
        };

        mybank::account account{ true, 100 };

        std::istringstream input{ inputHighFrequencySmallIntervalTransactions };
        std::ostringstream output;

        mybank::process_transactions(account, input, output);

        REQUIRE( account.activeAccount );
        REQUIRE( account.availableLimit == 30 );
        REQUIRE( output.str() == outputViolationHighFrequencySmallInterval );
    }

    SECTION( "with 3 equal transactions in 2 minutes interval, then 'doubled-transaction' is returned" )
    {
        constexpr auto inputDoubledTransaction{
            R"({"transaction":{"merchant":"Burger King","amount":20,"time":"2019-02-13T10:00:00.000Z"}}
               {"transaction":{"merchant":"Burger King","amount":20,"time":"2019-02-13T10:01:00.000Z"}}
               {"transaction":{"merchant":"Burger King","amount":20,"time":"2019-02-13T10:01:59.000Z"}}
               {"transaction":{"merchant":"Burger King","amount":20,"time":"2019-02-13T10:02:00.000Z"}})"
        };
        constexpr auto outputViolationDoubledTransaction{
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":80},\"violations\":[]}\n"
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":60},\"violations\":[]}\n"
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":60},\"violations\":[\"doubled-transaction\"]}\n"
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":40},\"violations\":[]}\n"
        };

        mybank::account account{ true, 100 };

        std::istringstream input{ inputDoubledTransaction };
        std::ostringstream output;

        mybank::process_transactions(account, input, output);

        REQUIRE( account.activeAccount );
        REQUIRE( account.availableLimit == 40 );
        REQUIRE( output.str() == outputViolationDoubledTransaction );
    }

    SECTION( "with unordered transactions" )
    {
        constexpr auto inputUnorderedTransactions{
            R"({"transaction":{"merchant":"Burger King","amount":20,"time":"2019-02-13T10:00:00.000Z"}}
               {"transaction":{"merchant":"Burger King","amount":30,"time":"2019-02-13T10:01:00.000Z"}}
               {"transaction":{"merchant":"Burger King","amount":30,"time":"2019-02-13T10:01:55.000Z"}}
               {"transaction":{"merchant":"Burger King","amount":20,"time":"2019-02-13T10:01:30.000Z"}}
               {"transaction":{"merchant":"Burger King","amount":20,"time":"2019-02-13T09:59:55.001Z"}})"
        };
        constexpr auto outputUnorderedTransactions{
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":80},\"violations\":[]}\n"
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":50},\"violations\":[]}\n"
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":20},\"violations\":[]}\n"
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":20},\"violations\":[\"high-frequency-small-interval\"]}\n"
            "{\"account\":{\"activeAccount\":true,\"availableLimit\":20},\"violations\":[\"high-frequency-small-interval\"]}\n"
        };

        mybank::account account{ true, 100 };

        std::istringstream input{ inputUnorderedTransactions };
        std::ostringstream output;

        mybank::process_transactions(account, input, output);

        REQUIRE( account.activeAccount );
        REQUIRE( account.availableLimit == 20 );
        REQUIRE( output.str() == outputUnorderedTransactions );
    }
}
