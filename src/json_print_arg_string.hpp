#include <cstdio>
#include <string>
#include <string.h>
#include <map>
#include <unordered_map>
#include <vector>
#include <array>
#include <algorithm>

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
