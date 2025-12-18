# C++20 Ranges: Complete Guide (Beginner to Master)

## Table of Contents
1. [Introduction](#introduction)
2. [Level 1: Understanding the Problem](#level-1-understanding-the-problem)
3. [Level 2: Basic Views](#level-2-basic-views)
4. [Level 3: Composing Views](#level-3-composing-views)
5. [Level 4: Advanced Views](#level-4-advanced-views)
6. [Level 5: Range Generators](#level-5-range-generators)
7. [Level 6: Range Algorithms](#level-6-range-algorithms)
8. [Level 7: Real-World Examples](#level-7-real-world-examples)
9. [Level 8: Performance & Safety](#level-8-performance-and-safety)
10. [Level 9: Custom Range Adaptors](#level-9-custom-range-adaptors)
11. [Level 10: Advanced Patterns](#level-10-advanced-patterns)
12. [Key Takeaways](#key-takeaways)

---

## Introduction

C++20 Ranges revolutionize how we work with data in C++. They provide:

- ✅ **Composable operations** - Chain transformations together
- ✅ **Lazy evaluation** - Compute only when needed
- ✅ **Better readability** - Left-to-right pipeline flow
- ✅ **Type safety** - Compile-time checks
- ✅ **Efficiency** - No temporary containers
- ✅ **Safety** - No iterator pair mismatches

---

## Level 1: Understanding the Problem

### The Old Way (Pre-C++20)

```cpp
#include <vector>
#include <algorithm>
#include <iostream>

std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Task: Double each number, then keep only those > 10

// OLD WAY: Verbose, requires temporary containers
std::vector<int> temp1;
std::transform(numbers.begin(), numbers.end(), 
              std::back_inserter(temp1),
              [](int n) 
              { 
                  return n * 2; 
              });

std::vector<int> result;
std::copy_if(temp1.begin(), temp1.end(),
            std::back_inserter(result),
            [](int n) 
            { 
                return n > 10; 
            });

// Output: 12 14 16 18 20
```

**Problems:**
- ❌ Creates temporary container (`temp1`)
- ❌ Two separate steps
- ❌ Hard to read (right-to-left)
- ❌ Not efficient (extra memory allocation)

### The New Way (C++20 Ranges)

```cpp
#include <ranges>
#include <vector>
#include <iostream>

std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// NEW WAY: Clean, composable, lazy
auto result = numbers 
    | std::views::transform([](int n) { return n * 2; })
    | std::views::filter([](int n) { return n > 10; });

for(int n : result)
{
    std::cout << n << " ";  // Output: 12 14 16 18 20
}
```

**Advantages:**
- ✅ No temporary containers
- ✅ Lazy evaluation (computed on-demand)
- ✅ Pipeline style (reads left-to-right)
- ✅ Composable operations

---

## Level 2: Basic Views

### 1. `filter` - Select elements matching condition

```cpp
std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Get only even numbers
auto evens = data | std::views::filter([](int n) 
{ 
    return n % 2 == 0; 
});

// Output: 2 4 6 8 10
```

### 2. `transform` - Apply function to each element

```cpp
// Square each number
auto squares = data | std::views::transform([](int n) 
{ 
    return n * n; 
});

// Output: 1 4 9 16 25 36 49 64 81 100
```

### 3. `take` - Take first N elements

```cpp
// Get first 5 elements
auto first_five = data | std::views::take(5);

// Output: 1 2 3 4 5
```

### 4. `drop` - Skip first N elements

```cpp
// Skip first 5 elements
auto skip_five = data | std::views::drop(5);

// Output: 6 7 8 9 10
```

### 5. `reverse` - Reverse order

```cpp
// Reverse the range
auto reversed = data | std::views::reverse;

// Output: 10 9 8 7 6 5 4 3 2 1
```

---

## Level 3: Composing Views

The real power of ranges comes from **composing** multiple views together!

### Example 1: Multiple Filters

```cpp
std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

// Numbers greater than 5 AND even
auto result = numbers
    | std::views::filter([](int n) { return n > 5; })
    | std::views::filter([](int n) { return n % 2 == 0; });

// Output: 6 8 10 12
```

### Example 2: Transform + Filter + Take

```cpp
// Square numbers, keep those > 50, take first 3
auto result = numbers
    | std::views::transform([](int n) { return n * n; })
    | std::views::filter([](int n) { return n > 50; })
    | std::views::take(3);

// Output: 64 81 100
```

### Example 3: Complex Pipeline

```cpp
// Drop 2, reverse, double, filter odd
auto result = numbers
    | std::views::drop(2)              // {3, 4, 5, ..., 12}
    | std::views::reverse              // {12, 11, 10, ..., 3}
    | std::views::transform([](int n) { return n * 2; })  // {24, 22, 20, ...}
    | std::views::filter([](int n) { return n % 2 != 0; }); // odd only

// Note: Result depends on the data flow!
```

---

## Level 4: Advanced Views

### 1. `take_while` - Take until condition fails

```cpp
std::vector<int> nums = {1, 2, 5, 3, 8, 4, 9};

// Take elements while < 6
auto taken = nums | std::views::take_while([](int n) 
{ 
    return n < 6; 
});

// Output: 1 2 5 3
// Stops at 8 because 8 >= 6
```

### 2. `drop_while` - Skip until condition fails

```cpp
// Skip elements while < 6
auto dropped = nums | std::views::drop_while([](int n) 
{ 
    return n < 6; 
});

// Output: 8 4 9
// Starts from 8 (first element >= 6)
```

### 3. `split` - Split range by delimiter

```cpp
std::string text = "Hello world from ranges";

// Split by spaces
auto words = text | std::views::split(' ');

for(auto word : words)
{
    for(char c : word)
    {
        std::cout << c;
    }
    std::cout << "\n";
}

// Output:
// Hello
// world
// from
// ranges
```

### 4. `join` - Flatten nested ranges

```cpp
std::vector<std::vector<int>> nested = {{1, 2}, {3, 4}, {5, 6}};

// Flatten to single range
auto flattened = nested | std::views::join;

// Output: 1 2 3 4 5 6
```

### 5. `keys` and `values` - For maps

```cpp
#include <map>

std::map<std::string, int> scores = 
{
    {"Alice", 95}, 
    {"Bob", 87}, 
    {"Carol", 92}
};

// Extract just keys
auto names = scores | std::views::keys;
// Output: Alice Bob Carol

// Extract just values
auto points = scores | std::views::values;
// Output: 95 87 92
```

---

## Level 5: Range Generators

Views can also **generate** data, not just transform existing ranges!

### 1. `iota` - Generate sequence

```cpp
// Generate numbers 1 to 10
auto seq = std::views::iota(1, 11);

// Output: 1 2 3 4 5 6 7 8 9 10
```

### 2. `iota` unbounded - Infinite sequence

```cpp
// Infinite sequence starting from 1, but take only 10
auto infinite = std::views::iota(1) | std::views::take(10);

// Output: 1 2 3 4 5 6 7 8 9 10
```

### 3. `repeat` - Repeat value

```cpp
// Repeat value 42, five times
auto repeated = std::views::repeat(42) | std::views::take(5);

// Output: 42 42 42 42 42
```

### 4. `empty` - Empty range

```cpp
auto empty_range = std::views::empty<int>;

std::cout << "Size: " << std::ranges::distance(empty_range);
// Output: Size: 0
```

### 5. `single` - Single element

```cpp
auto single = std::views::single(100);

// Output: 100
```

---

## Level 6: Range Algorithms

C++20 also provides range-based algorithms that are safer and cleaner!

### Comparison: Old vs New

```cpp
std::vector<int> data = {5, 2, 8, 1, 9, 3, 7, 4, 6};

// OLD WAY
std::sort(data.begin(), data.end());

// NEW WAY - No iterators needed!
std::ranges::sort(data);

// Output: 1 2 3 4 5 6 7 8 9
```

### Common Range Algorithms

#### 1. `find_if`

```cpp
// Find first element > 5
auto it = std::ranges::find_if(data, [](int n) 
{ 
    return n > 5; 
});

if(it != data.end())
{
    std::cout << "Found: " << *it;
}
```

#### 2. `count_if`

```cpp
// Count even numbers
auto count = std::ranges::count_if(data, [](int n) 
{ 
    return n % 2 == 0; 
});

std::cout << "Even count: " << count;
```

#### 3. `any_of`, `all_of`, `none_of`

```cpp
bool has_large = std::ranges::any_of(data, [](int n) { return n > 5; });
// true

bool all_positive = std::ranges::all_of(data, [](int n) { return n > 0; });
// true

bool none_negative = std::ranges::none_of(data, [](int n) { return n < 0; });
// true
```

#### 4. `copy_if`

```cpp
std::vector<int> odds;

// Copy odd numbers
std::ranges::copy_if(data, 
                     std::back_inserter(odds), 
                     [](int n) 
                     { 
                         return n % 2 != 0; 
                     });

// odds: {1, 3, 5, 7, 9}
```

---

## Level 7: Real-World Examples

### Example 1: Data Processing Pipeline

```cpp
struct Student
{
    std::string name;
    int score;
};

std::vector<Student> students = 
{
    {"Alice", 95}, {"Bob", 67}, {"Carol", 88},
    {"David", 72}, {"Eve", 91}, {"Frank", 55}
};

// Get top 3 passing students (score >= 70)
auto top_students = students
    | std::views::filter([](const Student& s) 
    { 
        return s.score >= 70; 
    })
    | std::views::transform([](const Student& s) 
    { 
        return s.name + ": " + std::to_string(s.score); 
    })
    | std::views::take(3);

// Output:
// Alice: 95
// Carol: 88
// David: 72
```

### Example 2: Text Processing

```cpp
std::string sentence = "The quick brown fox jumps over the lazy dog";

// Get words with more than 3 characters
auto long_words = sentence
    | std::views::split(' ')
    | std::views::filter([](auto word) 
    { 
        return std::ranges::distance(word) > 3; 
    });

// Output: quick brown jumps over lazy
```

### Example 3: Fibonacci Sequence

```cpp
// Generate first 10 Fibonacci numbers
auto fibonacci = std::views::iota(0)
    | std::views::transform([](int n) 
    {
        int a = 0, b = 1;
        for(int i = 0; i < n; ++i)
        {
            int temp = a + b;
            a = b;
            b = temp;
        }
        return a;
    })
    | std::views::take(10);

// Output: 0 1 1 2 3 5 8 13 21 34
```

---

## Level 8: Performance and Safety

### Advantage 1: Const Correctness

```cpp
const std::vector<int> const_data = {1, 2, 3, 4, 5};

// This works! Views don't modify the source
auto doubled = const_data 
    | std::views::transform([](int n) { return n * 2; });

// Output: 2 4 6 8 10
```

### Advantage 2: Lazy Evaluation

```cpp
std::vector<int> big_data(1000);
std::iota(big_data.begin(), big_data.end(), 1);

// Only processes until first match is found!
auto result = big_data
    | std::views::filter([](int n) 
    { 
        std::cout << "[Checking " << n << "] ";
        return n > 995; 
    })
    | std::views::take(1);

for(int n : result)
{
    std::cout << "\nFound: " << n;
}

// Output shows it only checks: 996 997 998...
// Then stops! Doesn't check all 1000 elements.
```

### Advantage 3: Type Safety

```cpp
// Pipeline with type transformations
auto safe_pipeline = std::views::iota(1, 6)
    | std::views::transform([](int n) 
    { 
        return n * 1.5;  // int -> double
    })
    | std::views::filter([](double d) 
    { 
        return d > 3.0;  // Correct type
    });

// Compiler ensures type safety throughout!
```

### Advantage 4: No Iterator Mismatches

```cpp
// OLD WAY - Easy to make mistakes
std::sort(vec1.begin(), vec2.end());  // WRONG! Mixing vectors
std::copy(vec1.begin(), vec1.end(), vec2.begin() + 100);  // WRONG! Out of bounds

// NEW WAY - Can't make these mistakes
std::ranges::sort(vec1);  // Always correct
std::ranges::copy(vec1, vec2);  // Type-safe
```

---

## Level 9: Custom Range Adaptors

You can create your own custom range adaptors!

### Example: Stride View (Take every Nth element)

```cpp
template<std::ranges::view V>
class stride_view : public std::ranges::view_interface<stride_view<V>>
{
private:
    V base_;
    std::size_t step_;
    
public:
    stride_view(V base, std::size_t step) 
        : base_(std::move(base)), step_(step) 
    {
    }
    
    auto begin()
    {
        return iterator(std::ranges::begin(base_), 
                       std::ranges::end(base_), 
                       step_);
    }
    
    auto end()
    {
        return std::ranges::end(base_);
    }
    
private:
    class iterator
    {
    private:
        using base_iter = std::ranges::iterator_t<V>;
        base_iter current_;
        base_iter end_;
        std::size_t step_;
        
    public:
        using value_type = std::ranges::range_value_t<V>;
        using difference_type = std::ptrdiff_t;
        
        iterator(base_iter current, base_iter end, std::size_t step)
            : current_(current), end_(end), step_(step) 
        {
        }
        
        auto operator*() const 
        { 
            return *current_; 
        }
        
        iterator& operator++()
        {
            for(std::size_t i = 0; i < step_ && current_ != end_; ++i)
            {
                ++current_;
            }
            return *this;
        }
        
        iterator operator++(int)
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }
        
        bool operator==(const iterator& other) const
        {
            return current_ == other.current_;
        }
        
        bool operator==(const base_iter& other) const
        {
            return current_ == other;
        }
    };
};

// Usage
std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

stride_view strided(data, 2);  // Every 2nd element

// Output: 1 3 5 7 9
```

---

## Level 10: Advanced Patterns

### Pattern 1: Cartesian Product

```cpp
std::vector<int> xs = {1, 2, 3};
std::vector<char> ys = {'A', 'B'};

// Generate all pairs
for(int x : xs)
{
    for(char y : ys)
    {
        std::cout << "(" << x << "," << y << ") ";
    }
}

// Output: (1,A) (1,B) (2,A) (2,B) (3,A) (3,B)
```

### Pattern 2: Zip (Pair elements)

```cpp
std::vector<std::string> names = {"Alice", "Bob", "Carol"};
std::vector<int> ages = {25, 30, 28};

// Zip names with ages
auto zipped = std::views::iota(0, std::min(names.size(), ages.size()))
    | std::views::transform([&](size_t i) 
    {
        return names[i] + ": " + std::to_string(ages[i]);
    });

// Output:
// Alice: 25
// Bob: 30
// Carol: 28
```

### Pattern 3: Sliding Window

```cpp
std::vector<int> nums = {1, 2, 3, 4, 5};

// Create pairs of adjacent elements
auto windows = std::views::iota(0u, nums.size() - 1)
    | std::views::transform([&](size_t i) 
    {
        return std::pair{nums[i], nums[i + 1]};
    });

// Output: [1,2] [2,3] [3,4] [4,5]
```

### Pattern 4: Group By (Manual Implementation)

```cpp
struct Person
{
    std::string name;
    int age;
    std::string city;
};

std::vector<Person> people = 
{
    {"Alice", 25, "NYC"},
    {"Bob", 30, "LA"},
    {"Carol", 25, "NYC"},
    {"David", 30, "NYC"}
};

// Group by age
std::map<int, std::vector<Person>> groups;
for(const auto& person : people)
{
    groups[person.age].push_back(person);
}

// Or using ranges:
auto age_25 = people 
    | std::views::filter([](const Person& p) 
    { 
        return p.age == 25; 
    });
```

---

## Key Takeaways

### 🎯 When to Use Ranges

**Use Ranges When:**
- You need to chain multiple transformations
- You want lazy evaluation for efficiency
- You're processing large datasets
- Readability and maintainability matter
- You want to avoid temporary containers

**Use Traditional Loops When:**
- You need complex control flow (multiple breaks/continues)
- You need to modify multiple variables simultaneously
- The logic is inherently imperative
- Performance is critical and you need fine-grained control

### 📊 Performance Characteristics

| Feature | Traditional | Ranges |
|---------|-------------|--------|
| Memory | Creates temporaries | Lazy (no temporaries) |
| Evaluation | Eager | Lazy |
| Composability | Poor | Excellent |
| Readability | Moderate | Excellent |
| Type Safety | Manual | Automatic |
| Early Exit | Manual | Automatic |

### ✨ Most Useful Views

**Filtering & Selection:**
- `filter` - Select elements
- `take` / `drop` - First/skip N elements
- `take_while` / `drop_while` - Conditional selection

**Transformation:**
- `transform` - Map function over elements
- `join` - Flatten nested ranges
- `split` - Split by delimiter

**Generation:**
- `iota` - Generate sequences
- `repeat` - Repeat value
- `single` / `empty` - Edge cases

**Algorithms:**
- `std::ranges::sort`
- `std::ranges::find_if`
- `std::ranges::count_if`
- `std::ranges::copy_if`

### 🚀 Best Practices

1. **Use pipeline style for readability**
   ```cpp
   auto result = data
       | view1
       | view2
       | view3;
   ```

2. **Prefer `auto` for range types**
   ```cpp
   auto filtered = data | std::views::filter(...);
   ```

3. **Remember views are lazy - materialize when needed**
   ```cpp
   auto view = data | std::views::filter(...);
   std::vector<int> materialized(view.begin(), view.end());
   ```

4. **Combine with range algorithms**
   ```cpp
   std::ranges::sort(data | std::views::reverse);
   ```

5. **Use `std::ranges::to` (C++23) for easy materialization**
   ```cpp
   // C++23
   auto vec = data 
       | std::views::filter(...)
       | std::ranges::to<std::vector>();
   ```

### 🎓 Learning Path

1. **Start Simple**: Use basic views (`filter`, `transform`, `take`)
2. **Compose**: Chain multiple views together
3. **Explore**: Try advanced views (`split`, `join`, `take_while`)
4. **Generate**: Use range generators (`iota`, `repeat`)
5. **Algorithms**: Use range-based algorithms
6. **Custom**: Create your own adaptors
7. **Patterns**: Master advanced patterns (zip, window, etc.)

---

## Resources

- **Official Documentation**: [cppreference - Ranges](https://en.cppreference.com/w/cpp/ranges)
- **Compiler Support**: GCC 10+, Clang 13+, MSVC 19.30+
- **Include**: `#include <ranges>` and `#include <algorithm>`
- **Namespace**: `std::ranges` and `std::views`

---

## Conclusion

C++20 Ranges represent a paradigm shift in C++ programming. They bring:

- 🎨 **Elegant syntax** that reads like natural language
- ⚡ **Performance** through lazy evaluation
- 🛡️ **Safety** through type checking
- 🔧 **Composability** for complex transformations

Start using ranges in your code today and experience the future of C++!

---

**Happy Coding! 🚀**
