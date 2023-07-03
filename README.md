# json_print: A printf-like JSON serializer with compile-time validation
Some people just want to printf their JSON, ok? While conventional wisdom says this is a terrible idea and you should use a full-featured JSON library, "printf" is simple, consistent, and you can see what it's going to output.

This library offers the best of both worlds: It's a small, focused, C++14 single-header library that lets you use printf-like string templates for formatting JSON:
```c++
json_print_c(R"({"hello": ?})", 42}); // prints {"hello": 42}
```

And if you accidentally use invalid JSON in your template string?
```c++
json_print_c(R"({hello: ?})", 42}); // missing quotation marks in key
```

You'll get an error at compile time, not a random exception a month later in production. It even includes some context in the error:
```
<source>:271:9: error: expression '<throw-expression>' is not a constant expression
  271 |         throw std::runtime_error(R"(expected '"' or '}')");
      |         ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

Since all the constexpr parsing and validation code melts away under optimization, it doesn't emit much more than it needs to:

```asm
.LC0:
        .string "{\"hello\": ?}"
.LC1:
        .string "%d"
main:
        push    rbx
        mov     edx, 1
        mov     esi, 10
        mov     edi, OFFSET FLAT:.LC0
        mov     rbx, QWORD PTR stdout[rip]
        mov     rcx, rbx
        call    fwrite
        mov     rdi, rbx
        mov     edx, 42
        xor     eax, eax
        mov     esi, OFFSET FLAT:.LC1
        call    fprintf
        mov     rcx, rbx
        mov     edx, 1
        mov     esi, 2
        mov     edi, OFFSET FLAT:.LC0+11
        call    fwrite
        xor     eax, eax
        pop     rbx
        ret
```

## Getting Started
The easiest way to include json_print in your project is to simply copy the header at `json_print/json_print.hpp` into your project. 

Alternatively, you can include this repository as a submodule and reference it from there.

This ilbrary isn't available from popular package managers like vcpkg or conan, _yet_.

## Examples

### Basic Usage
I recommend using raw strings(`R"(...)"`) for the format string, so that quotation marks don't need to be escaped.
```c++
#include "json_print/json_print.hpp"

int main() {
    json_print_c(
        R"({ "name": ?, "age": ? })", 
        "John Doe", 
        42
    ); // Prints { "name": "John Doe", "age": 42 }
}
```

### Vectors and Arrays
`std::vector` and `std::array` are serialized as JSON arrays
```c++
#include "json_print/json_print.hpp"

int main() {
    std::vector<int> fibo = { 1, 1, 2, 3, 5, 8, 13 };
    json_print_c("?", fibo); // Prints [1,1,2,3,5,8,13]

    std::array<int, 2> dimensions = { 1, 2 };
    json_print_c("?", dimensions); // Prints [1,2]
}
```

### Maps
`std::map` and `std::unordered_map` are serialized as JSON objects. The key must be a string type (const char*, std::string, or std::string_view)
```c++
#include "json_print/json_print.hpp"

int main() {
    std::map<const char*, int> fruit = { { "apple": 1 }, { "banana": 2 } };
    json_print_c("?", fruit); // Prints {"apple":1,"banana":2}
}
```

### Writing To A File
json_print supports writing to files opened iwth `fopen`. No support yet for `std::ostream`, unfortunately.
```c++
#include "json_print/json_print.hpp"

int main() {
    FILE* f = fopen("file.txt", "w");
    json_fprint_c(f, "?", "hello"); // Writes "hello" to the file
    fclose(f);
}
```

### Writing to a string buffer
json_print supports writing to a string buffer. No support yet for returning `std::string`, unfortunately. 
```c++
#include "json_print/json_print.hpp"

int main() {
    char buffer[128];
    json_sprint_c(buffer, sizeof(buffer), "?", "hello"); // String now contains "hello"
}
```

### Using The Low-Level API

```c++
#include "json_print/json_print.hpp"

