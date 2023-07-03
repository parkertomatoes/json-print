#include <utility>
#include "json_print_compile.hpp"
#include "json_print_arg_string.hpp"
#include "json_print_arg_file.hpp"
#include "json_print_arg.hpp"

namespace JsonPrint {
namespace detail {

template <typename Dest, size_t... Is, typename... Ts>
inline void json_print(Dest dest, const json_print_context& context, std::index_sequence<Is...>, Ts&&... args) {
    // print the part of the format string before the first placeholder
    json_print_part(dest, context.parts[0], context.parts[1]);

    // C++14 trick to perform a void action for each template argument.
    // Expands the arguments into a list of "false" that gets optimized away
    // Each argument uses the comma operator to perform a void action, then return false 
    //
    // For each template argument, 
    //   1) prints the argument JSON-formatted, and then 
    //   2) prints the part of the format string after the placeholder
    std::initializer_list<bool> _ { (
        json_print_arg(dest, args),
        json_print_part(dest, context.parts[Is+1] + 1, context.parts[Is+2]),
        false
    )... };
}

template <typename Dest, typename... Ts>
inline void json_print(Dest dest, const json_print_context& context, Ts&&... args) {
    // forward template arguments with index
    detail::json_print(dest, context, std::index_sequence_for<Ts...> {}, std::forward<Ts>(args)...);
}

}

template <typename... Ts>
inline void json_print(const json_print_context& context, Ts&&... args) {
    // forward template arguments with index
    detail::json_print(stdout, context, std::forward<Ts>(args)...);
}

template <typename... Ts>
inline void json_fprint(FILE* file, const json_print_context& context, Ts&&... args) {
    // forward template arguments with index
    detail::json_print(file, context, std::forward<Ts>(args)...);
}

template <typename... Ts>
inline void json_sprint(char* buffer, size_t size, const json_print_context& context, Ts&&... args) {
    // simulate stream with fat pointer
    detail::string_buffer sbuffer = { buffer, buffer + size };
    // forward template arguments with index
    detail::json_print(&sbuffer, context, std::forward<Ts>(args)...);
}

}

#define json_print_c(format, ...) ([&](){ constexpr auto x = JsonPrint::compile(format); JsonPrint::json_fprint(stdout, x, __VA_ARGS__); }())
#define json_fprint_c(file, format, ...) ([&](){ constexpr auto x = JsonPrint::compile(format); JsonPrint::json_fprint(file, x, __VA_ARGS__); }())
#define json_sprint_c(buffer, size, format, ...) ([&](){ constexpr auto x = JsonPrint::compile(format); JsonPrint::json_sprint(buffer, size, x, __VA_ARGS__); }())
