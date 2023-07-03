#include <cstdio>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <array>

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
