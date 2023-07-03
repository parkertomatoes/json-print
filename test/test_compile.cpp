#include "doctest/doctest.h"
#include "../src/json_print.hpp"

TEST_CASE("should compile true") {
    const char format[] = "true";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile false") {
    const char format[] = "false";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile null") {
    const char format[] = "null";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile zero") {
    const char format[] = "0";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a positive integer") {
    const char format[] = "42";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a negative integer") {
    const char format[] = "-25";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a floating point number") {
    const char format[] = "24.8754";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a negative floating point number") {
    const char format[] = "-8.2";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a positive fractional number") {
    const char format[] = "0.22233";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a negative fractional number") {
    const char format[] = "-0.98765";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile zero with exponent") {
    const char format[] = "0E1";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a positive integer with exponent") {
    const char format[] = "42e8";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a negative integer with exponent") {
    const char format[] = "-25E-1";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a floating point number with exponent") {
    const char format[] = "24.8754e-2";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a negative floating point number with exponent") {
    const char format[] = "-8.2e51";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a positive fractional number with exponent") {
    const char format[] = "0.22233E1";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a negative fractional number with exponent") {
    const char format[] = "-0.98765e-1";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a negative fractional number with exponent") {
    const char format[] = "-0.98765e-1";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a string") {
    const char format[] = R"("hello")";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a string with UTF8 code points") {
    const char format[] = u8"\"\u3053\u3093\u306B\u3061\u306F\"";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile a with valid escape characters") {
    const char format[] = R"("\"\\\/\b\f\n\r\t")";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile an empty array") {
    const char format[] = "[]";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile an array with one item") {
    const char format[] = "[42]";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile an array with many items") {
    const char format[] = R"([ 42, true, "hello", null])";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile an array with complex items") {
    const char format[] = R"([ [], {"hello": "world"} ])";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile an empty object") {
    const char format[] = "{}";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile an object with one member") {
    const char format[] = R"({ "hello": "world" })";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile an object with many members") {
    const char format[] = R"({ "a": "b", "c": 2, "d": false, "e": null })";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should compile an object with complex members") {
    const char format[] = R"({ "a": [1, 2, 3], "b": { "a": 5 } })";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 1);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + sizeof(format));
}

TEST_CASE("should parse a single placeholder") {
    const char format[] = "?";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 2);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format);
    CHECK(context.parts[2] == format + sizeof(format));
}

TEST_CASE("should parse a single placeholder with whitespace") {
    const char format[] = "  ? ";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 2);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + 2);
    CHECK(context.parts[2] == format + sizeof(format));
}

TEST_CASE("should parse a placeholder inside an array") {
    const char format[] = "[42,?]";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 2);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + 4);
    CHECK(context.parts[2] == format + sizeof(format));
}

TEST_CASE("should parse a placeholder inside an object") {
    const char format[] = R"({"hello": ?})";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    CHECK(context.count == 2);
    CHECK(context.parts[0] == format);
    CHECK(context.parts[1] == format + 10);
    CHECK(context.parts[2] == format + sizeof(format));
}

