#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>
#include <map>
#include <stdexcept>
#include <string>
#include <string.h>
#include <unordered_map>
#include <utility>
#include <vector>

#define JP_WHITESPACE ' ': case '\t': case '\r': case '\n'
#define JP_OTHER_CONTROL '\x00': case '\x01': case '\x02': case '\x03': \
    case '\x04': case '\x05': case '\x06': case '\x07': \
        /* \b */     /* \t */     /* \n */ case '\x0B': \
        /* \f */     /* \r */ case '\x0E': case '\x0F': \
    case '\x10': case '\x11': case '\x12': case '\x13': \
    case '\x14': case '\x15': case '\x16': case '\x17': \
    case '\x18': case '\x19': case '\x1A': case '\x1B': \
    case '\x1C': case '\x1D': case '\x1E': case '\x1F'
#define JP_CONTROL JP_OTHER_CONTROL:             case '\b':   \
    case '\t':   case '\n':   case '\f':   case '\r'
#define JP_NONZERO_DIGIT '1': case '2': case '3': case '4': \
         case '5': case '6': case '7': case '8': case '9'
#define JP_DIGIT '0': case JP_NONZERO_DIGIT
#define JP_HEX_ALPHA 'a': case 'b': case 'c': case 'd': case 'e': case 'f': \
             case 'A': case 'B': case 'C': case 'D': case 'E': case 'F'

#ifndef JP_MAX_PLACEHOLDERS
#define JP_MAX_PLACEHOLDERS 14
#endif

/* CONSTEXPR PARSER */

namespace JsonPrint {

/**
 * "Parsed" JSON format string structure
*/
struct json_print_context {
    /** 
     * Locations of each placeholder. 
     * First part is the start of the format string. 
     * Last part is the "end" of the format string 
     */
    const char* parts[JP_MAX_PLACEHOLDERS + 2]; 

