# `static` vs Anonymous Namespace (`namespace {}`) vs `extern` — Definitive Guide

> **Goal:** A concise, practical, and authoritative reference that clearly distinguishes `static` (at global scope), anonymous (unnamed) namespaces, and `extern` in C++. Includes examples, rules, pitfalls, and best practices for modern C++.

---

## Table of Contents

1. TL;DR
2. Basic Definitions: Scope, Linkage, Lifetime
3. Short Comparison Table
4. Concrete Examples (Multiple Translation Units)
5. Behavior Details and Language Rules
6. Edge Cases & Gotchas
7. Best Practices (Modern C++)
8. Common Linker Errors and How to Fix Them
9. Quick Reference Cheat Sheet
10. FAQ

---

## 1. TL;DR

- **`extern`**: Declares (or refers to) an entity with **external linkage** — the symbol is shared across translation units (TUs). Use it to *declare* globals in headers and *define* them in a single source file. Functions are `extern` by default.
- **`static` (global scope)**: Gives **internal linkage**. The name is visible **only within the translation unit** (source file). Use it to hide file-local symbols. (Note: `static` inside a function changes lifetime, not linkage.)
- **Anonymous (unnamed) namespace**: Anything declared inside `namespace { ... }` has **internal linkage** (effectively file-local). The preferred modern C++ alternative to `static` for file-level hiding.

---

## 2. Basic Definitions: Scope, Linkage, Lifetime

- **Scope**: Where in source code a name is accessible (block, function, class, namespace, file).
- **Linkage**: Whether different declarations of the same name in different translation units refer to the same entity:
  - **External linkage**: Single entity visible across translation units.
  - **Internal linkage**: Separate entity per translation unit.
  - **No linkage**: The identifier cannot be linked (e.g., local variables).
- **Lifetime**: How long an object exists during program execution (e.g., static storage duration means it lives for the entire program).

`extern`, `static`, and unnamed namespaces primarily affect **linkage** and whether storage is shared across TUs.

---

## 3. Short Comparison Table

| Feature                     | `extern`                                                                 | `static` (global)                                                   | `namespace { }` (anonymous)                                             |
|-----------------------------|--------------------------------------------------------------------------|---------------------------------------------------------------------|-------------------------------------------------------------------------|
| **Linkage**                 | External (shared across TUs)                                             | Internal (file-only)                                                | Internal (file-only)                                                    |
| **Defines storage?**        | With initializer: yes (definition). Without: no (declaration)            | Yes, if initializer present                                         | Yes, if initializer present                                             |
| **Default for functions?**  | Yes (functions are `extern` by default)                                  | No (must specify `static`)                                          | No (must put function inside namespace)                                 |
| **Modern C++ recommendation** | Use for shared globals; prefer alternatives to globals when possible     | Use rarely; prefer anonymous namespace                              | **Preferred** for file-local names                                      |

---

## 4. Concrete Examples

### 4.1 `extern` Variable (Header + Two Sources)

**lib.h**
```cpp
// Declaration only
extern int globalCount;
void printCount();
```

**lib.cpp**
```cpp
#include "lib.h"

int globalCount = 42; // Definition (one and only definition)

void printCount() {
    std::cout << globalCount << '\n';
}
```

**main.cpp**
```cpp
#include "lib.h"

int main() {
    printCount();        // Prints 42
    globalCount = 100;   // Modifies the same object in lib.cpp
}
```
**Note**: `extern` in the header is a declaration; the definition (with initializer) must appear in exactly one .cpp file.

### 4.2 `static` at Global Scope

**util.cpp**
```cpp
static int secretCounter = 0; // Internal linkage — visible only in util.cpp

void inc() { ++secretCounter; }
int read() { return secretCounter; }
```

**other.cpp**
```cpp
// This is a DIFFERENT object, even if same name
static int secretCounter = 0;
```
**Note**: Each TU that defines a `static` global gets its own independent copy.

### 4.3 Anonymous Namespace (Modern Alternative to `static`)

**impl.cpp**
```cpp
namespace {
    int helperValue = 10;      // Internal linkage
    void helperFunc() { }
}

void api() {
    helperFunc(); // Accessible only within this TU
}
```
**Note**: `helperValue` and `helperFunc` are visible only inside `impl.cpp`. This is the **preferred modern C++ approach** for file-local symbols.

---

## 5. Language Rules & Behavior (Concise)

- **Functions**: Have external linkage by default (unless declared `static` or inside an anonymous namespace).
- **`static` at file scope**: Gives internal linkage. The symbol cannot be referenced from other TUs.
- **Unnamed namespace**: Every name declared inside has internal linkage. The compiler generates a unique, TU‑local name.
- **`extern`**: Without initializer → declaration (no storage). With initializer → definition.
- **`const` at namespace scope**: `const` variables have **internal linkage by default** in C++. To share them across TUs, use `extern const`.
- **Inline variables (C++17)**: `inline` variables can be defined in headers without ODR violations — they have external linkage but allow multiple identical definitions.

---

## 6. Edge Cases & Gotchas

