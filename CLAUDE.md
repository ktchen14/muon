# Muon Programming Language - Claude Notes

## Project Overview
Muon is a programming language implementation with:
- Custom lexer/tokenizer that outputs detailed token position information
- Parser that builds an abstract syntax tree (AST)
- Support for various language constructs including variables, vectors, records, and lambda expressions

## Build System
- Uses CMake for building
- Main executable: `build/test/muon`
- Build directory: `build/`
- Test with: `cd build && ctest --output-on-failure`

## Testing Infrastructure
- **Test runner**: `test/run_tests.sh`
- **Test format**: Custom text-based format in `test/basic_tests.txt`
- **Test structure**:
  ```
  Test: Test Name
    [source code]
    
    Expected Stdout:
    [expected stdout output]
    
    Expected Stderr:
    [expected stderr output including tokens and AST]
  ```

## Language Features Supported
Based on `test/basic_tests.txt`, Muon supports:

1. **Variable definitions**: `define x = 42`
2. **Boolean literals**: `define flag = true`
3. **Vectors**: `define vec = [1, 2, 3]` and `define empty = []`
4. **Records**: `define point = (x: 42, y: true)` and `define unit = ()`
5. **Lambda expressions**: `define identity = lambda x = x`
6. **Multiple statements**: Multiple define statements in sequence

## Output Format
The Muon interpreter outputs:
1. **Token stream**: Detailed position and type information
   - Format: `[start_pos + length]: token_type(details)`
   - Example: `[0 + 6]: define`, `[11 + 2]: INTEGER_LITERAL(integer = 42)`

2. **Abstract Syntax Tree**: Hierarchical structure with 2-space indentation per level
   - Format: `NodeType(details) #node_id`
   - Example:
     ```
     Script:
       DefineStmt(name = x) #1
         IntegerExpr(data = 42) #0
     ```

## Test Runner Behavior
- Strips 2-space indentation from test content (preserves internal indentation)
- Compares expected vs actual stdout/stderr
- Uses diff format to show mismatches
- Whitespace handling is critical for test passing

## Common Commands
- Build: `cmake --build build`
- Test: `cd build && ctest --output-on-failure`
- Run individual file: `build/test/muon filename.muon`