    /** Number of parts (or number of placeholders plus one) */
    size_t count;
};

namespace detail {

constexpr bool is_whitespace(char c) {
    switch (c) {
        case JP_WHITESPACE:
            return true;
        default: 
            return false;
    }
}

constexpr bool is_decimal(char c) {
    switch(c) {
        case JP_DIGIT:
            return true;
        default:
            return false;
    }
}

constexpr bool is_hexadecimal(char c) {
    switch (c) {
        case JP_DIGIT:
        case JP_HEX_ALPHA:
            return true;
        default:
            return false;
    }
}

constexpr bool is_string_char(char c) {
    switch (c) {
        case JP_CONTROL:
        case '\\':
        case '"':
            return false;
        
        default:
            return true;
    }
}

constexpr const char* skip_whitespace(const char* begin, const char* end) {
    for (; begin != end && is_whitespace(*begin); begin++) {}
    return begin;
}

constexpr const char* skip_decimal(const char* begin, const char* end) {
    for (; begin != end && is_decimal(*begin); begin++) {}
    return begin;
}

constexpr const char* parse_string(const char* begin, const char* end) {
    begin++; // skip opening quote
    while (begin != end) {
        switch(*begin) {
            case JP_CONTROL:
                throw std::runtime_error("control characters not allowed inside strings");
            case '\\':
                begin++;
                if (begin == end)
                    throw std::runtime_error("expected escape character, reached end of text");
                switch (*begin) {
                    case '"':
                    case '\\':
                    case '/':
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':
                        begin++;
                        break;
                    case 'u': {
                        begin++;
                        const char* codepoint_end = begin + 4;
                        if (end < codepoint_end)
                            throw std::runtime_error("expected 4 hexadecimal digits, reached end of text");
                        for (; begin < codepoint_end; begin++) {
                            if (!is_hexadecimal(*begin))
                                throw std::runtime_error("expected hexadecimal digit");
                        }
                        break;
                    }
                    default:
                        throw std::runtime_error("unsupported escape character");
                }
                break;
            case '"':
                return begin + 1;
            default:
                begin++;
        }
    }
    return begin;
}

constexpr const char* parse_number(const char* begin, const char* end) {
    if (*begin == '-') {
        begin++;
        if (begin == end) 
            throw std::runtime_error("expected digit after '-', reached end of text");
    }

    // parse integer part
    switch (*begin) {
        case '0':
            begin++;
            break;

        case JP_NONZERO_DIGIT:
            begin++;
            begin = skip_decimal(begin, end);
            break;

        default: // wouldn't get here if first digit wasn't '-'
            throw std::runtime_error("expected digit after '-'");
    }

    // parse decimal point and decimal part
    if (begin != end && *begin == '.') {
        begin++;

        // parse decimal part
        if (begin == end)
            throw std::runtime_error("expected digit after '.', reached end of text");
        if (!is_decimal(*begin))
            throw std::runtime_error("expected digit after '.'");
        begin++;
        begin = skip_decimal(begin, end);
    }

    // parse exponent
    if (begin != end && (*begin == 'E' || *begin == 'e'))
    {
        begin++;
        if (begin != end && (*begin == '-' || *begin == '+'))
            begin++;
        if (begin == end)
            throw std::runtime_error("expected digit after exponent, reached end of text");
        if (!is_decimal(*begin))
            throw std::runtime_error("expected digit after exponent");
        begin++;
        return skip_decimal(begin, end);
    }

    return begin;
}

constexpr const char* parse_null(const char* begin, const char* end) {
    if (end - begin < 4 && (
        *(begin + 1) != 'u' || 
        *(begin + 2) != 'l' || 
        *(begin + 3) != 'l')) {
        throw std::runtime_error("unrecognized token");
    }
    return begin + 4;
}

constexpr const char* parse_true(const char* begin, const char* end) {
    if (end - begin < 4 && (
        *(begin + 1) != 'r' || 
        *(begin + 2) != 'u' || 
        *(begin + 3) != 'e')) {
        throw std::runtime_error("unrecognized token");
    }
    return begin + 4;
}

constexpr const char* parse_false(const char* begin, const char* end) {
    if (end - begin < 5 && (
        *(begin + 1) != 'a' || 
        *(begin + 2) != 'l' || 
        *(begin + 3) != 's' ||
        *(begin + 4) != 'e')) {
        throw std::runtime_error("unrecognized token");
    }
    return begin + 5;
}

constexpr const char* parse_value(const char* begin, const char* end, json_print_context& context);

constexpr const char* parse_array(const char* begin, const char* end, json_print_context& context) {
    begin++;
    begin = skip_whitespace(begin, end);

    // parse empty array
    if (begin == end)
        throw std::runtime_error("expected value or ']', reached end of text");
    if (*begin == ']') 
        return begin + 1;

    while (true) {
        // parse value
        begin = parse_value(begin, end, context);

        // parse ',' or ']'
        begin = skip_whitespace(begin, end);
        if (begin == end)
            throw std::runtime_error("expected ',' or ']', reached end of text");
        if (*begin == ']') 
            return begin + 1;
        if (*begin != ',') 
            throw std::runtime_error("expected ',' or ']'");    
        begin++;
    }
}

constexpr const char* parse_object(const char* begin, const char* end, json_print_context& context) {
    begin++;
    begin = skip_whitespace(begin, end);

    // parse empty object
    if (begin == end)
        throw std::runtime_error(R"(expected '"' or '}', reached end of text)");
    if (*begin == '}')
        return begin + 1;
    if (*begin != '"')
        throw std::runtime_error(R"(expected '"' or '}')");

    while (true) {
        // parse member name
        begin = parse_string(begin, end);
        begin = skip_whitespace(begin, end);

        // parse ':'
        if (begin == end)
            throw std::runtime_error("expected ':', reached end of text");
        if (*begin != ':')
            throw std::runtime_error("expected ':'");
        begin++;
        
        // parse value
        begin = parse_value(begin, end, context);
        begin = skip_whitespace(begin, end);

        // parse ',' or '}'
        if (begin == end)
            throw std::runtime_error("expected ',' or '}', reached end of text");
        if (*begin == '}')
            return begin + 1;
        if (*begin != ',')
            throw std::runtime_error("expected ',' or '}'");
        begin++;
        begin = skip_whitespace(begin, end);
    }
}

constexpr const char* parse_value(const char* begin, const char* end, json_print_context& context) {
    begin = skip_whitespace(begin, end);
    switch(*begin) {
        case '[':
            return parse_array(begin, end, context);

        case '{':
            return parse_object(begin, end, context);

        case '\"':
            return parse_string(begin, end);

        case '-':
        case JP_DIGIT:
            return parse_number(begin, end);

        case 'n':
            return parse_null(begin, end);

        case 't':
            return parse_true(begin, end);
        
        case 'f':
            return parse_false(begin, end);

        case '?':
            if (context.count == JP_MAX_PLACEHOLDERS) // last slot is reserved for end
                throw std::runtime_error("too many placeholder values");
            context.count++;
            context.parts[context.count] = begin;
            begin++;
            break;

        default:
            throw std::runtime_error("expected value");
    }
    return begin;
}

}

/** 
 * Validates a JSON format string, while marking the location of each placeholder 
 */
constexpr json_print_context compile(const char* begin, const char* end) {
    json_print_context context = { 0 };
    context.parts[0] = begin;
    begin = detail::parse_value(begin, end, context);
    begin = detail::skip_whitespace(begin, end);
    if (begin != end && *begin == '\0')
        begin++;
    if (begin != end)
        throw std::runtime_error("expected end of text, reached additional content");
    context.count++;
    context.parts[context.count] = end;
    return context;
}

template<int N>
constexpr json_print_context compile(const char (&format)[N]) {
    return compile(format, format + N);
}

}

