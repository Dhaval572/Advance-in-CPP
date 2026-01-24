# C++ Multithreading: Complete Guide (C++17 - C++23)

A comprehensive guide to modern C++ multithreading, covering all features from C++17 through C++23 with practical examples and best practices.

## Table of Contents

- [Introduction](#introduction)
- [Thread Basics](#thread-basics)
- [Thread Management](#thread-management)
- [Synchronization Primitives](#synchronization-primitives)
- [Atomics](#atomics)
- [Async, Future, and Promise](#async-future-and-promise)
- [Thread vs Async Comparison](#thread-vs-async-comparison)
- [Modern Features (C++20/23)](#modern-features-c2023)
---

## Introduction

Modern C++ provides robust support for multithreading through the `<thread>`, `<mutex>`, `<condition_variable>`, `<atomic>`, `<semaphore>`, `<barrier>`, `<future>`, and `<promise>` headers. This guide covers everything from basic thread creation to advanced synchronization techniques and asynchronous programming.

### Requirements

- **C++11**: Basic threading, mutexes, condition variables, futures, promises
- **C++17**: `std::scoped_lock`, `std::shared_mutex`
- **C++20**: `jthread`, semaphores, barriers, latches
- **C++23**: Enhanced atomics and improvements

---

## Thread Basics

### Creating and Starting Threads

Threads are created using `std::thread` and automatically begin execution.

```cpp
#include <iostream>
#include <thread>

void simpleTask()
{
    std::cout << "Hello from thread!\n";
}

int main()
{
    std::thread t(simpleTask);
    t.join(); // Wait for thread to complete
    return 0;
}
```

### Thread with Parameters

```cpp
#include <iostream>
#include <thread>
#include <string>

void greet(const std::string& name, int count)
{
    for (int i = 0; i < count; ++i)
    {
        std::cout << "Hello, " << name << "!\n";
    }
}

int main()
{
    std::thread t(greet, "Alice", 3);
    t.join();
    return 0;
}
```

### Lambda Functions with Threads

```cpp
#include <iostream>
#include <thread>

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

### Member Function Threads

```cpp
#include <iostream>
#include <thread>

class Worker
{
public:
    void doWork(int taskId)
    {
        std::cout << "Task " << taskId << " completed\n";
    }
};

int main()
{
    Worker worker;
    std::thread t(&Worker::doWork, &worker, 1);
    t.join();
    return 0;
}
```

---

## Thread Management

### Join vs Detach

**Join**: Wait for thread to complete (blocking).

```cpp
#include <iostream>
#include <thread>
#include <chrono>

void task()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Task completed\n";
}

int main()
{
    std::thread t(task);
    std::cout << "Waiting for task...\n";
    t.join(); // Blocks until task() completes
    std::cout << "Thread joined\n";
    return 0;
}
```

**Detach**: Thread runs independently (non-blocking).

```cpp
#include <iostream>
#include <thread>
#include <chrono>

void backgroundTask()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Background task done\n";
}

int main()
{
    std::thread t(backgroundTask);
    t.detach(); // Thread runs independently
    
    std::cout << "Main thread continues\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}
```

### Thread IDs and Hardware Concurrency

```cpp
#include <iostream>
#include <thread>
#include <vector>

void printThreadId()
{
    std::cout << "Thread ID: " << std::this_thread::get_id() << "\n";
}

int main()
{
    std::cout << "Hardware concurrency: " 
              << std::thread::hardware_concurrency() << " threads\n";
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back(printThreadId);
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    return 0;
}
```

### Exception Safety with Threads

```cpp
#include <iostream>
#include <thread>

class ThreadGuard
{
    std::thread& t;
public:
    explicit ThreadGuard(std::thread& thread) : t(thread) {}
    
    ~ThreadGuard()
    {
        if (t.joinable())
        {
            t.join();
        }
    }
    
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
};

void riskyOperation()
{
    // Simulated work
    std::cout << "Working...\n";
}

int main()
{
    std::thread t(riskyOperation);
    ThreadGuard guard(t);
    
    // Even if exception occurs, thread will be joined
    // through guard's destructor
    
    return 0;
}
```

---

## Synchronization Primitives

### Mutexes

Mutexes provide mutual exclusion to protect shared data.

#### Basic Mutex

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;
int sharedCounter = 0;

void increment(int iterations)
{
    for (int i = 0; i < iterations; ++i)
    {
        mtx.lock();
        ++sharedCounter;
        mtx.unlock();
    }
}

int main()
{
    std::thread t1(increment, 1000);
    std::thread t2(increment, 1000);
    
    t1.join();
    t2.join();
    
    std::cout << "Final counter: " << sharedCounter << "\n";
    return 0;
}
```

#### Lock Guards (RAII)

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;
std::vector<int> data;

void addData(int value)
{
    std::lock_guard<std::mutex> lock(mtx);
    data.push_back(value);
    // Mutex automatically unlocked when lock goes out of scope
}

int main()
{
    std::thread t1(addData, 1);
    std::thread t2(addData, 2);
    std::thread t3(addData, 3);
    
    t1.join();
    t2.join();
    t3.join();
    
    for (int val : data)
    {
        std::cout << val << " ";
    }
    std::cout << "\n";
    
    return 0;
}
```

#### Unique Lock (Flexible Locking)

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

void flexibleLocking()
{
    std::unique_lock<std::mutex> lock(mtx);
    
    // Do some work
    std::cout << "Locked section\n";
    
    lock.unlock();
    // Do work that doesn't need protection
    std::cout << "Unlocked section\n";
    
    lock.lock();
    // More protected work
    std::cout << "Locked again\n";
}

int main()
{
    std::thread t1(flexibleLocking);
    std::thread t2(flexibleLocking);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

#### Scoped Lock (C++17) - Multiple Mutexes

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx1, mtx2;
int account1 = 1000;
int account2 = 2000;

void transfer(int amount)
{
    std::scoped_lock lock(mtx1, mtx2); // Deadlock-free
    
    account1 -= amount;
    account2 += amount;
    
    std::cout << "Transferred " << amount << "\n";
}

int main()
{
    std::thread t1(transfer, 100);
    std::thread t2(transfer, 200);
    
    t1.join();
    t2.join();
    
    std::cout << "Account1: " << account1 << ", Account2: " << account2 << "\n";
    
    return 0;
}
```

#### Recursive Mutex

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::recursive_mutex rmtx;

void recursiveFunction(int depth)
{
    std::lock_guard<std::recursive_mutex> lock(rmtx);
    
    std::cout << "Depth: " << depth << "\n";
    
    if (depth > 0)
    {
        recursiveFunction(depth - 1);
    }
}

int main()
{
    std::thread t(recursiveFunction, 3);
    t.join();
    return 0;
}
```

#### Shared Mutex (C++17) - Reader-Writer Lock

```cpp
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <vector>

std::shared_mutex rwMutex;
int sharedData = 0;

void reader(int id)
{
    std::shared_lock<std::shared_mutex> lock(rwMutex);
    std::cout << "Reader " << id << " reads: " << sharedData << "\n";
}

void writer(int id, int value)
{
    std::unique_lock<std::shared_mutex> lock(rwMutex);
    sharedData = value;
    std::cout << "Writer " << id << " writes: " << value << "\n";
}

int main()
{
    std::vector<std::thread> threads;
    
    // Multiple readers can access simultaneously
    threads.emplace_back(reader, 1);
    threads.emplace_back(reader, 2);
    threads.emplace_back(writer, 1, 42);
    threads.emplace_back(reader, 3);
    threads.emplace_back(writer, 2, 100);
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    return 0;
}
```

### Condition Variables

Condition variables enable threads to wait for specific conditions.

#### Basic Producer-Consumer

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex mtx;
std::condition_variable cv;
std::queue<int> dataQueue;
bool finished = false;

void producer()
{
    for (int i = 0; i < 5; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::lock_guard<std::mutex> lock(mtx);
        dataQueue.push(i);
        std::cout << "Produced: " << i << "\n";
        cv.notify_one();
    }
    
    std::lock_guard<std::mutex> lock(mtx);
    finished = true;
    cv.notify_all();
}

void consumer(int id)
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []{ return !dataQueue.empty() || finished; });
        
        if (dataQueue.empty() && finished)
        {
            break;
        }
        
        int value = dataQueue.front();
        dataQueue.pop();
        lock.unlock();
        
        std::cout << "Consumer " << id << " consumed: " << value << "\n";
    }
}

int main()
{
    std::thread prod(producer);
    std::thread cons1(consumer, 1);
    std::thread cons2(consumer, 2);
    
    prod.join();
    cons1.join();
    cons2.join();
    
    return 0;
}
```

#### Condition Variable with Timeout

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void waitWithTimeout()
{
    std::unique_lock<std::mutex> lock(mtx);
    
    if (cv.wait_for(lock, std::chrono::seconds(2), []{ return ready; }))
    {
        std::cout << "Condition met!\n";
    }
    else
    {
        std::cout << "Timeout occurred\n";
    }
}

int main()
{
    std::thread t(waitWithTimeout);
    
    // Uncomment to signal before timeout
    // std::this_thread::sleep_for(std::chrono::seconds(1));
    // {
    //     std::lock_guard<std::mutex> lock(mtx);
    //     ready = true;
    //     cv.notify_one();
    // }
    
    t.join();
    return 0;
}
```

---

## Atomics

Atomic operations provide lock-free thread-safe access to shared variables.

### Basic Atomic Operations

```cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

std::atomic<int> atomicCounter(0);

void incrementAtomic(int iterations)
{
    for (int i = 0; i < iterations; ++i)
    {
        atomicCounter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main()
{
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back(incrementAtomic, 1000);
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    std::cout << "Final atomic counter: " << atomicCounter.load() << "\n";
    
    return 0;
}
```

### Atomic Flag (Simple Spinlock)

```cpp
#include <iostream>
#include <thread>
#include <atomic>

std::atomic_flag lock = ATOMIC_FLAG_INIT;

void criticalSection(int id)
{
    while (lock.test_and_set(std::memory_order_acquire))
    {
        // Spin until lock is acquired
    }
    
    std::cout << "Thread " << id << " in critical section\n";
    
    lock.clear(std::memory_order_release);
}

int main()
{
    std::thread t1(criticalSection, 1);
    std::thread t2(criticalSection, 2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

### Compare-Exchange (CAS)

```cpp
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<int> value(0);

void compareAndSwap()
{
    int expected = 0;
    int desired = 42;
    
    if (value.compare_exchange_strong(expected, desired))
    {
        std::cout << "Swapped: 0 -> 42\n";
    }
    else
    {
        std::cout << "Swap failed, current value: " << value << "\n";
    }
}

int main()
{
    std::thread t1(compareAndSwap);
    std::thread t2(compareAndSwap);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

### Memory Orders

```cpp
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<int> data(0);
std::atomic<bool> ready(false);

void producer()
{
    data.store(42, std::memory_order_relaxed);
    ready.store(true, std::memory_order_release); // Synchronizes with acquire
}

void consumer()
{
    while (!ready.load(std::memory_order_acquire)) // Waits for release
    {
        // Spin
    }
    
    std::cout << "Data: " << data.load(std::memory_order_relaxed) << "\n";
}

int main()
{
    std::thread t1(producer);
    std::thread t2(consumer);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

### Atomic Shared Pointer (C++20)

```cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <memory>

std::atomic<std::shared_ptr<int>> atomicPtr;

void updatePointer(int value)
{
    auto newPtr = std::make_shared<int>(value);
    atomicPtr.store(newPtr);
    std::cout << "Updated to: " << value << "\n";
}

void readPointer(int id)
{
    auto ptr = atomicPtr.load();
    if (ptr)
    {
        std::cout << "Thread " << id << " read: " << *ptr << "\n";
    }
}

int main()
{
    atomicPtr.store(std::make_shared<int>(0));
    
    std::thread t1(updatePointer, 42);
    std::thread t2(readPointer, 1);
    std::thread t3(readPointer, 2);
    
    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}
```

---

## Async, Future, and Promise

This section covers asynchronous programming with futures, promises, and async functions in C++.

### Understanding Async and Future

`std::async` launches a task asynchronously and returns a `std::future` that holds the result. This provides a higher-level abstraction than manually creating threads.

#### Basic Async with std::future

```cpp
#include <iostream>
#include <future>
#include <chrono>

int computeValue(int x, int y)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return x + y;
}

int main()
{
    std::cout << "Starting computation...\n";
    
    // Launch async task
    std::future<int> result = std::async(std::launch::async, computeValue, 10, 20);
    
    std::cout << "Main thread continues while computation happens...\n";
    
    // Get result (blocks if not ready)
    int value = result.get();
    
    std::cout << "Result: " << value << "\n";
    
    return 0;
}
```

#### Launch Policies

`std::async` supports two launch policies:

**std::launch::async** - Runs asynchronously in a separate thread.

```cpp
#include <iostream>
#include <future>

int task()
{
    std::cout << "Task running in separate thread\n";
    return 42;
}

int main()
{
    std::future<int> fut = std::async(std::launch::async, task);
    std::cout << "Main thread continues\n";
    std::cout << "Result: " << fut.get() << "\n";
    return 0;
}
```

**std::launch::deferred** - Runs lazily when result is requested.

```cpp
#include <iostream>
#include <future>

int task()
{
    std::cout << "Task running (deferred)\n";
    return 42;
}

int main()
{
    std::future<int> fut = std::async(std::launch::deferred, task);
    
    std::cout << "Task hasn't run yet\n";
    std::cout << "Result: " << fut.get() << "\n"; // Task runs here
    
    return 0;
}
```

#### Using std::future::wait_for

```cpp
#include <iostream>
#include <future>
#include <chrono>

int slowTask()
{
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 100;
}

int main()
{
    std::future<int> result = std::async(std::launch::async, slowTask);
    
    std::future_status status = result.wait_for(std::chrono::seconds(1));
    
    if (status == std::future_status::ready)
    {
        std::cout << "Result ready: " << result.get() << "\n";
    }
    else if (status == std::future_status::timeout)
    {
        std::cout << "Result not ready, still waiting...\n";
        std::cout << "Final result: " << result.get() << "\n"; // Blocks until ready
    }
    else if (status == std::future_status::deferred)
    {
        std::cout << "Task is deferred\n";
    }
    
    return 0;
}
```

### Understanding Promise and Future

A `std::promise` is used to set a value that a `std::future` will retrieve. This provides a way to pass results between threads explicitly.

#### Basic Promise and Future

```cpp
#include <iostream>
#include <future>
#include <thread>

void computeInThread(std::promise<int> prom)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    int result = 42;
    prom.set_value(result); // Set the result
}

int main()
{
    std::promise<int> promise;
    std::future<int> future = promise.get_future();
    
    // Create thread and pass promise
    std::thread t(computeInThread, std::move(promise));
    
    std::cout << "Waiting for result...\n";
    int value = future.get(); // Blocks until result is available
    std::cout << "Got result: " << value << "\n";
    
    t.join();
    
    return 0;
}
```

#### Promise with Exception Handling

```cpp
#include <iostream>
#include <future>
#include <thread>

void riskyComputation(std::promise<int> prom)
{
    try
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        throw std::runtime_error("Computation failed!");
    }
    catch (const std::exception& e)
    {
        prom.set_exception(std::current_exception()); // Set exception
    }
}

int main()
{
    std::promise<int> promise;
    std::future<int> future = promise.get_future();
    
    std::thread t(riskyComputation, std::move(promise));
    
    try
    {
        int value = future.get(); // Will throw the exception
    }
    catch (const std::exception& e)
    {
        std::cout << "Caught exception: " << e.what() << "\n";
    }
    
    t.join();
    
    return 0;
}
```

#### Multiple Futures from One Promise (Packaged Task)

When you need multiple threads to wait for the same result, use `std::packaged_task`:

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <vector>

int compute()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 42;
}

int main()
{
    std::packaged_task<int()> task(compute);
    std::future<int> fut = task.get_future();
    
    // Run task in separate thread
    std::thread t(std::move(task));
    
    std::cout << "Result: " << fut.get() << "\n";
    
    t.join();
    
    return 0;
}
```

#### Using packaged_task with Thread Pool

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

class SimpleThreadPool
{
public:
    SimpleThreadPool(size_t numThreads)
    {
        for (size_t i = 0; i < numThreads; ++i)
        {
            workers.emplace_back([this]
            {
                while (true)
                {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [this]{ return !tasks.empty() || stop; });
                        
                        if (stop && tasks.empty())
                            return;
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    
                    task();
                }
            });
        }
    }
    
    template<typename F>
    auto enqueue(F&& f) -> std::future<typename std::result_of<F()>::type>
    {
        using return_type = typename std::result_of<F()>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::forward<F>(f)
        );
        
        std::future<return_type> res = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(mtx);
            tasks.emplace([task](){ (*task)(); });
        }
        
        cv.notify_one();
        return res;
    }
    
    ~SimpleThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        
        for (auto& t : workers)
            t.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop = false;
};

int main()
{
    SimpleThreadPool pool(4);
    
    std::vector<std::future<int>> futures;
    
    for (int i = 0; i < 8; ++i)
    {
        futures.push_back(pool.enqueue([i]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * i));
            return i * i;
        }));
    }
    
    for (auto& f : futures)
    {
        std::cout << "Result: " << f.get() << "\n";
    }
    
    return 0;
}
```

#### Chaining Futures with then() Pattern (C++23 or using custom wrappers)

```cpp
#include <iostream>
#include <future>

int compute1()
{
    std::cout << "Computing value 1...\n";
    return 10;
}

int compute2(int prev)
{
    std::cout << "Computing value 2 using: " << prev << "\n";
    return prev * 2;
}

int main()
{
    // Traditional approach: chaining futures
    auto fut1 = std::async(std::launch::async, compute1);
    int val1 = fut1.get();
    
    auto fut2 = std::async(std::launch::async, [val1]{ return compute2(val1); });
    int val2 = fut2.get();
    
    std::cout << "Final result: " << val2 << "\n";
    
    return 0;
}
```

---

## Thread vs Async Comparison

### Key Differences

This section explains when to use `std::thread` versus `std::async` with `std::future`.

#### std::thread

**Pros:**
- Explicit control over thread lifetime
- Can use `join()` and `detach()` explicitly
- Works well for long-running background tasks
- No automatic return value management

**Cons:**
- Must manually join/detach
- Returning values requires additional synchronization
- More boilerplate code

**Best for:** Long-running tasks, background workers, real-time systems where explicit control matters.

#### std::async with std::future

**Pros:**
- Automatic return value handling
- Higher-level abstraction
- Launch policies provide flexibility (async vs deferred)
- Exception-safe return value passing
- Future can check task status with `wait_for()`

**Cons:**
- Less explicit control over threading
- Thread may be reused or deferred depending on implementation
- Less suitable for fine-grained thread management

**Best for:** One-off computations, scenarios where you need results, tasks with unknown duration, high-level concurrent operations.

### Comparison Example

#### Using std::thread

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

int result = 0;
std::mutex mtx;

void computation()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::lock_guard<std::mutex> lock(mtx);
    result = 42;
}

int main()
{
    std::thread t(computation);
    
    // Do other work
    std::cout << "Waiting for computation...\n";
    
    t.join(); // Must explicitly wait
    
    std::cout << "Result: " << result << "\n";
    
    return 0;
}
```

#### Using std::async with std::future

```cpp
#include <iostream>
#include <future>
#include <chrono>

int computation()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 42;
}

int main()
{
    std::future<int> fut = std::async(std::launch::async, computation);
    
    // Do other work
    std::cout << "Waiting for computation...\n";
    
    int result = fut.get(); // Automatic synchronization and result retrieval
    
    std::cout << "Result: " << result << "\n";
    
    return 0;
}
```

### Practical Decision Guide

**Use std::thread when:**
- Managing a thread pool
- Running background services
- Explicit thread lifecycle control is needed
- Working with detached threads

**Use std::async when:**
- Launching a one-off computation
- Need the return value easily
- Task duration is uncertain
- Want automatic exception propagation
- Building higher-level constructs

### Combined Usage: ThreadPool with Async

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

class AsyncThreadPool
{
public:
    AsyncThreadPool(size_t numThreads)
    {
        for (size_t i = 0; i < numThreads; ++i)
        {
            workers.emplace_back([this]
            {
                while (true)
                {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [this]{ return !tasks.empty() || stop; });
                        
                        if (stop && tasks.empty())
                            return;
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    
                    task();
                }
            });
        }
    }
    
    template<typename F>
    auto submit(F&& f) -> std::future<typename std::result_of<F()>::type>
    {
        using return_type = typename std::result_of<F()>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::forward<F>(f)
        );
        
        std::future<return_type> res = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(mtx);
            tasks.emplace([task](){ (*task)(); });
        }
        
        cv.notify_one();
        return res;
    }
    
    ~AsyncThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        
        for (auto& t : workers)
            t.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop = false;
};

int main()
{
    AsyncThreadPool pool(4);
    
    std::vector<std::future<int>> results;
    
    for (int i = 0; i < 10; ++i)
    {
        results.push_back(pool.submit([i]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return i * i;
        }));
    }
    
    for (auto& fut : results)
    {
        std::cout << "Result: " << fut.get() << "\n";
    }
    
    return 0;
}
```

---

## Modern Features (C++20/23)

### jthread (C++20) - Self-Joining Thread

`jthread` automatically joins on destruction and supports cooperative cancellation.

```cpp
#include <iostream>
#include <thread>
#include <chrono>

void autoJoinTask(std::stop_token stoken)
{
    for (int i = 0; i < 10; ++i)
    {
        if (stoken.stop_requested())
        {
            std::cout << "Stop requested, exiting...\n";
            return;
        }
        
        std::cout << "Working... " << i << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main()
{
    {
        std::jthread thread(autoJoinTask);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        thread.request_stop(); // Request cooperative cancellation
        
    } // Automatically joins here
    
    std::cout << "Thread finished\n";
    
    return 0;
}
```

### Counting Semaphore (C++20)

Semaphores control access to a limited number of resources.

```cpp
#include <iostream>
#include <thread>
#include <semaphore>
#include <vector>
#include <chrono>

std::counting_semaphore<3> semaphore(3); // Max 3 concurrent threads

void accessResource(int id)
{
    semaphore.acquire(); // Wait for available slot
    
    std::cout << "Thread " << id << " accessing resource\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Thread " << id << " releasing resource\n";
    
    semaphore.release(); // Free slot
}

int main()
{
    std::vector<std::jthread> threads;
    
    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back(accessResource, i);
    }
    
    return 0;
}
```

### Binary Semaphore (C++20)

```cpp
#include <iostream>
#include <thread>
#include <semaphore>

std::binary_semaphore signal(0); // Initially unavailable

void waiter()
{
    std::cout << "Waiting for signal...\n";
    signal.acquire();
    std::cout << "Signal received!\n";
}

void signaler()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Sending signal...\n";
    signal.release();
}

int main()
{
    std::jthread t1(waiter);
    std::jthread t2(signaler);
    
    return 0;
}
```

### Latch (C++20) - One-Time Barrier

A latch is a single-use synchronization point.

```cpp
#include <iostream>
#include <thread>
#include <latch>
#include <vector>

std::latch workDone(3); // Wait for 3 threads

void worker(int id)
{
    std::cout << "Worker " << id << " doing work...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(id * 100));
    
    std::cout << "Worker " << id << " done\n";
    workDone.count_down(); // Decrement counter
}

int main()
{
    std::vector<std::jthread> workers;
    
    for (int i = 1; i <= 3; ++i)
    {
        workers.emplace_back(worker, i);
    }
    
    workDone.wait(); // Wait until count reaches 0
    std::cout << "All workers finished!\n";
    
    return 0;
}
```

### Barrier (C++20) - Reusable Synchronization

Barriers synchronize threads at multiple points.

```cpp
#include <iostream>
#include <thread>
#include <barrier>
#include <vector>

void phaseComplete()
{
    std::cout << "=== Phase completed ===\n";
}

std::barrier syncPoint(3, phaseComplete);

void phasedWorker(int id)
{
    for (int phase = 1; phase <= 3; ++phase)
    {
        std::cout << "Worker " << id << " phase " << phase << "\n";
        
        syncPoint.arrive_and_wait(); // Wait for all threads
    }
}

int main()
{
    std::vector<std::jthread> workers;
    
    for (int i = 1; i <= 3; ++i)
    {
        workers.emplace_back(phasedWorker, i);
    }
    
    return 0;
}
```
