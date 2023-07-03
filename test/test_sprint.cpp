#include "doctest/doctest.h"
#include "../src/json_print.hpp"

TEST_CASE("should print a literal with no placeholders") {
    char buffer[128] = { 0 };
    const char format[] = R"({"hello": "world"})";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context);
    CHECK(std::string(buffer) == format);
}

TEST_CASE("should print an integer") {
    char buffer[128] = { 0 };
    const char format[] = "?";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, 42);
    CHECK(std::string(buffer) == "42");
}

TEST_CASE("should print a const char*") {
    char buffer[128] = { 0 };
    const char format[] = "?";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, "hello world!");
    CHECK(std::string(buffer) == R"("hello world!")");
}

TEST_CASE("should print an std::string") {
    char buffer[128] = { 0 };
    const char format[] = "?";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, std::string("hello world!"));
    CHECK(std::string(buffer) == R"("hello world!")");
}

TEST_CASE("should print an std::string_view") {
    char buffer[128] = { 0 };
    const char format[] = "?";
    std::string_view data = "hello world!";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, data);
    CHECK(std::string(buffer) == R"("hello world!")");
}

TEST_CASE("should print a boolean") {
    char buffer[128] = { 0 };
    const char format[] = "?";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, true);
    CHECK(std::string(buffer) == "true");
}

TEST_CASE("should print null") {
    char buffer[128] = { 0 };
    const char format[] = "?";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, nullptr);
    CHECK(std::string(buffer) == "null");
}

TEST_CASE("should print a vector<int>") {
    char buffer[128] = { 0 };
    const char format[] = "?";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, std::vector<int> { 24, 42 });
    CHECK(std::string(buffer) == "[24,42]");
}

TEST_CASE("should print an array<bool>") {
    char buffer[128] = { 0 };
    const char format[] = "?";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, std::array<bool, 3> { false, true, false });
    CHECK(std::string(buffer) == "[false,true,false]");
}

TEST_CASE("should print a map<std::string, int>") {
    char buffer[128] = { 0 };
    const char format[] = "?";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, std::map<std::string, int> { {"a", 24}, {"b", 42} });
    CHECK(std::string(buffer) == R"({"a":24,"b":42})");
}

TEST_CASE("should print a map<const char*, int>") {
    char buffer[128] = { 0 };
    const char format[] = "?";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, std::map<const char*, int> { {"a", 24}, {"b", 42} });
    CHECK(std::string(buffer) == R"({"a":24,"b":42})");
}

TEST_CASE("should print a map<std::string_view, int>") {
    char buffer[128] = { 0 };
    const char format[] = "?";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, std::map<std::string_view, int> { {"a", 24}, {"b", 42} });
    CHECK(std::string(buffer) == R"({"a":24,"b":42})");
}

TEST_CASE("should print an array nested within a map") {
    char buffer[128] = { 0 };
    const char format[] = "?";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, std::map<const char*, std::vector<int>> { {"a", { 24 } }, {"b", { 42,25 } } });
    CHECK(std::string(buffer) == R"({"a":[24],"b":[42,25]})");
}

TEST_CASE("should print value inside an array") {
    char buffer[128] = { 0 };
    const char format[] = "[?]";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, 42);
    CHECK(std::string(buffer) == "[42]");
}

TEST_CASE("should print value inside an object") {
    char buffer[128] = { 0 };
    const char format[] = R"({"a":?})";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, 42);
    CHECK(std::string(buffer) == R"({"a":42})");
}

TEST_CASE("should print value inside an object") {
    char buffer[128] = { 0 };
    const char format[] = R"({"a":?})";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, 42);
    CHECK(std::string(buffer) == R"({"a":42})");
}

TEST_CASE("should print multiple placeholders") {
    char buffer[128] = { 0 };
    const char format[] = "[?,?,?]";
    JsonPrint::json_print_context context = JsonPrint::compile(format, format + sizeof(format));
    json_sprint(buffer, sizeof(buffer), context, 42, false, std::vector<std::string> { "hello" } );
    CHECK(std::string(buffer) == R"([42,false,["hello"]])");
}