int main() {
    constexpr auto context = JsonPrint::compile(R"{"hello": ?}");
    JsonPrint::json_print(context, "world");
}
```

## API Documentation

### JSON Printing With Compile-Time Validation
For the most basic usage, json_print has 3 macros for writing to the console, files, and strings respectively. Because these are macros, they need to be used as statmenets and not inside an expression.

#### json_print_c
```c++
void json_print_c(const char format[], ...args)
```
Prints JSON text to the console
 * **format** - The template string. Must be valid JSON, except for placeholders marked by "?"" 
 * **args** - Zero or more arguments to substitute the placeholders for. 

#### json_fprint_c
```c++
void json_fprint_c(FILE* file, const char format[], ...args)
```
Writes JSON text to a file
 * **file** - The file to write to
 * **format** - The template string. Must be valid JSON, except for placeholders marked by "?"" 
 * **args** - Zero or more arguments to substitute the placeholders for. 

#### json_sprint_c
```c++
void json_sprint_c(char* buffer, size_t size, const char format[], ...args)
```
Writes JSON text to a string buffer
 * **buffer** - The string to write to
 * **size** - The size of the buffer in bytes
 * **format** - The template string. Must be valid JSON, except for placeholders marked by "?"" 
 * **args** - Zero or more arguments to substitute the placeholders for. 

### Low Level API
The macros use a 2-step process to process the format string, then print it with arguments. This low-level API can be used to validate the format string at run-time if desired, or to use the same processed format string multiple times. 

#### JsonPrint::compile
```c++
namespace JsonPrint {
    constexpr json_print_context compile(const char format[]);
}
```
Validates a JSON format string, and converts it into parts. *note*: If the result is assigned to a constexpr context, then the validation and processing will take place at compile-itme.
 * **format** - The template string. Must be valid JSON, except for placeholders marked by "?"" 
 * **returns** - A data structure containing the processed data.

#### JsonPrint::json_print
```c++
namespace JsonPrint {
    void json_print(const json_print_context& context, ...args);
}
```
Prints JSON text to the console
 * **context** - A format string that has been process with `JsonPrint::compile`
 * **args** - Zero or more arguments to substitute the placeholders for. 

#### JsonPrint::json_fprint
```c++
namespace JsonPrint {
    void json_fprint(FILE* file, const json_print_context& context, ...args);
}
```
Prints JSON text to the console
 * **file** - The file to write to
 * **context** - A format string that has been process with `JsonPrint::compile`
 * **args** - Zero or more arguments to substitute the placeholders for. 

#### JsonPrint::json_sprint
Writes JSON text to a string buffer

```c++
namespace JsonPrint {
    void json_sprint(
        char* buffer,  
        size_t size, 
        const json_print_context& context, 
        ...args
    );
}
```
 * **buffer** - The string to write to
 * **size** - The size of the buffer in bytes
 * **context** - A format string that has been process with `JsonPrint::compile`
 * **args** - Zero or more arguments to substitute the placeholders for. 

## When To Use json_print:
  * If you prefer the readability of printf to DSLs and serializer APIs
  * If your dignity is offended by having to package a full-featured JSON library with your console utility

## When Not To Use json_print:
 * If you need a mature, bullet-proof, battle-tested JSON serializer for production. 
 * If you want to do anything else with JSON than just printing.
 * If you need auto-mapping between JSON and custom data structures
 * If you think the output will be ultra-optimized

## Developing
This library is header-only and only uses the standard library, so only a few dependencies are needed to get started
 * A command-line C++ toolchain
 * [cmake](https://cmake.org/)
 * [cpp-merge](https://github.com/FastAlien/cpp-merge)

The headers are are located in `<root>/src`. The top-level CMakeLists.txt simply merges the headers into a single header at `<root>/json_print/json_print.hpp` using cpp-merge:
```
mkdir build
cd build
cmake ..
cmake --build .
```

There is a separate test project with unit tests located at `<root>/test`. Because the `doctest` library is included as a submodule, remember to initialize the submodule once before starting
```
git submodule update --init --recursive
```

Then build with CMake and run with CTest
```
cd test
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
ctest
```

## License
json_print is MIT licensed. See LICENSE for details

## Contributing
Thanks! See CONTRIBUTING.md for details.
