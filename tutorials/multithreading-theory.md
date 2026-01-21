# C++ Multithreading: Professional Guide

## Table of Contents
- [Introduction](#introduction)
- [Thread Basics](#thread-basics)
- [Thread Management](#thread-management)
- [Synchronization Primitives](#synchronization-primitives)
- [Atomics](#atomics)
- [Modern Features](#modern-features)
- [Best Practices](#best-practices)
- [Common Patterns](#common-patterns)

---

## Introduction

Multithreading allows your program to perform multiple operations simultaneously. Think of it as having multiple workers in a factory - each can work independently but they share the same workspace and resources.

### Why Use Threads?
- **Performance**: Utilize multiple CPU cores
- **Responsiveness**: Keep UI responsive while doing background work
- **Efficiency**: Handle multiple I/O operations concurrently

### C++ Threading Evolution
- **C++11**: Standard threading library introduced
- **C++17**: Improved locking and parallel algorithms
- **C++20**: `jthread`, semaphores, barriers
- **C++23**: Enhanced atomics

---

## Thread Basics

### Creating Threads
Threads are created using `std::thread` and start execution immediately.

```cpp
#include <thread>
#include <iostream>

void worker()
{
    std::cout << "Working in thread\n";
}

int main()
{
    std::thread t(worker);  // Thread starts immediately
    t.join();               // Wait for completion
    return 0;
}
```

### Passing Parameters
```cpp
void greet(std::string name, int count)
{
    for (int i = 0; i < count; ++i)
    {
        std::cout << "Hello " << name << "\n";
    }
}

int main()
{
    std::thread t(greet, "Alice", 3);
    t.join();
    return 0;
}
```

### Lambda Threads
```cpp
int main()
{
    int value = 42;
    std::thread t([value]()
    {
        std::cout << "Value: " << value << "\n";
    });
    t.join();
    return 0;
}
```

---

## Thread Management

### Join vs Detach

**Join**: Wait for thread completion (blocking)
```cpp
std::thread t(worker);
t.join();  // Wait here until thread finishes
```

**Detach**: Let thread run independently
```cpp
std::thread t(backgroundTask);
t.detach();  // Thread runs independently
// Main thread continues immediately
```

⚠️ **Warning**: Detached threads cannot be controlled or joined later.

### Thread IDs and Hardware
```cpp
std::cout << "Thread ID: " << std::this_thread::get_id() << "\n";
std::cout << "CPU cores: " << std::thread::hardware_concurrency() << "\n";
```

### Exception Safety
Always ensure threads are properly cleaned up, even when exceptions occur.

```cpp
class ThreadGuard
{
    std::thread& t;
public:
    explicit ThreadGuard(std::thread& thread) : t(thread)
    {
    }
    
    ~ThreadGuard()
    {
        if (t.joinable())
        {
            t.join();
        }
    }
};

std::thread t(riskyTask);
ThreadGuard guard(t);  // Auto-joins even if exception occurs
```

---

## Synchronization Primitives

### The Data Race Problem
When multiple threads access shared data without coordination, you get unpredictable results.

```cpp
int counter = 0;  // Shared data

void unsafe_increment()
{
    ++counter;  // Data race!
}
```

### Mutex Basics
A mutex ensures only one thread can access protected code at a time.

```cpp
#include <mutex>

std::mutex mtx;
int safe_counter = 0;

void safe_increment()
{
    std::lock_guard<std::mutex> lock(mtx);
    ++safe_counter;  // Thread-safe
}
```

### Mutex Types

**Basic Mutex**: Standard mutual exclusion
```cpp
std::mutex mtx;
```

**Recursive Mutex**: Same thread can lock multiple times
```cpp
std::recursive_mutex rmtx;
```

**Shared Mutex**: Multiple readers or one writer (C++17)
```cpp
std::shared_mutex rw_mutex;

// Readers
std::shared_lock lock(rw_mutex);
// Multiple readers can enter

// Writer
std::unique_lock lock(rw_mutex);
// Only one writer
```

### Advanced Locking

**`std::unique_lock`**: Flexible locking
```cpp
std::unique_lock<std::mutex> lock(mtx);
lock.unlock();  // Manual control
// ... non-critical work
lock.lock();    // Re-lock
```

**`std::scoped_lock`**: Multiple mutexes (C++17)
```cpp
std::mutex mtx1, mtx2;
std::scoped_lock lock(mtx1, mtx2);  // Deadlock-free
```

### Condition Variables
Allow threads to wait for specific conditions.

```cpp
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool data_ready = false;

void consumer()
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, []{ return data_ready; });
    // Proceed when data_ready becomes true
}

void producer()
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        data_ready = true;
    }
    cv.notify_one();  // Wake one waiting thread
}
```

---

## Atomics

### Lock-Free Operations
Atomics provide thread-safe access without mutex overhead.

```cpp
#include <atomic>

std::atomic<int> atomic_counter(0);

void increment()
{
    atomic_counter.fetch_add(1);  // Thread-safe and fast
}
```

### Atomic Operations
- `load()`: Read value
- `store()`: Write value
- `fetch_add()`, `fetch_sub()`: Atomic arithmetic
- `exchange()`: Swap values
- `compare_exchange_strong()`: Conditional update

### Memory Orders
Control how atomic operations synchronize between threads.

```cpp
std::atomic<bool> ready(false);

void producer()
{
    // ... prepare data
    ready.store(true, std::memory_order_release);
}

void consumer()
{
    while (!ready.load(std::memory_order_acquire))
    {
        // Wait
    }
    // ... use prepared data
}
```

**Common memory orders:**
- `relaxed`: No synchronization, just atomicity
- `acquire`: Load with synchronization
- `release`: Store with synchronization
- `seq_cst`: Full synchronization (default)

### Atomic Flag
Lightweight atomic for simple flags.

```cpp
std::atomic_flag flag = ATOMIC_FLAG_INIT;

// Test and set atomically
while (flag.test_and_set(std::memory_order_acquire))
{
    // Spin until lock acquired
}
flag.clear(std::memory_order_release);
```

---

## Modern Features

### jthread (C++20)
Self-joining thread with cancellation support.

```cpp
#include <stop_token>

void worker(std::stop_token stoken)
{
    while (!stoken.stop_requested())
    {
        // Do work...
    }
}

int main()
{
    std::jthread t(worker);
    
    // Later...
    t.request_stop();  // Cooperative cancellation
    // Auto-joins on destruction
}
```

### Semaphores (C++20)
Control access to limited resources.

**Counting Semaphore**:
```cpp
#include <semaphore>
std::counting_semaphore<5> sem(5);  // 5 available slots

void access_resource()
{
    sem.acquire();   // Wait for slot
    // Use resource...
    sem.release();   // Release slot
}
```

**Binary Semaphore**:
```cpp
std::binary_semaphore sem(0);  // Initially unavailable

void waiter()
{
    sem.acquire();  // Wait for signal
}

void signaler()
{
    sem.release();  // Send signal
}
```

### Latches and Barriers (C++20)

**Latch**: One-time synchronization point
```cpp
#include <latch>
std::latch completion_latch(3);  // Wait for 3 threads

void worker()
{
    // Do work...
    completion_latch.count_down();  // Decrement counter
}

// Main thread
completion_latch.wait();  // Wait until counter reaches 0
```

**Barrier**: Reusable synchronization
```cpp
#include <barrier>
std::barrier sync_point(3);  // Sync 3 threads

void phased_worker()
{
    for (int phase = 0; phase < 3; ++phase)
    {
        // Phase work...
        sync_point.arrive_and_wait();  // Sync between phases
    }
}
```

---

## Best Practices

### 1. Prefer RAII for Lock Management
```cpp
// Good
{
    std::lock_guard<std::mutex> lock(mtx);
    // Critical section
} // Auto-unlock

// Bad
mtx.lock();
// Critical section - unsafe if exception
mtx.unlock();
```

### 2. Avoid Deadlocks
- Always acquire locks in consistent order
- Use `std::scoped_lock` for multiple mutexes
- Avoid calling unknown code while holding locks

### 3. Minimize Lock Scope
```cpp
// Good - minimal lock time
auto result = expensive_computation();  // Outside lock
{
    std::lock_guard<std::mutex> lock(mtx);
    shared_data = result;  // Quick update
}

// Bad - long critical section
{
    std::lock_guard<std::mutex> lock(mtx);
    shared_data = expensive_computation();  // Holding lock too long
}
```

### 4. Choose Right Synchronization
- **Atomics**: Simple counters, flags
- **Mutex**: Complex data structures
- **Shared Mutex**: Read-heavy workloads
- **Condition Variable**: Event waiting
- **Semaphore**: Resource counting

### 5. Use jthread When Possible (C++20+)
```cpp
// Good - auto-joining
std::jthread t(worker);

// Requires manual management
std::thread t(worker);
t.join();  // Must remember
```

### 6. Handle Spurious Wakeups
```cpp
// Correct - use predicate
cv.wait(lock, []{ return condition; });

// Incorrect - vulnerable to spurious wakeups
cv.wait(lock);
if (!condition)
{
    /* Problem! */
}
```

### 7. Appropriate Memory Order
```cpp
// Simple counter - relaxed is sufficient
counter.fetch_add(1, std::memory_order_relaxed);

// Synchronization needed - use acquire/release
ready.store(true, std::memory_order_release);
```

---

## Common Patterns

### Thread Pool
Reuse threads instead of creating/destroying frequently.

```cpp
class ThreadPool
{
    std::vector<std::jthread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable cv;
    bool stop = false;

public:
    ThreadPool(size_t threads)
    {
        for (size_t i = 0; i < threads; ++i)
        {
            workers.emplace_back([this]
            {
                while (true)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(queue_mutex);
                        cv.wait(lock, [this]
                        {
                            return stop || !tasks.empty();
                        });
                        if (stop && tasks.empty())
                        {
                            return;
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template<typename F>
    void enqueue(F&& f)
    {
        {
            std::lock_guard lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        cv.notify_one();
    }

    ~ThreadPool()
    {
        {
            std::lock_guard lock(queue_mutex);
            stop = true;
        }
        cv.notify_all();
    }
};
```

### Producer-Consumer
Coordinate data production and consumption.

```cpp
template<typename T>
class BoundedBuffer
{
    std::queue<T> buffer;
    size_t max_size;
    std::mutex mtx;
    std::condition_variable not_full;
    std::condition_variable not_empty;

public:
    BoundedBuffer(size_t size) : max_size(size)
    {
    }

    void produce(T item)
    {
        std::unique_lock lock(mtx);
        not_full.wait(lock, [this] { return buffer.size() < max_size; });
        buffer.push(item);
        not_empty.notify_one();
    }

    T consume()
    {
        std::unique_lock lock(mtx);
        not_empty.wait(lock, [this] { return !buffer.empty(); });
        T item = buffer.front();
        buffer.pop();
        not_full.notify_one();
        return item;
    }
};
```

### Reader-Writer Lock
Multiple readers or single writer.

```cpp
class ThreadSafeData
{
    mutable std::shared_mutex mtx;
    std::string data;

public:
    std::string read() const
    {
        std::shared_lock lock(mtx);  // Multiple readers can enter
        return data;
    }

    void write(const std::string& new_data)
    {
        std::unique_lock lock(mtx);  // Exclusive access for writers
        data = new_data;
    }
};
```

### Future-Promise
Asynchronous result handling.

```cpp
#include <future>

int compute_async(int input)
{
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();

    std::jthread worker([prom = std::move(prom), input]() mutable
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        prom.set_value(input * input);  // Set result
    });

    return fut.get();  // Wait for result
}
```

### Lock-Free Stack
Atomic-based data structure.

```cpp
template<typename T>
class LockFreeStack
{
    struct Node
    {
        T data;
        Node* next;
    };
    std::atomic<Node*> head = nullptr;

public:
    void push(const T& data)
    {
        Node* new_node = new Node{data, head.load()};
        while (!head.compare_exchange_weak(new_node->next, new_node))
        {
            // Retry until successful
        }
    }

    bool pop(T& result)
    {
        Node* old_head = head.load();
        while (old_head && !head.compare_exchange_weak(old_head, old_head->next))
        {
            // Retry until successful
        }
        if (!old_head)
        {
            return false;
        }
        result = old_head->data;
        delete old_head;
        return true;
    }
};
```

---

## Performance Considerations

### Thread Creation Overhead
Creating threads is expensive. Use thread pools for short tasks.

### False Sharing
When unrelated variables share cache lines, causing unnecessary synchronization.

```cpp
// Bad - potential false sharing
struct Counters
{
    std::atomic<int> a;  // Might share cache line
    std::atomic<int> b;  // with other counter
};

// Good - cache line padding
struct alignas(64) Counters  // Typical cache line size
{
    std::atomic<int> a;
    // Padding to separate cache lines
    char padding[60];
    std::atomic<int> b;
};
```

### Contention Reduction
- Use reader-writer locks for read-heavy data
- Partition data to reduce hot spots
- Consider lock-free algorithms for high contention

---

## Debugging Tips

### Common Issues
1. **Data Races**: Unsynchronized access to shared data
2. **Deadlocks**: Circular lock dependencies
3. **Livelocks**: Threads busy but no progress
4. **Starvation**: Some threads never get CPU time

### Tools
- **Thread Sanitizer**: `-fsanitize=thread`
- **Deadlock detectors**: Available in debuggers
- **Profilers**: Identify contention points

### Defensive Programming
- Use timeouts for waits
- Implement health checks
- Validate invariants with assertions
- Test with different thread counts

---

## Quick Reference

### Synchronization Choices
| Use Case | Recommended Tool |
|----------|------------------|
| Simple counter | `std::atomic` |
| Complex data structure | `std::mutex` |
| Read-heavy data | `std::shared_mutex` |
| Event waiting | `std::condition_variable` |
| Resource counting | `std::counting_semaphore` |
| One-time sync | `std::latch` |
| Reusable sync | `std::barrier` |

### Memory Order Guide
- **`relaxed`**: No ordering constraints
- **`acquire`**: Load with synchronization
- **`release`**: Store with synchronization  
- **`acq_rel`**: Read-modify-write operations
- **`seq_cst`**: Strongest ordering (default)

### Key Points
1. **Start Simple**: Use mutexes before lock-free
2. **Measure**: Profile before optimizing
3. **Test Thoroughly**: Threading bugs are subtle
4. **Use RAII**: Automatic resource management
5. **Avoid Premature Optimization**: Correctness first

---

This guide covers essential C++ multithreading concepts with professional-grade examples. Master these fundamentals before exploring advanced patterns and optimizations.
