# C++ `std::span` - Complete Guide with Tips & Tricks (C++23)

## 📋 Table Of Contents
- [Overview](#overview)
- [Basic Usage](#basic-usage)
- [Common Patterns](#common-patterns)
- [Performance Tips](#performance-tips)
- [Safety Considerations](#safety-considerations)
- [Advanced Techniques](#advanced-techniques)
- [Common Pitfalls](#common-pitfalls)
- [C++23 Updates](#c++23-updates)
- [Examples](#examples)

## Overview

`std::span` is a C++20 feature that provides a non-owning view over a contiguous sequence of objects. In C++23, it has been enhanced with additional features and improvements.

### Key Characteristics
- **Non-owning**: Doesn't manage memory
- **Lightweight**: Typically 2 pointers (or pointer + size)
- **Bounds-checking**: Optional with `at()` or `operator[]` + `_UNCHECKED`
- **Interoperability**: Works with arrays, vectors, and C-style arrays

## Basic Usage

### Declaration & Initialization
```cpp
#include <span>
#include <vector>
#include <array>

// From C-style array
int my_array[] = {1, 2, 3, 4, 5};
std::span<int> span_1{my_array};  // Size deduced

// From std::array
std::array<float, 6> my_std_array = {1.0f, 2.0f, 3.0f};
std::span<float> span_2{my_std_array};

// From std::vector
std::vector<double> my_vector = {1.0, 2.0, 3.0};
std::span<double> span_3{my_vector};

// From pointer and size
int* data_ptr = new int[10];
std::span<int> span_4{data_ptr, 10};

// Const span (read-only view)
std::span<const int> const_span = span_1;
```

### Fixed-size vs Dynamic Extent
```cpp
// Dynamic extent (runtime size, default)
std::span<int> dynamic_span;

// Fixed extent (compile-time size)
std::span<int, 5> fixed_span;  // Always contains 5 elements

// Deduction guides
auto span_auto_1 = std::span{my_array};     // std::span<int, 5>
auto span_auto_2 = std::span{my_vector};    // std::span<double, dynamic_extent>
```

## Common Patterns

### Function Parameters (Replacing Pointer+Size)
```cpp
// Old way (error-prone)
void ProcessOld(int* data, size_t size);

// New way (safer)
void ProcessNew(std::span<int> data)
{
    for (auto& element : data)
    {
        // Process element
    }
}

// Const correctness
void ReadData(std::span<const int> data);
void ModifyData(std::span<int> data);
```

### Subviews And Slicing
```cpp
std::span<int> full_span{my_array, 10};

// First N elements
auto first_three = full_span.first(3);

// Last N elements  
auto last_two = full_span.last(2);

// Subspan (offset, count)
auto middle = full_span.subspan(3, 4);

// All operations are bounds-checked at runtime
```

### Iteration
```cpp
std::span<int> data = /* ... */;

// Range-based for loop
for (auto& element : data)
{
    element *= 2;
}

// With index
for (size_t i = 0; i < data.size(); ++i)
{
    data[i] = i * 10;
}

// Using algorithms
std::sort(data.begin(), data.end());
auto it = std::find(data.begin(), data.end(), 42);
```

## Performance Tips

### 1. Pass By Value
```cpp
// GOOD: span is small (typically 2 pointers)
void ProcessData(std::span<int> data);

// BAD: Unnecessary reference
void ProcessData(const std::span<int>& data);  // Extra indirection
```

### 2. Use `std::as_bytes()` / `std::as_writable_bytes()`
```cpp
std::span<int> data = /* ... */;

// For binary I/O or low-level operations
auto read_only_bytes = std::as_bytes(data);           // Read-only bytes
auto writable_bytes = std::as_writable_bytes(data); // Writable bytes
```

### 3. Avoid Unnecessary Bounds Checks
```cpp
std::span<int> data = /* ... */;

// Use unchecked access when bounds are already verified
if (index < data.size())
{
    // Safe to use unchecked
    data[index];  // Or data.data()[index] for maximum performance
}

// For performance-critical loops
for (size_t i = 0; i < data.size(); ++i)
{
    auto* raw_ptr = data.data();  // Single pointer acquisition
    raw_ptr[i] = TransformValue(raw_ptr[i]);
}
```

### 4. Fixed Extent For Compile-time Optimization
```cpp
template<size_t N>
void ProcessFixed(std::span<int, N> data)
{
    // Compiler knows size at compile-time
    // May enable better optimization
}

// Use when size is known at compile-time
std::array<int, 1024> buffer;
ProcessFixed(std::span{buffer});
```

## Safety Considerations

### 1. Lifetime Management
```cpp
// DANGER: span outlives data
std::span<int> GetInvalidSpan()
{
    std::vector<int> local_data = {1, 2, 3};
    return {local_data};  // Dangling span!
}

// SAFE: Return by value of underlying container
std::vector<int> GetData()
{
    return {1, 2, 3, 4, 5};
}
```

### 2. Bounds Checking
```cpp
std::span<int> data = /* ... */;

// Checked access (throws std::out_of_range)
try
{
    int value = data.at(100);  // Throws if out of bounds
}
catch (const std::out_of_range& error)
{
    // Handle error
}

// Unchecked access (undefined behavior if out of bounds)
int value = data[100];  // UB if out of bounds

// Safe pattern: check then access
if (index < data.size())
{
    int value = data[index];  // Safe
}
```

### 3. Const Correctness
```cpp
void BadFunction(std::span<int> data)
{
    data[0] = 42;  // Modifies original data
}

void GoodFunction(std::span<const int> data)
{
    // Can only read, prevents accidental modification
    int value = data[0];
}

// Use const spans for read-only parameters
```

## Advanced Techniques

### 1. Type Erasure For Heterogeneous APIs
```cpp
void ProcessBytes(std::span<const std::byte> data)
{
    // Can accept any data type via as_bytes()
}

template<typename T>
void SerializeObject(const T& object)
{
    auto bytes = std::as_bytes(std::span{&object, 1});
    ProcessBytes(bytes);
}
```

### 2. Multi-dimensional Views
```cpp
// 2D span using spans of spans
using Matrix = std::span<std::span<int>>;

void ProcessMatrix(Matrix matrix)
{
    for (auto row : matrix)
    {
        for (auto& element : row)
        {
            // Process element
        }
    }
}

// Or using a flat buffer with row stride
void ProcessFlatMatrix(std::span<int> data, size_t rows, size_t cols)
{
    for (size_t i = 0; i < rows; ++i)
    {
        auto row = data.subspan(i * cols, cols);
        // Process row
    }
}
```

### 3. Interoperability With Legacy Code
```cpp
// C interface
void LegacyFunction(int* data, int size);

// Modern wrapper
void ModernWrapper(std::span<int> data)
{
    LegacyFunction(data.data(), static_cast<int>(data.size()));
}

// Reverse: from legacy to span
void LegacyCallback(int* data, int size)
{
    std::span<int> data_span{data, static_cast<size_t>(size)};
    // Modern processing
}
```

### 4. Custom View Types
```cpp
template<typename T>
class StridedSpan
{
    std::span<T> data_;
    size_t stride_;
    
public:
    class Iterator
    {
        // custom iterator with stride implementation
        T* current_;
        size_t stride_;
        
    public:
        Iterator(T* ptr, size_t stride) : current_(ptr), stride_(stride)
        {
        }
        
        // Iterator operations...
    };
    
    Iterator begin()
    {
        return Iterator(data_.data(), stride_);
    }
    
    Iterator end()
    {
        return Iterator(data_.data() + data_.size() * stride_, stride_);
    }
    
    T& operator[](size_t index)
    {
        return data_[index * stride_];
    }
};
```

## Common Pitfalls

### 1. Lifetime Issues
```cpp
// WRONG: Temporary container
std::span<int> bad_span = std::vector<int>{1, 2, 3};

// WRONG: Local variable
std::span<int> CreateDanglingSpan()
{
    int local_array[10];
    return {local_array};  // Dangling!
}

// RIGHT: Ensure span doesn't outlive data
class DataProcessor
{
    std::vector<int> owned_data_;  // Owns data
    std::span<int> data_view_;     // Views owned data
    
public:
    DataProcessor() : data_view_{owned_data_}
    {
    }
};
```

### 2. Assuming Null-Termination
```cpp
// BAD: span doesn't guarantee null termination
std::span<char> string_span = /* ... */;
std::cout << string_span.data();  // Could overflow!

// GOOD: Use string_view for strings
std::string_view string_view{string_span.data(), string_span.size()};
```

### 3. Misusing Fixed Extent
```cpp
// Only use fixed extent when size is truly compile-time constant
std::vector<int> dynamic_data;
std::span<int> dynamic_view{dynamic_data};  // GOOD

std::array<int, 10> fixed_data;
std::span<int, 10> fixed_view{fixed_data};  // GOOD

// Don't force fixed extent on dynamic data
int* dynamic_ptr = new int[ComputeSize()];
std::span<int> correct_view{dynamic_ptr, ComputeSize()};  // GOOD
// std::span<int, ???> wrong_view{dynamic_ptr, ComputeSize()};  // BAD
```

### 4. Overlooking Alignment
```cpp
struct t_AlignedData
{
    alignas(64) int data[16];
};

std::span<t_AlignedData> aligned_span;
// as_bytes() preserves alignment information
auto bytes = std::as_bytes(aligned_span);
```

## C++23 Updates

### 1. `std::span` Constructors From Other Containers
```cpp
// C++23: More container types supported
std::span<int> from_span = /* ... */;
std::span<const int> const_from_span{from_span};  // Always worked

// C++23: Additional interoperability
#include <ranges>

auto range_view = std::views::iota(0, 10);
// std::span<int> from_range{range_view};  // Not directly, but patterns exist
```

### 2. Range Compatibility Enhancements
```cpp
// C++23: Better integration with ranges library
std::span<int> data_span = /* ... */;

// Use with range adaptors
auto filtered = data_span | std::views::filter([](int x) { return x > 0; });
auto transformed = data_span | std::views::transform([](int x) { return x * 2; });

// Note: These return range views, not spans
```

### 3. `mdspan` Integration (Multi-dimensional)
```cpp
// C++23 introduces std::mdspan for multi-dimensional arrays
#include <mdspan>

std::array<int, 24> buffer;
std::mdspan<int, std::extents<size_t, 3, 4, 2>> matrix_3d{buffer.data()};

// Convert mdspan slice to span
auto slice_2d = matrix_3d[1];  // Returns mdspan
// Can extract 1D slices as spans when needed
```

### 4. Improved `as_bytes` And `as_writable_bytes`
```cpp
// C++23: More explicit about byte representation
std::span<float> float_data = /* ... */;

auto bytes = std::as_bytes(float_data);
static_assert(std::is_same_v<decltype(bytes)::element_type, const std::byte>);

// Better const propagation
std::span<const float> const_float_data = float_data;
auto const_bytes = std::as_bytes(const_float_data);  // Always const std::byte
```

### 5. `strided_span` Proposal (Future)
```cpp
// Not in C++23, but proposed for future standards
// Example of what it might look like:
namespace std
{
    template<class T, class Extents, class Layout, class Accessor>
    class mdspan;
    
    // Special layout for strided access
    using strided_layout = /* ... */;
}

// Potential usage:
// std::mdspan<int, std::dextents<2>, std::strided_layout> strided_view;
```

## Best Practices Summary

1. **Prefer `std::span` over `(pointer, size)` pairs**
2. **Pass by value, not by reference**
3. **Use `const std::span<const T>` for read-only parameters**
4. **Always consider lifetime of referenced data**
5. **Use `at()` during development, switch to `operator[]` in production if needed**
6. **Leverage subviews instead of manual pointer arithmetic**
7. **Use `std::as_bytes()` for binary operations**
8. **Consider fixed extent when size is compile-time constant**
9. **Don't use spans for null-terminated strings (use `string_view`)**
10. **Document ownership/lifetime expectations in APIs**
11. **In C++23, leverage range adaptors for complex transformations**
12. **Consider `mdspan` for multi-dimensional data when available**

---
