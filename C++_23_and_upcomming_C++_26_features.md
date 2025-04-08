# Ultimate Features of C++23 (upcoming C++26 features too)

Welcome to the ultimate guide to **C++23** features! (C++24/25 is not a standard version — it's C++23 and upcoming C++26.)

---

## Major C++23 Language Features

### 1. Deduction Guides for `std::ranges`
- Better support for CTAD (Class Template Argument Deduction) in ranges.

### 2. `constexpr` Improvements
- More functions and algorithms (`std::vector`, `std::string`, etc.) are now `constexpr`.

### 3. `if consteval`
```cpp
template<typename T>
void func(T t) {
    if consteval {
        // evaluated at compile-time
    } else {
        // evaluated at run-time
    }
}
```
- Choose code path depending on compile-time vs run-time.

### 4. Static `operator()` for Lambdas
```cpp
auto l = []() static { return 42; };
```
- Lambdas can now be marked `static`.

### 5. `explicit(this)` for Conversion Operators
```cpp
struct S {
    explicit(true) operator bool() const { return true; }
};
```
- Control explicitness based on conditions.

### 6. `multidimensional subscript operator`
```cpp
struct Matrix {
    int operator()(size_t i, size_t j) const;
};
```
- Direct multi-index access.

### 7. `auto(x)` — Deducing `this`
```cpp
struct Point {
    auto operator<=>(auto const&) const = default;
};
```
- Deduce `this` automatically inside member functions.

---

## Major C++23 Library Features

### 1. `std::expected`
```cpp
#include <expected>

std::expected<int, std::string> func();
```
- Safer error handling without exceptions.

### 2. `std::print` and `std::println`
```cpp
#include <print>

std::print("Hello {}
", "World");
```
- Fast and type-safe printing.

### 3. `std::move_only_function`
```cpp
std::move_only_function<void()> task = [] { do_work(); };
```
- A function wrapper for move-only callables.

### 4. `std::flat_map` and `std::flat_set`
- Sorted associative containers backed by contiguous storage (vector-like performance).

### 5. `std::generator`
```cpp
std::generator<int> numbers() {
    for (int i = 0; i < 5; ++i)
        co_yield i;
}
```
- Simplified coroutines for sequences.

### 6. Extended Ranges Library
- `views::cartesian_product`, `views::chunk`, `views::slide`, `views::join_with`, etc.

### 7. `std::stacktrace`
```cpp
#include <stacktrace>

std::cout << std::stacktrace::current();
```
- Built-in stack traces for debugging.

---

## Other Enhancements

- Improved Monadic Interfaces (and_then, or_else, transform for expected).
- `std::optional` and `std::variant` enhancements.
- New attributes: `[[assume]]`, `[[no_unique_address]]` enhancements.
- Better chrono utilities: `chrono::utc_clock`, `chrono::tai_clock`, etc.

---

# Upcoming C++26 Highlights (Preview)

- Pattern Matching (similar to `match` in Rust, switch expressions).
- Contracts (preconditions, postconditions, assertions).
- Improved module ecosystem.
- More constexpr containers and algorithms.

---

# Summary

C++23 is a powerful, evolutionary release, focusing on:
- **Better ergonomics** (generators, expected, static lambdas)
- **Better compile-time capabilities**
- **Modern safety and productivity**

**C++ is more elegant, safer, and faster than ever!**
