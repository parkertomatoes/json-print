#include "doctest/doctest.h"
#include "../src/json_print.hpp"

TEST_CASE("should not allow an unquoted string") {
    char buffer[128] = { 0 };
    const char format[] = "hello";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow unclosed string quotes") {
    char buffer[128] = { 0 };
    const char format[] = R"("hello)";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow control characters in strings") {
    char buffer[128] = { 0 };
    const char format[] = "hel\nlo";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow leading zeros in integers") {
    char buffer[128] = { 0 };
    const char format[] = "01";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow '-' thinking it's a number") {
    char buffer[128] = { 0 };
    const char format[] = "-";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow an exponent without a leading number") {
    char buffer[128] = { 0 };
    const char format[] = "e24";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow a number ending in a decimal point") {
    char buffer[128] = { 0 };
    const char format[] = "12.";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow a decimal point without a leading number") {
    char buffer[128] = { 0 };
    const char format[] = ".24";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow positive signs in integers") {
    char buffer[128] = { 0 };
    const char format[] = "+1";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow leading zeros in floats") {
    char buffer[128] = { 0 };
    const char format[] = "01.0";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow positive signs in floats") {
    char buffer[128] = { 0 };
    const char format[] = "+1.0";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow unclosed array brackets") {
    char buffer[128] = { 0 };
    const char format[] = "[1,2";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow unclosed object braces") {
    char buffer[128] = { 0 };
    const char format[] = R"({"a": 42)";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow array with missing comma") {
    char buffer[128] = { 0 };
    const char format[] = "[42 43]";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow object with missing colon") {
    char buffer[128] = { 0 };
    const char format[] = R"({"a" 42})";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should treat placeholder as separate object") {
    char buffer[128] = { 0 };
    const char format[] = "[42?]";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}

TEST_CASE("should not allow balanced but invalid nesting") {
    char buffer[128] = { 0 };
    const char format[] = R"([{"a": 42]})";
    CHECK_THROWS(JsonPrint::compile(format, format + sizeof(format)));
}