namespace JsonPrint {
namespace detail {

struct string_buffer {
    char* begin;
    char* end;
};

inline void write_char(string_buffer* buffer, const char c) {
    if (buffer->begin < buffer->end)
        *buffer->begin++ = c;
}

inline int write_string(string_buffer* buffer, const char* begin, const char* end) {
    size_t size = (std::min)(end - begin, buffer->end - buffer->begin);
    memcpy(buffer->begin, begin, size);
    buffer->begin += size;
    return size;
}

inline int write_string_unsafe(string_buffer* buffer, const char* text) {
    return write_string(buffer, text, text + strlen(text));
}

template <typename... T>
int write_printf(string_buffer* buffer, const char* format, T&&... args)
{
    int result = (std::min)(
        snprintf(buffer->begin, buffer->end - buffer->begin, format, std::forward<T>(args)...), 
        static_cast<int>(buffer->end - buffer->begin));
    buffer->begin += result;
    return result;
}

}
}

namespace JsonPrint {
namespace detail {

inline void write_char(FILE* f, const char c)
{
    fputc((int)c, f);
}

inline int write_string_unsafe(FILE* f, const char* text)
{
    return fputs(text, f);
}

inline int write_string(FILE* f, const char* begin, const char* end)
{
    return fwrite(begin, end - begin, 1, f);
}

template <typename... T>
int write_printf(FILE* f, const char* format, T&&... args)
{
    return fprintf(f, format, std::forward<T>(args)...);
}

}
}

