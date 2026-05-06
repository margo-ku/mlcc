# mlcc — My Little C Compiler

A compiler for a subset of C targeting **AArch64 (ARM64)**, written in C++20.
Built with Flex + Bison for lexing/parsing, generates native AArch64 assembly,
and delegates preprocessing and final linking to `clang`.

---

## Requirements

| Tool | Notes |
|------|-------|
| CMake >= 3.16 | Build system |
| Flex | Lexer generator (Homebrew on macOS) |
| Bison | Parser generator (Homebrew on macOS) |
| Clang | Used for preprocessing (`clang -E`) and final assembly/linking |
| C++20 compiler | To build the compiler itself |

On macOS, install the dependencies with Homebrew:

```bash
brew install cmake flex bison
```

---

## Building

```bash
cmake -B build
cmake --build build
```

The compiled binary will be placed at `build/mlcc`.

---

## Usage

```
mlcc [flags] <source-file> [<source-file> ...]
```

### Flags

| Flag | Description |
|------|-------------|
| `-o <file>` | Write output to `<file>` |
| `-c` | Compile to object file (`.o`), skip linking |
| `--print-ast` | Print the AST, then continue compilation |
| `--print-ast-only` | Print the AST and exit without compiling |
| `--keep-asm` | Keep the generated assembly file (`.s`) |
| `--keep-tac` | Keep the generated TAC files (`.tac.txt`) |
| `--debug` | Print verbose pipeline progress |
| `-p` | Enable parser trace (Bison debug output) |
| `-s` | Enable scanner trace (Flex debug output) |

### Examples

Compile and link a single file:

```bash
./build/mlcc program.c
./program
```

Compile to an object file:

```bash
./build/mlcc -c program.c -o program.o
```

Compile multiple files and link together:

```bash
./build/mlcc -c foo.c -o foo.o
./build/mlcc -c bar.c -o bar.o
clang foo.o bar.o -o program
```

Print the AST without compiling:

```bash
./build/mlcc --print-ast-only program.c
```

Inspect intermediate output:

```bash
./build/mlcc --keep-tac --keep-asm program.c
# produces: program.tac.txt, program.s, program
```

---

## Supported Language Features

### Types

| Type | Description |
|------|-------------|
| `int` | 32-bit signed integer |
| `long` | 64-bit signed integer |
| `unsigned int` | 32-bit unsigned integer |
| `unsigned long` | 64-bit unsigned integer |
| `double` | 64-bit floating-point |
| `void` | For function return types and `void` parameter lists |
| `T *` | Pointer to `T` (multi-level pointers supported) |
| `T[N]` | Fixed-size array of `N` elements |

### Storage Classes

- `static` — file-scope or local static storage
- `extern` — external linkage declaration

### Operators

- **Arithmetic**: `+` `-` `*` `/` `%`
- **Bitwise**: `&` `|` `^` `~` `<<` `>>`
- **Logical**: `&&` `||` `!`
- **Comparison**: `<` `<=` `>` `>=` `==` `!=`
- **Other**: `=` (assignment), `? :` (ternary), `&`/`*` (address-of/dereference), `(type)` (cast), `arr[i]` (subscript)

---

## Known Limitations

The following features are not yet supported but will be added later:

- `struct`, `union`, `enum`
- `char`, `short`, `float`
- String literals
- Hexadecimal and octal integer literals
- Increment/decrement operators (`++`, `--`)
- Compound assignment operators (`+=`, `-=`, etc.)
- `sizeof`
- `goto`
- Dynamic memory allocation (`malloc` / `free`)

---

## Compiler Pipeline

```
source.c
   │
   ▼
clang -E                      ← macro expansion, #include processing
   │
   ▼
Flex (lexer)                  ← tokenisation
   │
   ▼
Bison LALR(1) (parser)        ← builds AST
   │
   ▼
Semantic analysis
  ├─ Symbol resolution
  ├─ Type checking
  └─ Loop analysis (break/continue validation)
   │
   ▼
TAC generation                ← Three-Address Code (per function)
   │
   ▼
TAC optimisation
  ├─ Constant folding
  └─ Unreachable code elimination (CFG-based)
   │
   ▼
AArch64 codegen               ← .s file
   │
   ▼
clang                         ← assemble + link → final binary
```

---

## Target Architecture

The compiler targets **AArch64 (ARM64)** and follows the **AAPCS64** calling convention:

- Integer arguments in `x0`–`x7` (64-bit) / `w0`–`w7` (32-bit)
- Floating-point arguments in `d0`–`d7`
- Return values in `x0` / `w0` / `d0`
- `x29` — frame pointer, `x30` — link register
- Stack grows downward; 16-byte aligned at calls
