# C++ Multithreading: Complete Guide (C++17 - C++23)

A comprehensive guide to modern C++ multithreading, covering all features from C++17 through C++23 with practical examples and best practices.

## Table of Contents

- [Introduction](#introduction)
- [Thread Basics](#thread-basics)
- [Thread Management](#thread-management)
- [Synchronization Primitives](#synchronization-primitives)
- [Atomics](#atomics)
- [Modern Features (C++20/23)](#modern-features-c2023)
- [CPU Management](#cpu-management)
- [Best Practices](#best-practices)
- [Common Patterns](#common-patterns)

---

## Introduction

Modern C++ provides robust support for multithreading through the `<thread>`, `<mutex>`, `<condition_variable>`, `<atomic>`, `<semaphore>`, and `<barrier>` headers. This guide covers everything from basic thread creation to advanced synchronization techniques.

### Requirements

- **C++17**: Basic threading, mutexes, condition variables
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

---

## CPU Management

### Thread Affinity

Control which CPU cores threads run on (platform-specific).

```cpp
#include <iostream>
#include <thread>
#include <vector>

#ifdef __linux__
#include <pthread.h>

void setCoreAffinity(int coreId)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreId, &cpuset);
    
    pthread_t thread = pthread_self();
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    
    std::cout << "Thread running on core " << coreId << "\n";
}
#endif

void worker(int coreId)
{
#ifdef __linux__
    setCoreAffinity(coreId);
#endif
    
    // Simulate work
    volatile int sum = 0;
    for (int i = 0; i < 100000000; ++i)
    {
        sum += i;
    }
}

int main()
{
    unsigned int numCores = std::thread::hardware_concurrency();
    std::cout << "Available cores: " << numCores << "\n";
    
    std::vector<std::thread> threads;
    
    for (unsigned int i = 0; i < numCores; ++i)
    {
        threads.emplace_back(worker, i % numCores);
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    return 0;
}
```

### Thread Priority (Platform-Specific)

```cpp
#include <iostream>
#include <thread>

#ifdef __linux__
#include <pthread.h>

void setThreadPriority(int priority)
{
    sched_param param;
    param.sched_priority = priority;
    
    pthread_t thread = pthread_self();
    pthread_setschedparam(thread, SCHED_FIFO, &param);
}
#endif

void highPriorityTask()
{
#ifdef __linux__
    setThreadPriority(10);
#endif
    std::cout << "High priority task running\n";
}

int main()
{
    std::thread t(highPriorityTask);
    t.join();
    
    return 0;
}
```

---

## Best Practices

### 1. Prefer RAII for Lock Management

```cpp
// ✓ Good: Automatic unlock
{
    std::lock_guard<std::mutex> lock(mtx);
    // Critical section
} // Automatically unlocked

// ✗ Bad: Manual lock/unlock
mtx.lock();
// Critical section
mtx.unlock(); // Can be missed if exception occurs
```

### 2. Avoid Deadlocks

```cpp
// ✓ Good: std::scoped_lock acquires all locks atomically
std::scoped_lock lock(mtx1, mtx2);

// ✗ Bad: Can deadlock if another thread locks in reverse order
mtx1.lock();
mtx2.lock();
```

### 3. Minimize Lock Scope

```cpp
// ✓ Good: Short critical section
{
    std::lock_guard<std::mutex> lock(mtx);
    data.push_back(value);
}
processData(); // Outside lock

// ✗ Bad: Unnecessarily long critical section
{
    std::lock_guard<std::mutex> lock(mtx);
    data.push_back(value);
    processData(); // Holding lock too long
}
```

### 4. Use Atomics for Simple Operations

```cpp
// ✓ Good: Lock-free atomic increment
std::atomic<int> counter(0);
counter.fetch_add(1);

// ✗ Overkill: Mutex for simple counter
std::mutex mtx;
int counter = 0;
{
    std::lock_guard<std::mutex> lock(mtx);
    ++counter;
}
```

### 5. Prefer jthread Over thread (C++20+)

```cpp
// ✓ Good: Auto-joins, supports cancellation
{
    std::jthread t(task);
    // Automatically joins on scope exit
}

// ✗ Requires manual join
{
    std::thread t(task);
    t.join(); // Must remember to join
}
```

### 6. Use Condition Variables Correctly

```cpp
// ✓ Good: Predicate prevents spurious wakeups
std::unique_lock<std::mutex> lock(mtx);
cv.wait(lock, []{ return ready; });

// ✗ Bad: No predicate, vulnerable to spurious wakeups
std::unique_lock<std::mutex> lock(mtx);
cv.wait(lock);
if (!ready) { /* Problem! */ }
```

### 7. Choose Appropriate Memory Order

```cpp
// ✓ Good: Relaxed for simple counter
atomicCounter.fetch_add(1, std::memory_order_relaxed);

// ✓ Good: Acquire-release for synchronization
ready.store(true, std::memory_order_release);
while (!ready.load(std::memory_order_acquire));

// ✗ Overkill: Sequential consistency when not needed
atomicCounter.fetch_add(1, std::memory_order_seq_cst); // Default, but slower
```

---

## Common Patterns

### Thread Pool

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool
{
public:
    ThreadPool(size_t numThreads)
    {
        for (size_t i = 0; i < numThreads; ++i)
        {
            workers.emplace_back([this]
            {
                while (true)
                {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this]
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
            std::lock_guard<std::mutex> lock(queueMutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }
    
    ~ThreadPool()
    {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        
        for (auto& worker : workers)
        {
            worker.join();
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop = false;
};

int main()
{
    ThreadPool pool(4);
    
    for (int i = 0; i < 10; ++i)
    {
        pool.enqueue([i]
        {
            std::cout << "Task " << i << " executing\n";
        });
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return 0;
}
```

### Parallel Accumulate

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <numeric>

template<typename Iterator, typename T>
void accumulateBlock(Iterator first, Iterator last, T& result)
{
    result = std::accumulate(first, last, result);
}

template<typename Iterator, typename T>
T parallelAccumulate(Iterator first, Iterator last, T init)
{
    unsigned long const length = std::distance(first, last);
    
    if (length == 0)
    {
        return init;
    }
    
    unsigned long const numThreads = std::thread::hardware_concurrency();
    unsigned long const blockSize = length / numThreads;
    
    std::vector<T> results(numThreads);
    std::vector<std::thread> threads(numThreads - 1);
    
    Iterator blockStart = first;
    
    for (unsigned long i = 0; i < (numThreads - 1); ++i)
    {
        Iterator blockEnd = blockStart;
        std::advance(blockEnd, blockSize);
        
        threads[i] = std::thread(accumulateBlock<Iterator, T>,
                                 blockStart, blockEnd, std::ref(results[i]));
        
        blockStart = blockEnd;
    }
    
    accumulateBlock(blockStart, last, results[numThreads - 1]);
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    return std::accumulate(results.begin(), results.end(), init);
}

int main()
{
    std::vector<int> data(10000000);
    std::iota(data.begin(), data.end(), 1);
    
    auto start = std::chrono::high_resolution_clock::now();
    long long sum = parallelAccumulate(data.begin(), data.end(), 0LL);
    auto end = std::chrono::high_resolution_clock::now();
    
    std::cout << "Sum: " << sum << "\n";
    std::cout << "Time: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
              << "ms\n";
    
    return 0;
}
```

### Producer-Consumer with Bounded Buffer

```cpp
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>

template<typename T>
class BoundedBuffer
{
public:
    BoundedBuffer(size_t maxSize) : maxSize(maxSize) {}
    
    void produce(T item)
    {
        std::unique_lock<std::mutex> lock(mtx);
        notFull.wait(lock, [this]{ return buffer.size() < maxSize; });
        
        buffer.push(item);
        std::cout << "Produced: " << item << " (size: " << buffer.size() << ")\n";
        
        notEmpty.notify_one();
    }
    
    T consume()
    {
        std::unique_lock<std::mutex> lock(mtx);
        notEmpty.wait(lock, [this]{ return !buffer.empty(); });
        
        T item = buffer.front();
        buffer.pop();
        std::cout << "Consumed: " << item << " (size: " << buffer.size() << ")\n";
        
        notFull.notify_one();
        
        return item;
    }

private:
    std::queue<T> buffer;
    size_t maxSize;
    std::mutex mtx;
    std::condition_variable notFull;
    std::condition_variable notEmpty;
};

void producer(BoundedBuffer<int>& buffer, int count)
{
    for (int i = 0; i < count; ++i)
    {
        buffer.produce(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void consumer(BoundedBuffer<int>& buffer, int count)
{
    for (int i = 0; i < count; ++i)
    {
        buffer.consume();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main()
{
    BoundedBuffer<int> buffer(5);
    
    std::thread prod(producer, std::ref(buffer), 10);
    std::thread cons(consumer, std::ref(buffer), 10);
    
    prod.join();
    cons.join();
    
    return 0;
}
```

### Read-Write Lock Pattern

```cpp
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <vector>
#include <chrono>
#include <string>

class SharedData
{
public:
    std::string read() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return data;
    }
    
    void write(const std::string& newData)
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        data = newData;
        std::cout << "Data written: " << data << "\n";
    }

private:
    mutable std::shared_mutex mutex;
    std::string data = "initial";
};

void reader(SharedData& shared, int id)
{
    for (int i = 0; i < 3; ++i)
    {
        std::string value = shared.read();
        std::cout << "Reader " << id << " read: " << value << "\n";
    }
}

void writer(SharedData& shared, int id)
{
    for (int i = 0; i < 2; ++i)
    {
        shared.write("data_" + std::to_string(id) + "_" + std::to_string(i));
    }
}

int main()
{
    SharedData shared;
    
    std::vector<std::thread> threads;
    
    // Multiple readers
    for (int i = 1; i <= 3; ++i)
    {
        threads.emplace_back(reader, std::ref(shared), i);
    }
    
    // Few writers
    for (int i = 1; i <= 2; ++i)
    {
        threads.emplace_back(writer, std::ref(shared), i);
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    return 0;
}
```

### Lock-Free Stack (using Atomics)

```cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>

template<typename T>
class LockFreeStack
{
private:
    struct Node
    {
        T data;
        Node* next;
        
        Node(const T& data) : data(data), next(nullptr) {}
    };
    
    std::atomic<Node*> head;

public:
    LockFreeStack() : head(nullptr) {}
    
    void push(const T& data)
    {
        Node* newNode = new Node(data);
        newNode->next = head.load(std::memory_order_relaxed);
        
        while (!head.compare_exchange_weak(newNode->next, newNode,
                                           std::memory_order_release,
                                           std::memory_order_relaxed))
        {
            // Retry if CAS fails
        }
    }
    
    bool pop(T& result)
    {
        Node* oldHead = head.load(std::memory_order_relaxed);
        
        while (oldHead && !head.compare_exchange_weak(oldHead, oldHead->next,
                                                      std::memory_order_acquire,
                                                      std::memory_order_relaxed))
        {
            // Retry if CAS fails
        }
        
        if (oldHead)
        {
            result = oldHead->data;
            delete oldHead;
            return true;
        }
        
        return false;
    }
    
    ~LockFreeStack()
    {
        T dummy;
        while (pop(dummy));
    }
};

void pusher(LockFreeStack<int>& stack, int start, int count)
{
    for (int i = start; i < start + count; ++i)
    {
        stack.push(i);
    }
}

void popper(LockFreeStack<int>& stack, int count)
{
    for (int i = 0; i < count; ++i)
    {
        int value;
        if (stack.pop(value))
        {
            std::cout << "Popped: " << value << "\n";
        }
    }
}

int main()
{
    LockFreeStack<int> stack;
    
    std::vector<std::thread> threads;
    
    threads.emplace_back(pusher, std::ref(stack), 0, 10);
    threads.emplace_back(pusher, std::ref(stack), 100, 10);
    threads.emplace_back(popper, std::ref(stack), 10);
    threads.emplace_back(popper, std::ref(stack), 10);
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    return 0;
}
```

### Future and Promise Pattern

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <chrono>

int computeValue(int input)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return input * input;
}

void promiseBasedCompute(std::promise<int> prom, int input)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    prom.set_value(input * input);
}

int main()
{
    // Using std::async
    std::cout << "=== Using std::async ===\n";
    std::future<int> futureResult = std::async(std::launch::async, computeValue, 10);
    std::cout << "Computing in background...\n";
    std::cout << "Result: " << futureResult.get() << "\n\n";
    
    // Using std::promise and std::future
    std::cout << "=== Using std::promise ===\n";
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    
    std::thread t(promiseBasedCompute, std::move(prom), 20);
    std::cout << "Waiting for promise...\n";
    std::cout << "Result: " << fut.get() << "\n\n";
    t.join();
    
    // Multiple async tasks
    std::cout << "=== Multiple async tasks ===\n";
    std::vector<std::future<int>> futures;
    
    for (int i = 1; i <= 5; ++i)
    {
        futures.push_back(std::async(std::launch::async, computeValue, i));
    }
    
    for (auto& f : futures)
    {
        std::cout << "Result: " << f.get() << "\n";
    }
    
    return 0;
}
```

---

## Performance Considerations

### 1. Thread Creation Overhead

Creating threads is expensive. Use thread pools for frequent short-lived tasks.

```cpp
// ✗ Bad: Creating thread for each task
for (int i = 0; i < 1000; ++i)
{
    std::thread t(smallTask, i);
    t.join();
}

// ✓ Good: Use thread pool
ThreadPool pool(4);
for (int i = 0; i < 1000; ++i)
{
    pool.enqueue([i]{ smallTask(i); });
}
```

### 2. False Sharing

Avoid multiple threads writing to adjacent memory locations.

```cpp
// ✗ Bad: False sharing
struct Counter
{
    std::atomic<int> count1;
    std::atomic<int> count2; // Likely in same cache line
};

// ✓ Good: Cache-line padding
struct alignas(64) Counter
{
    std::atomic<int> count1;
    char padding[60]; // Prevent false sharing
    std::atomic<int> count2;
};
```

### 3. Contention Reduction

Minimize time spent holding locks.

```cpp
// ✗ Bad: High contention
void process()
{
    std::lock_guard<std::mutex> lock(mtx);
    // Long computation while holding lock
    expensiveOperation();
}

// ✓ Good: Reduced contention
void process()
{
    auto result = expensiveOperation(); // Outside lock
    
    std::lock_guard<std::mutex> lock(mtx);
    // Only critical section under lock
    updateSharedData(result);
}
```

### 4. Choose Right Synchronization

Match synchronization primitive to use case:

- **Atomic**: Simple counters, flags
- **Mutex**: General critical sections
- **Shared Mutex**: Read-heavy workloads
- **Condition Variable**: Event-based synchronization
- **Semaphore**: Resource counting
- **Barrier/Latch**: Phased computation

---

## Debugging Tips

### 1. Thread Sanitizer

Compile with `-fsanitize=thread` to detect data races:

```bash
g++ -std=c++20 -fsanitize=thread -g program.cpp -o program
./program
```

### 2. Race Detection Example

```cpp
#include <iostream>
#include <thread>

int sharedValue = 0; // Race condition!

void increment()
{
    for (int i = 0; i < 1000000; ++i)
    {
        ++sharedValue; // Data race detected by thread sanitizer
    }
}

int main()
{
    std::thread t1(increment);
    std::thread t2(increment);
    
    t1.join();
    t2.join();
    
    std::cout << "Value: " << sharedValue << "\n";
    
    return 0;
}
```

### 3. Deadlock Detection

Look for circular dependencies in lock acquisition:

```cpp
// Thread 1
mtx1.lock();
mtx2.lock(); // Deadlock if Thread 2 holds mtx2

// Thread 2
mtx2.lock();
mtx1.lock(); // Deadlock if Thread 1 holds mtx1
```

---

## Summary

### Quick Reference

| Feature | C++ Version | Use Case |
|---------|-------------|----------|
| `std::thread` | C++11 | Basic threading |
| `std::mutex` | C++11 | Mutual exclusion |
| `std::lock_guard` | C++11 | RAII mutex lock |
| `std::unique_lock` | C++11 | Flexible locking |
| `std::scoped_lock` | C++17 | Multiple mutexes |
| `std::shared_mutex` | C++17 | Reader-writer locks |
| `std::condition_variable` | C++11 | Event synchronization |
| `std::atomic` | C++11 | Lock-free operations |
| `std::jthread` | C++20 | Auto-joining thread |
| `std::counting_semaphore` | C++20 | Resource counting |
| `std::binary_semaphore` | C++20 | Simple signaling |
| `std::latch` | C++20 | One-time barrier |
| `std::barrier` | C++20 | Reusable barrier |

### Memory Order Quick Guide

| Memory Order | Use Case |
|--------------|----------|
| `memory_order_relaxed` | Simple counters, no ordering |
| `memory_order_acquire` | Load with synchronization |
| `memory_order_release` | Store with synchronization |
| `memory_order_acq_rel` | Read-modify-write |
| `memory_order_seq_cst` | Strongest ordering (default) |

---

## Resources

- [C++ Reference - Thread Support](https://en.cppreference.com/w/cpp/thread)
- [C++ Concurrency in Action (Book)](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition)
- [CppCon Talks on Concurrency](https://www.youtube.com/user/CppCon)

---
