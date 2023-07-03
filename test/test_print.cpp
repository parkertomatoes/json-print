#include "doctest/doctest.h"
#include "../src/json_print.hpp"

TEST_CASE("json_print_c compiles without errors") {
    const char format[] = R"({"hello": ?})";
    json_print_c("[?, 42]", "hello");
}
