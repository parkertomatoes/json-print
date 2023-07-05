#include <cstdio>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <array>

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
