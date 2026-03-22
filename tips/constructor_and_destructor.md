# C++ Constructor & Destructor — Ultimate Tips & Tricks
> Modern C++17/20 · Maximum Safety · Peak Optimization

---

## Table of Contents
1. [The Golden Rule](#1-the-golden-rule)
2. [Initializer List — Always Use It](#2-initializer-list--always-use-it)
3. [Constructor Types & When to Use Them](#3-constructor-types--when-to-use-them)
4. [Destructor Rules](#4-destructor-rules)
5. [RAII — The Core Philosophy](#5-raii--the-core-philosophy)
6. [The Rule of 0 / 3 / 5](#6-the-rule-of-0--3--5)
7. [Explicit Constructors — Prevent Silent Bugs](#7-explicit-constructors--prevent-silent-bugs)
8. [Delegating Constructors — Eliminate Duplication](#8-delegating-constructors--eliminate-duplication)
9. [= default and = delete](#9--default-and--delete)
10. [Move Semantics in Constructors](#10-move-semantics-in-constructors)
11. [noexcept — Performance & Safety](#11-noexcept--performance--safety)
12. [Avoid These Traps](#12-avoid-these-traps)
13. [Quick Reference Cheat Sheet](#13-quick-reference-cheat-sheet)

---

<a id="1-the-golden-rule"></a>
## 1. The Golden Rule

> **Constructor = Acquire resources only.**
> **Destructor = Release resources only.**
> **Never put real business logic in either.**

```cpp
// ✅ GOOD — constructor only acquires, nothing more
class DatabaseConnection
{
public:
    DatabaseConnection(std::string_view connection_string)
        : connection_string_(connection_string),
          handle_(OpenConnection(connection_string))  // acquire
    {
    }

    ~DatabaseConnection()
    {
        CloseConnection(handle_);  // release
    }

private:
    std::string       connection_string_;
    ConnectionHandle  handle_;
};
```

```cpp
// ❌ BAD — heavy logic inside constructor
class DataProcessor
{
public:
    DataProcessor(const std::string& file_path)
    {
        auto raw_data = LoadFile(file_path);       // I/O
        raw_data      = ParseJson(raw_data);       // parsing
        raw_data      = ValidateSchema(raw_data);  // validation
        result_       = Transform(raw_data);       // transformation
        // if ANY step throws, object is partially constructed — nightmare!
    }
};
```

```cpp
// ✅ GOOD — use a factory function for heavy work
class DataProcessor
{
public:
    // Factory: does the heavy lifting BEFORE constructing the object
    static DataProcessor Create(const std::string& file_path)
    {
        auto raw_data = LoadFile(file_path);
        raw_data      = ParseJson(raw_data);
        raw_data      = ValidateSchema(raw_data);
        return DataProcessor(Transform(raw_data));  // fully ready data
    }

private:
    explicit DataProcessor(ProcessedData data)
        : data_(std::move(data))
    {
    }

    ProcessedData data_;
};
```

---

<a id="2-initializer-list--always-use-it"></a>
## 2. Initializer List — Always Use It

### Why it is faster

Without an initializer list, members are **default-constructed first**, then **assigned** — two operations.
With an initializer list, members are **constructed once, directly** — one operation.

```cpp
class Player
{
public:
    std::string name_;
    int         health_;
    float       speed_;

    // ❌ BAD — default construct then assign (wasteful for non-trivial types)
    Player(std::string name, int health, float speed)
    {
        name_   = name;    // default-construct "" then copy-assign
        health_ = health;
        speed_  = speed;
    }

    // ✅ GOOD — direct construction, no wasted step
    Player(std::string name, int health, float speed)
        : name_(std::move(name)),   // move, not copy
          health_(health),
          speed_(speed)
    {
    }
};
```

### Initializer list is MANDATORY for these members

```cpp
class Engine
{
public:
    Engine(int cylinder_count, float displacement)
        : cylinder_count_(cylinder_count),    // const — MUST use initializer list
          displacement_(displacement),         // reference — MUST use initializer list
          id_(GenerateId())                    // no default constructor — MUST use initializer list
    {
    }

private:
    const int       cylinder_count_;   // const member
    float&          displacement_;     // reference member
    UniqueEngine    id_;               // type with no default constructor
};
```

### Initializer list order = declaration order

> Members are initialized in **declaration order**, not initializer list order.
> Always write them in the same order to avoid confusion and subtle bugs.

```cpp
class Vector2D
{
    float x_;   // initialized 1st
    float y_;   // initialized 2nd

public:
    // ✅ matches declaration order — clear and safe
    Vector2D(float x, float y)
        : x_(x),
          y_(y)
    {
    }

    // ❌ reversed order in list, but x_ still initializes before y_ at runtime
    // If y_ depended on x_, this would silently bug out
    Vector2D(float x, float y)
        : y_(y),
          x_(x)
    {
    }
};
```

---

<a id="3-constructor-types--when-to-use-them"></a>
## 3. Constructor Types & When to Use Them

### Default Constructor

```cpp
class Config
{
public:
    Config() = default;   // let compiler generate it — zero overhead

private:
    int     timeout_ms_  = 5000;   // in-class default values (C++11)
    bool    retry_       = true;
    size_t  max_retries_ = 3;
};
```

### Parameterized Constructor

```cpp
class HttpRequest
{
public:
    HttpRequest(std::string url, std::string method = "GET")
        : url_(std::move(url)),
          method_(std::move(method))
    {
    }

private:
    std::string url_;
    std::string method_;
};
```

### Copy Constructor

```cpp
class Buffer
{
public:
    Buffer(const Buffer& other)
        : size_(other.size_),
          data_(std::make_unique<uint8_t[]>(other.size_))
    {
        std::copy(other.data_.get(), other.data_.get() + size_, data_.get());
    }

private:
    size_t                      size_;
    std::unique_ptr<uint8_t[]>  data_;
};
```

### Move Constructor

```cpp
class Buffer
{
public:
    Buffer(Buffer&& other) noexcept
        : size_(std::exchange(other.size_, 0)),
          data_(std::move(other.data_))      // steal the resource
    {
        // other is now empty — valid but unspecified state
    }
};
```

### Converting Constructor

```cpp
class Radius
{
public:
    explicit Radius(double value)   // explicit prevents accidental implicit conversion
        : value_(value)
    {
    }

private:
    double value_;
};
```

---

<a id="4-destructor-rules"></a>
## 4. Destructor Rules

```cpp
class ResourceOwner
{
public:
    // ✅ Always virtual destructor in base classes
    virtual ~ResourceOwner() = default;

    // ✅ noexcept on destructor — destructors must NEVER throw
    ~ResourceOwner() noexcept
    {
        CleanUp();   // must not throw — wrap with try/catch if needed
    }

private:
    void CleanUp() noexcept
    {
        try
        {
            ReleaseHandle(handle_);
        }
        catch (...)
        {
            // swallow — destructors must not propagate exceptions
        }
    }

    Handle handle_;
};
```

### Rule: Base class destructor MUST be virtual

```cpp
// ❌ BAD — memory leak when deleting via base pointer
class Base
{
public:
    ~Base() { }   // non-virtual
};

class Derived : public Base
{
    std::vector<int> data_;  // never freed!
};

Base* ptr = new Derived();
delete ptr;   // only Base::~Base() runs — Derived::~Derived() skipped

// ✅ GOOD
class Base
{
public:
    virtual ~Base() = default;
};
```

---

<a id="5-raii--the-core-philosophy"></a>
## 5. RAII — The Core Philosophy

**Resource Acquisition Is Initialization** — tie every resource to an object's lifetime.

```cpp
// ✅ The RAII pattern — resource is safe by design
class FileHandle
{
public:
    explicit FileHandle(const char* path, const char* mode)
        : file_(std::fopen(path, mode))
    {
        if (!file_)
        {
            throw std::runtime_error("Failed to open file");
        }
    }

    ~FileHandle() noexcept
    {
        if (file_)
        {
            std::fclose(file_);
        }
    }

    // Disable copy — a file handle is unique
    FileHandle(const FileHandle&)            = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    // Allow move
    FileHandle(FileHandle&& other) noexcept
        : file_(std::exchange(other.file_, nullptr))
    {
    }

    std::FILE* Get() const noexcept { return file_; }

private:
    std::FILE* file_;
};

// Usage — file closes automatically, even on exception
void ProcessFile(const char* path)
{
    FileHandle file(path, "r");
    // ... do work ...
}   // destructor runs here — guaranteed
```

### Prefer smart pointers over raw pointers

```cpp
// ❌ BAD — manual memory management
class Scene
{
    Mesh* mesh_;   // who owns this? when does it delete?

public:
    Scene() : mesh_(new Mesh()) { }
    ~Scene() { delete mesh_; }   // easy to forget, easy to double-delete
};

// ✅ GOOD — ownership is explicit and automatic
class Scene
{
    std::unique_ptr<Mesh> mesh_;   // single owner, auto-deleted

public:
    Scene() : mesh_(std::make_unique<Mesh>()) { }
    // No destructor needed — unique_ptr handles it
};

// ✅ GOOD — shared ownership
class Scene
{
    std::shared_ptr<Mesh> mesh_;   // reference-counted, safe to share
};
```

---

<a id="6-the-rule-of-0--3--5"></a>
## 6. The Rule of 0 / 3 / 5

### Rule of Zero — the ideal

> If your class manages no raw resources, define **nothing**.
> Let the compiler generate everything — it is correct and fast.

```cpp
// ✅ Rule of Zero — nothing to define
class Player
{
    std::string             name_;
    std::vector<Item>       inventory_;
    std::unique_ptr<Weapon> weapon_;
    // All members manage themselves — compiler-generated special functions are perfect
};
```

### Rule of Five — when you own raw resources

> If you define **any** of the five, define **all five**.

```cpp
class RawBuffer
{
public:
    explicit RawBuffer(size_t size)
        : size_(size),
          data_(new uint8_t[size])
    {
    }

    // 1. Destructor
    ~RawBuffer() noexcept
    {
        delete[] data_;
    }

    // 2. Copy constructor
    RawBuffer(const RawBuffer& other)
        : size_(other.size_),
          data_(new uint8_t[other.size_])
    {
        std::copy(other.data_, other.data_ + size_, data_);
    }

    // 3. Copy assignment
    RawBuffer& operator=(const RawBuffer& other)
    {
        if (this != &other)
        {
            uint8_t* new_data = new uint8_t[other.size_];
            std::copy(other.data_, other.data_ + other.size_, new_data);
            delete[] data_;
            data_ = new_data;
            size_ = other.size_;
        }
        return *this;
    }

    // 4. Move constructor
    RawBuffer(RawBuffer&& other) noexcept
        : size_(std::exchange(other.size_, 0)),
          data_(std::exchange(other.data_, nullptr))
    {
    }

    // 5. Move assignment
    RawBuffer& operator=(RawBuffer&& other) noexcept
    {
        if (this != &other)
        {
            delete[] data_;
            size_ = std::exchange(other.size_, 0);
            data_ = std::exchange(other.data_, nullptr);
        }
        return *this;
    }

private:
    size_t   size_;
    uint8_t* data_;
};
```

---

<a id="7-explicit-constructors--prevent-silent-bugs"></a>
## 7. Explicit Constructors — Prevent Silent Bugs

```cpp
class Timeout
{
public:
    // ❌ BAD — implicit conversion allowed
    Timeout(int milliseconds) : ms_(milliseconds) { }

private:
    int ms_;
};

void Connect(Timeout t) { }

Connect(5000);    // ok, intentional
Connect(true);    // ❌ silently compiles — bool to int to Timeout!
Connect(3.7f);    // ❌ silently truncates float to int


class Timeout
{
public:
    // ✅ GOOD — explicit blocks accidental conversions
    explicit Timeout(int milliseconds) : ms_(milliseconds) { }

private:
    int ms_;
};

Connect(Timeout(5000));   // ✅ must be intentional
Connect(true);            // ❌ compile error — caught at compile time
```

---

<a id="8-delegating-constructors--eliminate-duplication"></a>
## 8. Delegating Constructors — Eliminate Duplication

```cpp
// ❌ BAD — initialization logic repeated in each constructor
class Server
{
public:
    Server()
        : host_("localhost"), port_(8080), timeout_ms_(5000)
    {
        InitLogger();
        InitMetrics();
    }

    Server(std::string host, int port)
        : host_(std::move(host)), port_(port), timeout_ms_(5000)
    {
        InitLogger();    // duplicated
        InitMetrics();   // duplicated
    }
};

// ✅ GOOD — delegate to one canonical constructor
class Server
{
public:
    Server()
        : Server("localhost", 8080)   // delegate — no duplication
    {
    }

    Server(std::string host, int port)
        : Server(std::move(host), port, 5000)   // delegate further
    {
    }

    Server(std::string host, int port, int timeout_ms)
        : host_(std::move(host)),
          port_(port),
          timeout_ms_(timeout_ms)
    {
        InitLogger();    // runs once, from one place
        InitMetrics();
    }

private:
    std::string  host_;
    int          port_;
    int          timeout_ms_;
};
```

---

<a id="9--default-and--delete"></a>
## 9. `= default` and `= delete`

```cpp
class UniqueId
{
public:
    UniqueId() = default;

    // Prevent copying — an ID should be unique
    UniqueId(const UniqueId&)            = delete;
    UniqueId& operator=(const UniqueId&) = delete;

    // Allow moving
    UniqueId(UniqueId&&)            = default;
    UniqueId& operator=(UniqueId&&) = default;
};


// Prevent heap allocation — force stack-only use
class StackOnly
{
public:
    void* operator new(std::size_t)   = delete;
    void* operator new[](std::size_t) = delete;
};


// Singleton — delete all construction except controlled factory
class AppConfig
{
public:
    static AppConfig& Instance()
    {
        static AppConfig config;
        return config;
    }

    AppConfig(const AppConfig&)            = delete;
    AppConfig& operator=(const AppConfig&) = delete;

private:
    AppConfig() = default;
};
```

---

<a id="10-move-semantics-in-constructors"></a>
## 10. Move Semantics in Constructors

```cpp
class EventSystem
{
public:
    // ✅ take by value + move when storing strings/vectors
    explicit EventSystem(std::string name)
        : name_(std::move(name))   // move into member — zero copy
    {
    }

    // ✅ for large objects, forward perfectly
    template<typename T>
    void RegisterHandler(T&& handler)
    {
        handlers_.emplace_back(std::forward<T>(handler));
    }

private:
    std::string                          name_;
    std::vector<std::function<void()>>   handlers_;
};

// ✅ use std::exchange in move constructor for clean state reset
class Socket
{
public:
    Socket(Socket&& other) noexcept
        : fd_(std::exchange(other.fd_, -1))   // steal fd, leave other in defined state
    {
    }

private:
    int fd_ = -1;
};
```

---

<a id="11-noexcept--performance--safety"></a>
## 11. `noexcept` — Performance & Safety

```cpp
class Vec3
{
public:
    // ✅ move constructor MUST be noexcept for STL optimizations
    // std::vector uses move only if move constructor is noexcept
    Vec3(Vec3&& other) noexcept
        : x_(other.x_), y_(other.y_), z_(other.z_)
    {
    }

    // ✅ destructor is implicitly noexcept — be explicit anyway
    ~Vec3() noexcept = default;

    // ✅ trivial operations — mark noexcept for inlining hints
    float Length() const noexcept
    {
        return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
    }

private:
    float x_, y_, z_;
};
```

> **Rule:** If your move constructor can throw, `std::vector` will **copy** instead of move on reallocation.
> Always mark move constructors and move assignment operators `noexcept`.

---

<a id="12-avoid-these-traps"></a>
## 12. Avoid These Traps

### Never call virtual functions in constructor/destructor

```cpp
class Base
{
public:
    Base()
    {
        Init();   // ❌ calls Base::Init(), NOT Derived::Init() — vtable not ready yet
    }

    virtual void Init() { }
};

class Derived : public Base
{
    void Init() override
    {
        // This NEVER runs when constructing via Base()
    }
};

// ✅ GOOD — use factory or two-phase init
class Base
{
public:
    static std::unique_ptr<Base> Create()
    {
        auto obj = std::make_unique<Derived>();
        obj->Init();   // called AFTER full construction
        return obj;
    }

    virtual void Init() { }
};
```

### Never let exceptions escape destructors

```cpp
// ❌ BAD — exception in destructor during stack unwinding = std::terminate()
~NetworkClient() noexcept(false)
{
    Disconnect();   // might throw!
}

// ✅ GOOD — swallow or log
~NetworkClient() noexcept
{
    try
    {
        Disconnect();
    }
    catch (const std::exception& e)
    {
        LogError("Disconnect failed: ", e.what());
        // swallow — never propagate from destructor
    }
}
```

### Avoid two-phase initialization

```cpp
// ❌ BAD — object exists in invalid state between new and Init()
AudioEngine engine;
engine.Init("config.json");   // what if someone uses engine before Init()?

// ✅ GOOD — use constructor + factory
class AudioEngine
{
public:
    static std::optional<AudioEngine> Create(const std::string& config_path)
    {
        auto cfg = LoadConfig(config_path);
        if (!cfg) return std::nullopt;
        return AudioEngine(std::move(*cfg));
    }

private:
    explicit AudioEngine(Config cfg) : cfg_(std::move(cfg)) { }

    Config cfg_;
};
```

### Avoid `this` pointer escape in constructor

```cpp
// ❌ BAD — object not fully constructed yet when passed to registry
class Widget
{
public:
    Widget()
    {
        Registry::Register(this);   // this is not fully alive yet!
    }
};

// ✅ GOOD — register after construction
auto widget = std::make_unique<Widget>();
Registry::Register(widget.get());   // object is complete
```

---

<a id="13-quick-reference-cheat-sheet"></a>
## 13. Quick Reference Cheat Sheet

| Situation | Best Practice |
|---|---|
| Heavy work before construction | Use a static factory function |
| Storing a parameter | Take by value, then `std::move` into member |
| `const` or reference member | **Must** use initializer list |
| Base class | Always declare `virtual ~Base() = default` |
| Exception in destructor | Catch and swallow, never propagate |
| No raw resources | Apply Rule of Zero — define nothing |
| Raw resource ownership | Apply Rule of Five — define all five |
| Single-argument constructor | Mark `explicit` to block implicit conversions |
| Duplicate constructor logic | Use delegating constructors |
| Move constructor / assignment | Always mark `noexcept` |
| Virtual call in ctor/dtor | Never — use factory pattern instead |
| Two-phase `Init()` pattern | Replace with constructor + `std::optional` factory |
| Shared resource | Use `std::shared_ptr` |
| Unique resource | Use `std::unique_ptr` |

---

> **Summary:** Constructors acquire. Destructors release. Everything else belongs somewhere else.
> Use RAII, smart pointers, initializer lists, `noexcept` on moves, and the Rule of Zero whenever possible.
> The less logic in your constructor, the safer, faster, and more testable your class becomes.