namespace JsonPrint {
namespace detail {

inline bool is_special_character(const char c) {
    return c < 32 || c == '\\' || c == '\"';
}

template <typename Dest>
void json_print_string(Dest dest, const char* begin, const char* end) {
    write_char(dest, '"');
    for (; begin != end; begin++) {
        const char* c;
        for (c = begin; c != end && !is_special_character(*c); c++) {};
        write_string(dest, begin, c);

        if (c == end)
            break;
        if (c == end - 1 && *c == '\0')
            break;

        switch(*c) {
            case '"':
                write_string_unsafe(dest, R"(\")");
                break;
            case '\\':
                write_string_unsafe(dest, R"(\\)");
                break;
            case '\b':
                write_string_unsafe(dest, R"(\b)");
                break;
            case '\f':
                write_string_unsafe(dest, R"(\f)");
                break;
            case '\n':
                write_string_unsafe(dest, R"(\n)");
                break;
            case '\r':
                write_string_unsafe(dest, R"(\r)");
                break;
            case '\t':
                write_string_unsafe(dest, R"(\t)");
                break;
            default:
                write_printf(dest, "\\%04u", static_cast<unsigned>(*c));
                break;
        }

        begin = c;
    }
    write_char(dest, '"');
}

/* string types */

template <typename Dest>
inline void json_print_arg(Dest dest, const std::string& arg) {
    json_print_string(dest, arg.data(), arg.data() + arg.size());
}

#ifdef __cpp_lib_string_view
template <typename Dest>
inline void json_print_arg(Dest dest, std::string_view arg) {
    json_print_string(dest, arg.data(), arg.data() + arg.size());
}
#endif

template <typename Dest>
inline void json_print_arg(Dest dest, const char* arg) {
    json_print_string(dest, arg, arg + strlen(arg));
}

template <typename Dest>
inline void json_print_arg(Dest dest, const char n) {
    json_print_string(dest, &n, &n + 1);
}

/* number types */

template <typename Dest>
inline void json_print_arg(Dest dest, const unsigned char n) {
    write_printf(dest, "%d", static_cast<int>(n));
}

template <typename Dest>
inline void json_print_arg(Dest dest, bool b) {
    write_string_unsafe(dest, b ? "true" : "false");
}

template <typename Dest>
inline void json_print_arg(Dest dest, std::nullptr_t) {
    write_string_unsafe(dest, "null");
}

template <typename Dest>
inline void json_print_arg(Dest dest, short n) {
    write_printf(dest, "%h", n);
}

template <typename Dest>
inline void json_print_arg(Dest dest, unsigned short n) {
    write_printf(dest, "%hu", n);
}

template <typename Dest>
inline void json_print_arg(Dest dest, const int n) {
    write_printf(dest, "%d", n);
}

template <typename Dest>
inline void json_print_arg(Dest dest, unsigned n) {
    write_printf(dest, "%u", n);
}

template <typename Dest>
inline void json_print_arg(Dest dest, long n) {
    write_printf(dest, "%ld", n);
}

template <typename Dest>
inline void json_print_arg(Dest dest, unsigned long n) {
    write_printf(dest, "%lu", n);
}

template <typename Dest>
inline void json_print_arg(Dest dest, long long n) {
    write_printf(dest, "%lld", n);
}

template <typename Dest>
inline void json_print_arg(Dest dest, unsigned long long n) {
    write_printf(dest, "%llu", n);
}

template <typename Dest, typename T>
inline void json_print_float_arg(Dest dest, T n) {
    if (std::isnan(n) || std::isinf(n))
        json_print_arg(dest, nullptr);
    else
        write_printf(dest, "%g", n);
}

template <typename Dest>
inline void json_print_arg(Dest dest, float n) {
    json_print_float_arg(dest, n); 
}

template <typename Dest>
inline void json_print_arg(Dest dest, double n) {
    json_print_float_arg(dest, n); 
}

template <typename Dest>
inline void json_print_arg(Dest dest, long double n) {
    json_print_float_arg(dest, n); 
}

/* array types */

template <typename Dest, typename T>
inline void json_print_array_arg(Dest dest, T n) {
    write_char(dest, '[');
    auto it = n.begin();
    if (it != n.end()) {
        json_print_arg(dest, *it++);
    }
    while (it != n.end()) {
        write_char(dest, ',');
        json_print_arg(dest, *it++);
    }
    write_char(dest, ']');
}

template <typename Dest, typename T>
inline void json_print_arg(Dest dest, const std::vector<T>& n) {
    json_print_array_arg(dest, n);
}

template <typename Dest, typename T, size_t N>
inline void json_print_arg(Dest dest, const std::array<T, N>& n) {
    json_print_array_arg(dest, n);
}

/* object types */

template <typename Dest, typename T>
inline void json_print_object_arg(Dest dest, T n)
{
    write_char(dest, '{');
    auto it = n.begin();
    if (it != n.end()) {
        json_print_arg(dest, it->first);
        write_char(dest, ':');
        json_print_arg(dest, it->second);
        it++;
    }
    while (it != n.end()) {
        write_char(dest, ',');
        json_print_arg(dest, it->first);
        write_char(dest, ':');
        json_print_arg(dest, it->second);
        it++;
    }
    write_char(dest, '}');
}

template <typename Dest, typename T>
inline void json_print_arg(Dest dest, const std::map<const char*,T>& n)
{
    json_print_object_arg(dest, n);
}

template <typename Dest, typename T>
inline void json_print_arg(Dest dest, const std::map<std::string,T>& n)
{
    json_print_object_arg(dest, n);
}

#ifdef __cpp_lib_string_view
template <typename Dest, typename T>
inline void json_print_arg(Dest dest, const std::map<std::string_view,T>& n)
{
    json_print_object_arg(dest, n);
}
#endif

template <typename Dest, typename T>
inline void json_print_arg(Dest dest, const std::unordered_map<const char*,T>& n)
{
    json_print_object_arg(dest, n);
}

template <typename Dest, typename T>
inline void json_print_arg(Dest dest, const std::unordered_map<std::string,T>& n)
{
    json_print_object_arg(dest, n);
}

#ifdef __cpp_lib_string_view
template <typename Dest, typename T>
inline void json_print_arg(Dest dest, const std::unordered_map<std::string_view,T>& n)
{
    json_print_object_arg(n);
}
#endif

template <typename Dest>
inline void json_print_part(Dest dest, const char* begin, const char* end) {
    write_string(dest, begin, end);
}

}

}

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
