# AGENTS.md - Muon Programming Language

## Project Overview

Muon is a programming language implementation written in C. It includes:
- Custom lexer/tokenizer (re2c-based)
- Parser building an AST
- Type inductor for type inference
- Code author for LLVM compilation

Standards: C11 for the public API (`muon` library), C23 for main executable and tests. Requires LLVM for code generation.

## Build Commands

- **Build**: `cmake --build build`
- **Test**: `cd build && ctest --output-on-failure`
- **Run file**: `build/test/muon <source_file>`
- **Build test binary only**: `cmake --build build --target main-test`
- **Clean build**: `rm -rf build && mkdir build && cmake -B build`

## Code Style Guidelines

### File Organization

- **Headers** (`header/muon/`): Public API, use `#ifndef MU_X_H` guards
- **Sources** (`source/`): Implementation files
- **Module pattern**: Header-only modules use `#define MUON_X_MODULE` before including

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Types | `PascalCase` | `MuonNode`, `MuonEngine` |
| Functions | `snake_case` with `muon_` prefix | `muon_node_create()` |
| Variables | `snake_case` | `engine`, `node_number` |
| Macros/Enums | `UPPER_SNAKE_CASE` with `MUON_` prefix | `MUON_EACH_EXPR_STEM` |

### Type Usage

- Use `_Bool` instead of C99 `bool`
- Use `size_t` for sizes and counts; `uint64_t`/`uint32_t` for fixed-width integers
- Use `int` for error codes (`0` success, `-1` failure)

### Function Attributes

Use GCC/Clang attributes: `malloc`, `nonnull`, `nonnull(N)`, `pure`, `const`, `returns_nonnull`, `format(printf, N, M)`.

### Import Organization

Order: (1) corresponding header, (2) local headers, (3) parent headers, (4) system headers, (5) LLVM headers. Separate system and LLVM groups with blank lines.

### Error Handling

Use the `goto except` pattern for cleanup:
```c
int function(void) {
    struct thing *result = NULL;
    if ((result = allocate()) == NULL)
        goto except_allocate;
    if (process(result) != 0)
        goto except_process;
    return 0;
except_process:
    deallocate(result);
except_allocate:
    return -1;
}
```

- Return `NULL` for allocation failures (set `errno`)
- Return `-1`/`0` for failure/success

### Control Flow

- Use `if (condition) goto label;` for error handling
- Use `for (;;)` for infinite loops
- Prefer early returns for error cases
- Use `assert()` for internal invariants (not user input)

### Memory Management

- Use arena/area-based allocation where possible (`area_t`)
- Always check allocation return values
- Use compound literals for zero-initialization: `(Type){0}`
- Use designated initializers: `(Type){.field = value}`

### Macro Patterns

Use X-macro style for code generation:
```c
#define MUON_EACH_EXPR_STEM(emit, ...) \
    emit(access, ACCESS, Access, ##__VA_ARGS__) \
    emit(boolean, BOOLEAN, Boolean, ##__VA_ARGS__)
```

### Formatting

- 2-space indentation
- Opening brace on same line for functions
- Space after keywords (`if (`, `while (`, `for (`), no space before function call parens

### Documentation

- Use `/** */` for Doxygen-style docs, `///` for single-line
- Document preconditions, postconditions, and NULL behavior

### Debugging

Debug flags: `muon_debug_stream`, `mu_debug_colorize`, `debug_indent`, `debug_negate`, `debug_shortcore`, `debug_dot`, `debug_scan`. Use the `debug()` macro for output.

## Testing

- **Test runner**: `test/run_tests.sh`
- **Test definitions**: `test/basic_tests.txt`
- **Test structure**:
  ```
  Test: Test Name
    [source code]

    Expected Stdout:
    [expected stdout output]

    Expected Stderr:
    [expected stderr output including tokens and AST]
  ```
- Strips 2-space indentation from test content (preserves internal indentation)
- Compares expected vs actual stdout/stderr using diff
- Whitespace handling is critical for test passing

To run a single test manually:
```bash
echo 'define x = 42' > /tmp/test.muon
build/test/muon /tmp/test.muon
```

## Language Features

Muon supports: variable definitions (`define x = 42`), boolean literals (`true`/`false`), vectors (`[1, 2, 3]`, `[]`), records (`(x: 42, y: true)`, `()`), lambda expressions (`lambda x = x`), and multiple statements in sequence.

## Output Format

1. **Token stream**: `[start_pos + length]: token_type(details)`
   - Example: `[0 + 6]: define`, `[11 + 2]: INTEGER_LITERAL(integer = 42)`
2. **AST**: Hierarchical with 2-space indentation, format `NodeType(details) #node_id`
   - Example: `DefineStmt(name = x) #1` with children indented below

## CMake

- `add_library(muon SHARED)` for the main library
- `add_executable(main source/main.c)` for the CLI
- `find_package(LLVM REQUIRED CONFIG)` for LLVM integration