### 6.1 `const` Variables
```cpp
// header.h
const int N = 5; // Internal linkage in C++ → each TU has its own copy
```
To share a single `N`:
```cpp
// header.h
extern const int N;
// source.cpp
const int N = 5; // Definition
```
Or, in C++17+:
```cpp
inline constexpr int N = 5; // Header-friendly single entity
```

### 6.2 `static` Inside a Function
```cpp
void f() {
    static int counter = 0; // Lifetime = program duration; scope = block
    ++counter;
}
```
This `static` affects **lifetime** (static storage duration), not linkage.

### 6.3 Anonymous Namespace or `static` in Headers
**Do not** put anonymous namespaces or `static` globals in headers for symbols you intend to be unique across the program. Each TU will get its own copy, which is rarely desired. For header-only utilities, use `inline` functions/variables or templates.

### 6.4 `extern` + Initialization
```cpp
extern int x = 5; // This is a DEFINITION, not just a declaration.
```
Placing this in a header included in multiple TUs creates multiple definitions → violation of the One Definition Rule (ODR).

### 6.5 ODR (One Definition Rule)
- Multiple definitions of the same entity with external linkage across TUs violate ODR (unless they are `inline` and identical, as allowed by the standard).
- Internal‑linkage symbols (`static` or inside anonymous namespace) are not subject to ODR across TUs because each TU has its own entity.

---

## 7. Best Practices (Modern C++)

1. **Avoid non‑const globals** when possible. Prefer dependency injection, function parameters, or class members.
2. **For file‑local (private) symbols**: Prefer **anonymous namespace** over `static` for clarity and consistency.
3. **For constants in headers**: Use `inline constexpr` (C++17+) instead of `extern const` + separate definition.
4. **For shared globals**: Declare with `extern` in the header, define exactly once in a .cpp file.
5. **For helper functions used in only one TU**: Place them inside an anonymous namespace.
6. **Avoid `static` for file‑local variables in headers** — each TU gets a separate copy, which may be unexpected.

---

## 8. Common Linker Errors & Fixes

| Error Message                          | Cause                                                                 | Fix                                                                 |
|----------------------------------------|-----------------------------------------------------------------------|---------------------------------------------------------------------|
| `undefined reference to 'foo'`         | Declaration (`extern`) without a definition in any linked TU.         | Provide a definition in a .cpp file and ensure it's linked.         |
| `multiple definition of 'x'`           | Non‑inline symbol defined in multiple TUs (e.g., global in a header). | Move definition to a single .cpp; keep `extern` declaration in header, or use `inline` (C++17+). |
| Symbol conflicts when using libraries  | Accidental external linkage where internal was intended.              | Mark as `static` or wrap in anonymous namespace.                    |

---

## 9. Quick Reference Cheat Sheet

| Declaration                          | Linkage          | Notes                                                                 |
|--------------------------------------|------------------|-----------------------------------------------------------------------|
| `extern int x;`                      | External         | Declaration only (no storage).                                        |
| `int x = 1;`                         | External         | Definition (unless inside `static` or anonymous ns).                  |
| `static int x = 1;`                  | Internal         | Definition, visible only in this TU.                                  |
| `namespace { int x = 1; }`           | Internal         | Modern replacement for `static`.                                      |
| `static void f() {}`                 | Internal         | Function visible only in this TU.                                     |
| `inline int x = 1;` (C++17)          | External         | Can be defined in headers; one entity across TUs.                     |
| `const int N = 5;`                   | Internal         | Each TU gets its own copy (C++ rule).                                 |
| `extern const int N = 5;`            | External         | Single shared constant (define in one .cpp).                          |

---

## 10. FAQ

**Q1: Should I use `static` or anonymous namespace for file‑local symbols?**  
Ans: Prefer **anonymous namespace** in modern C++. It works for all symbols (variables, functions, types) and clearly groups file‑local code.

**Q2: Can I use `extern` in a .cpp file?**  
Ans: Yes, to declare a global defined in another TU. However, typically you'd put `extern` declarations in a shared header.

**Q3: Why does `const` have internal linkage by default?**  
Ans: Historical reasons and to allow safe inclusion in headers (each TU gets its own copy). Use `extern const` or `inline constexpr` for sharing.

**Q4: What about `static` class members?**  
Ans: Class `static` members have class scope and external linkage (unless defined as `inline`). They are unrelated to file‑scope `static`.

**Q5: How do `inline` functions relate to linkage?**  
Ans: `inline` functions have external linkage but can be defined in multiple TUs (must be identical). They are not “file‑local.”

**Q6: Does `static` affect performance?**  
Ans: No direct performance impact. Internal linkage may allow better optimization (since the compiler knows the symbol isn’t used elsewhere).

**Q7: Can I mix `static` and anonymous namespace?**  
Ans: Technically yes, but redundant. `namespace { static int x; }` still has internal linkage, but the `static` is unnecessary.

---

## Summary of Recommendations

1. **Need a symbol in only one TU?** → Use **anonymous namespace**.
2. **Need to share a global variable across TUs?** → Use `extern` declaration in header + definition in one .cpp.
3. **Need a header‑only constant?** → Use `inline constexpr` (C++17+).
4. **Need a function‑local persistent variable?** → Use `static` inside function.
5. **When in doubt, minimize global symbols** — prefer encapsulation and passing parameters.
