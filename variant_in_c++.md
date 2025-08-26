# `std::optional` The typesafe union (Modern C++ up to C++26)

> `std::optional<T>` is a lightweight, type-safe wrapper that models “either a `T` **or nothing**.” It explicitly encodes *absence of value* in the type system.

---

## 1. What It Is & Where to Find It

* **Header:** `#include <optional>`
* **Name:** `std::optional<T>`
* **Semantics:** Either:

  * **Engaged** → contains a value of type `T`.
  * **Disengaged** → contains nothing. Accessing via `.value()` in this state throws `std::bad_optional_access`.

---

## 2. Why Use `std::optional`

✔️ Explicitly models “no result” (instead of sentinels like `-1` or null pointers).
✔️ Safer and clearer than raw pointers.
✔️ Improves readability — APIs document the possibility of missing data.

---

## 3. Idiomatic Uses

### Function Return (when a result may be missing)

```cpp
std::optional<User> find_user(std::string_view name) {
    if (exists(name)) return User{name};
    return std::nullopt;
}

if (auto maybe = find_user("alice")) {
    use(*maybe);
}
```

### Optional Members

```cpp
struct Config {
    std::optional<int> max_conn;           // use default if empty
    std::optional<std::string> log_path;   // optional path
};
```

### Lazy Initialization

```cpp
std::optional<std::string> cache;
if (!cache) cache = compute();
```

### Monadic Style (C++23)

```cpp
auto result = parse_int(s)
                .transform([](int x){ return x*2; })
                .and_then(try_compute);
```

### Ranges (C++26)

```cpp
for (auto& v : maybe_value) { /* runs if engaged */ }
```

---

## 4. Mini API Cheat-Sheet

* `std::nullopt` → empty state.
* `opt.has_value()` / `if(opt)` → check engagement.
* `*opt`, `opt->` → unchecked access.
* `opt.value()` → checked (throws if empty).
* `opt.value_or(default)` → fallback.
* `opt.emplace(args...)` → construct in-place.
* `opt.reset()` → disengage.
* `transform`, `and_then`, `or_else` → monadic helpers (C++23).

---

## 5. Rules & Guidelines

1. Use `optional<T>` when “absence is normal.”
2. Use `expected<T,E>` if you also need to return *why* it failed.
3. Avoid for ownership semantics → prefer `unique_ptr` / `shared_ptr`.
4. Avoid `optional<bool>` → prefer `enum`.
5. For large `T`, prefer move-only wrappers if copying is costly.
6. `optional<T&>` → subtle semantics, check library/compiler support.

---

## 6. Advantages

✅ Clearer APIs.
✅ Type safety.
✅ Lightweight (inline storage + flag).
✅ Composable with monadic helpers.

---

## 7. Disadvantages / Pitfalls

⚠️ Not for error reporting → no diagnostic info.
⚠️ Copying large `T` can be costly.
⚠️ Reference vs pointer semantics can confuse.
⚠️ `.value()` throws → avoid for normal control flow.

---

## 8. Tips & Tricks

* Use `if(opt)` or structured bindings for clarity.
* Use `value_or` for concise defaults.
* Use `transform` to avoid manual unwrap-map-wrap.
* Use `and_then` to chain dependent operations.
* Prefer `expected` over `optional` when returning error info.
* In C++26, treat `optional` as a range in algorithms.

---

## 9. C++26 Notes & Evolution

* ✅ `begin()`/`end()` → `optional` is a single-element range.
* ✅ `optional<T&>` wording clarified (check implementation).
* ✅ Ongoing library fixes/hardening — track compiler release notes.

---

## 10. Example (C++23 + C++26)

```cpp
#include <optional>
#include <string>
#include <iostream>

std::optional<int> parse_int(std::string_view s) {
    int v = 0;
    for (char c : s) {
        if (!std::isdigit((unsigned)c)) return std::nullopt;
        v = v*10 + (c - '0');
    }
    return v;
}

int main() {
    std::optional<std::string> s = "123";

    auto result = s
        .transform([](auto& str){ return std::string_view{str}; })
        .and_then(parse_int)
        .transform([](int x){ return x*2; });

    if (result) std::cout << "doubled: " << *result << '\n';

    for (auto& v : result) std::cout << "iterated: " << v << '\n';
}
```

---

## 11. Final Checklist

* ✔️ Use `optional` only for absence of value.
* ✔️ Use `expected` for error + value.
* ✔️ Prefer smart pointers for ownership.
* ✔️ Avoid `optional<bool>` for tri-state.
* ✔️ Watch performance with large `T`.
* ✔️ Confirm library support for C++23/26 helpers.

---
