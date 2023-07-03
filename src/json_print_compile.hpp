#include <cstddef>
#include <stdexcept>

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
