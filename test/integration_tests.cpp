#include <sstream>

#include "catch.hpp"

#include "../include/process_operations/process_operations.h"

TEST_CASE("Test process_operations with vast input covering all violations", "[process_operations]")
{
    constexpr auto inputAuthorizerCompleteTest{
        R"({"account":{"activeAccount":true,"availableLimit":1000}}
           {"account":{"activeAccount":true,"availableLimit":100}}
           {"transaction":{"merchant":"Burger King","amount":1001,"time":"2019-02-13T10:00:00.000Z"}}
           {"transaction":{"merchant":"Burger King","amount":50,  "time":"2019-02-13T10:00:00.000Z"}}
           {"transaction":{"merchant":"Burger King","amount":50,  "time":"2019-02-13T10:01:00.000Z"}}
           {"transaction":{"merchant":"Burger King","amount":50,  "time":"2019-02-13T10:01:57.000Z"}}
           {"transaction":{"merchant":"Burger King","amount":100, "time":"2019-02-13T10:01:58.911Z"}}
           {"transaction":{"merchant":"Burger King","amount":100, "time":"2019-02-13T10:01:59.000Z"}}
           {"transaction":{"merchant":"Burger King","amount":100, "time":"2019-02-13T10:02:30.000Z"}}
           {"transaction":{"merchant":"Burger King","amount":650, "time":"2019-02-13T10:03:30.000Z"}}
           {"transaction":{"merchant":"Burger King","amount":100, "time":"2019-02-13T10:03:58.910Z"}}
           {"transaction":{"merchant":"Burger King","amount":50,  "time":"2019-02-13T10:03:58.911Z"}})"
    };
    constexpr auto outputAuthorizerCompleteTest{
        "{\"account\":{\"activeAccount\":true,\"availableLimit\":1000},\"violations\":[]}\n"
        "{\"account\":{\"activeAccount\":true,\"availableLimit\":1000},\"violations\":[\"account-already-initialized\"]}\n"
        "{\"account\":{\"activeAccount\":true,\"availableLimit\":1000},\"violations\":[\"insufficient-limit\"]}\n"
        "{\"account\":{\"activeAccount\":true,\"availableLimit\":950},\"violations\":[]}\n"
        "{\"account\":{\"activeAccount\":true,\"availableLimit\":900},\"violations\":[]}\n"
        "{\"account\":{\"activeAccount\":true,\"availableLimit\":900},\"violations\":[\"doubled-transaction\"]}\n"
        "{\"account\":{\"activeAccount\":true,\"availableLimit\":800},\"violations\":[]}\n"
        "{\"account\":{\"activeAccount\":true,\"availableLimit\":800},\"violations\":[\"high-frequency-small-interval\"]}\n"
        "{\"account\":{\"activeAccount\":true,\"availableLimit\":700},\"violations\":[]}\n"
        "{\"account\":{\"activeAccount\":true,\"availableLimit\":50},\"violations\":[]}\n"
        "{\"account\":{\"activeAccount\":true,\"availableLimit\":50},\"violations\":[\"insufficient-limit\",\"doubled-transaction\",\"high-frequency-small-interval\"]}\n"
        "{\"account\":{\"activeAccount\":true,\"availableLimit\":0},\"violations\":[]}\n"
    };

    std::istringstream input{ inputAuthorizerCompleteTest };
    std::ostringstream output;

    mybank::process_operations(input, output);

    REQUIRE( output.str() == outputAuthorizerCompleteTest );
}
