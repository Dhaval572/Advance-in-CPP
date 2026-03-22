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
    DatabaseConnection(std::string_view conn_str)
        : m_ConnStr(conn_str),
          m_Handle(OpenConnection(conn_str))  // acquire
    {
    }

    ~DatabaseConnection()
    {
        CloseConnection(m_Handle);  // release
    }

private:
    std::string       m_ConnStr;
    ConnectionHandle  m_Handle;
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
        m_Result      = Transform(raw_data);       // transformation
        // if ANY step throws, object is partially constructed — nightmare!
    }

private:
    ProcessedData m_Result;
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
        : m_Data(std::move(data))
    {
    }

    ProcessedData m_Data;
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
    // ❌ BAD — default construct then assign (wasteful for non-trivial types)
    Player(std::string name, int health, float speed)
    {
        m_Name   = name;    // default-construct "" then copy-assign
        m_Health = health;
        m_Speed  = speed;
    }

    // ✅ GOOD — direct construction, no wasted step
    Player(std::string name, int health, float speed)
        : m_Name(std::move(name)),   // move, not copy
          m_Health(health),
          m_Speed(speed)
    {
    }

private:
    std::string m_Name;
    int         m_Health;
    float       m_Speed;
};
```

### Initializer list is MANDATORY for these members

```cpp
class Engine
{
public:
    Engine(int cylinders, float& displacement)
        : m_Cylinders(cylinders),       // const — MUST use initializer list
          m_Displacement(displacement),  // reference — MUST use initializer list
          m_Id(GenerateId())             // no default constructor — MUST use initializer list
    {
    }

private:
    const int  m_Cylinders;    // const member
    float&     m_Displacement; // reference member
    EngineId   m_Id;           // type with no default constructor
};
```

### Initializer list order = declaration order

> Members are initialized in **declaration order**, not initializer list order.
> Always write them in the same order to avoid confusion and subtle bugs.

```cpp
class Vector2D
{
public:
    // ✅ matches declaration order — clear and safe
    Vector2D(float x, float y)
        : m_X(x),
          m_Y(y)
    {
    }

    // ❌ reversed order in list, but m_X still initializes before m_Y at runtime
    // If m_Y depended on m_X, this would silently bug out
    Vector2D(float x, float y)
        : m_Y(y),
          m_X(x)
    {
    }

private:
    float m_X;   // initialized 1st
    float m_Y;   // initialized 2nd
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
    int     m_TimeoutMs  = 5000;   // in-class default values (C++11)
    bool    m_Retry      = true;
    size_t  m_MaxRetries = 3;
};
```

### Parameterized Constructor

```cpp
class HttpRequest
{
public:
    HttpRequest(std::string url, std::string method = "GET")
        : m_Url(std::move(url)),
          m_Method(std::move(method))
    {
    }

private:
    std::string m_Url;
    std::string m_Method;
};
```

### Copy Constructor

```cpp
class Buffer
{
public:
    Buffer(const Buffer& other)
        : m_Size(other.m_Size),
          m_Data(std::make_unique<uint8_t[]>(other.m_Size))
    {
        std::copy(other.m_Data.get(), other.m_Data.get() + m_Size, m_Data.get());
    }

private:
    size_t                      m_Size;
    std::unique_ptr<uint8_t[]>  m_Data;
};
```

### Move Constructor

```cpp
class Buffer
{
public:
    Buffer(Buffer&& other) noexcept
        : m_Size(std::exchange(other.m_Size, 0)),
          m_Data(std::move(other.m_Data))      // steal the resource
    {
        // other is now empty — valid but unspecified state
    }

private:
    size_t                      m_Size;
    std::unique_ptr<uint8_t[]>  m_Data;
};
```

### Converting Constructor

```cpp
class Radius
{
public:
    explicit Radius(double value)   // explicit prevents accidental implicit conversion
        : m_Value(value)
    {
    }

private:
    double m_Value;
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
            ReleaseHandle(m_Handle);
        }
        catch (...)
        {
            // swallow — destructors must not propagate exceptions
        }
    }

    Handle m_Handle;
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
    std::vector<int> m_Items;  // never freed!
};

Base* obj = new Derived();
delete obj;   // only Base::~Base() runs — Derived::~Derived() skipped

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
        : m_File(std::fopen(path, mode))
    {
        if (!m_File)
        {
            throw std::runtime_error("Failed to open file");
        }
    }

    ~FileHandle() noexcept
    {
        if (m_File)
        {
            std::fclose(m_File);
        }
    }

    // Disable copy — a file handle is unique
    FileHandle(const FileHandle&)            = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    // Allow move
    FileHandle(FileHandle&& other) noexcept
        : m_File(std::exchange(other.m_File, nullptr))
    {
    }

    std::FILE* Get() const noexcept { return m_File; }

private:
    std::FILE* m_File;
};

// Usage — file closes automatically, even on exception
void ProcessLog(const char* path)
{
    FileHandle log(path, "r");
    // ... do work ...
}   // destructor runs here — guaranteed
```

### Prefer smart pointers over raw pointers

```cpp
// ❌ BAD — manual memory management
class Scene
{
public:
    Scene() : m_Mesh(new Mesh()) { }
    ~Scene() { delete m_Mesh; }   // easy to forget, easy to double-delete

private:
    Mesh* m_Mesh;   // who owns this? when does it delete?
};

// ✅ GOOD — ownership is explicit and automatic
class Scene
{
public:
    Scene() : m_Mesh(std::make_unique<Mesh>()) { }
    // No destructor needed — unique_ptr handles it

private:
    std::unique_ptr<Mesh> m_Mesh;   // single owner, auto-deleted
};

// ✅ GOOD — shared ownership
class Scene
{
private:
    std::shared_ptr<Mesh> m_Mesh;   // reference-counted, safe to share
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
private:
    std::string             m_Name;
    std::vector<Item>       m_Inventory;
    std::unique_ptr<Weapon> m_Weapon;
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
        : m_Size(size),
          m_Data(new uint8_t[size])
    {
    }

    // 1. Destructor
    ~RawBuffer() noexcept
    {
        delete[] m_Data;
    }

    // 2. Copy constructor
    RawBuffer(const RawBuffer& other)
        : m_Size(other.m_Size),
          m_Data(new uint8_t[other.m_Size])
    {
        std::copy(other.m_Data, other.m_Data + m_Size, m_Data);
    }

    // 3. Copy assignment
    RawBuffer& operator=(const RawBuffer& other)
    {
        if (this != &other)
        {
            uint8_t* new_data = new uint8_t[other.m_Size];
            std::copy(other.m_Data, other.m_Data + other.m_Size, new_data);
            delete[] m_Data;
            m_Data = new_data;
            m_Size = other.m_Size;
        }
        return *this;
    }

    // 4. Move constructor
    RawBuffer(RawBuffer&& other) noexcept
        : m_Size(std::exchange(other.m_Size, 0)),
          m_Data(std::exchange(other.m_Data, nullptr))
    {
    }

    // 5. Move assignment
    RawBuffer& operator=(RawBuffer&& other) noexcept
    {
        if (this != &other)
        {
            delete[] m_Data;
            m_Size = std::exchange(other.m_Size, 0);
            m_Data = std::exchange(other.m_Data, nullptr);
        }
        return *this;
    }

private:
    size_t   m_Size;
    uint8_t* m_Data;
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
    Timeout(int ms) : m_Ms(ms) { }

private:
    int m_Ms;
};

void Connect(Timeout t) { }

Connect(5000);    // ok, intentional
Connect(true);    // ❌ silently compiles — bool to int to Timeout!
Connect(3.7f);    // ❌ silently truncates float to int


class Timeout
{
public:
    // ✅ GOOD — explicit blocks accidental conversions
    explicit Timeout(int ms) : m_Ms(ms) { }

private:
    int m_Ms;
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
        : m_Host("localhost"), m_Port(8080), m_TimeoutMs(5000)
    {
        InitLogger();
        InitMetrics();
    }

    Server(std::string host, int port)
        : m_Host(std::move(host)), m_Port(port), m_TimeoutMs(5000)
    {
        InitLogger();    // duplicated
        InitMetrics();   // duplicated
    }

private:
    std::string m_Host;
    int         m_Port;
    int         m_TimeoutMs;
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
        : m_Host(std::move(host)),
          m_Port(port),
          m_TimeoutMs(timeout_ms)
    {
        InitLogger();    // runs once, from one place
        InitMetrics();
    }

private:
    std::string m_Host;
    int         m_Port;
    int         m_TimeoutMs;
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
        : m_Name(std::move(name))   // move into member — zero copy
    {
    }

    // ✅ for large objects, forward perfectly
    template<typename T>
    void RegisterHandler(T&& handler)
    {
        m_Handlers.emplace_back(std::forward<T>(handler));
    }

private:
    std::string                          m_Name;
    std::vector<std::function<void()>>   m_Handlers;
};

// ✅ use std::exchange in move constructor for clean state reset
class Socket
{
public:
    Socket(Socket&& other) noexcept
        : m_Fd(std::exchange(other.m_Fd, -1))   // steal fd, leave other in defined state
    {
    }

private:
    int m_Fd = -1;
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
        : m_X(other.m_X), m_Y(other.m_Y), m_Z(other.m_Z)
    {
    }

    // ✅ destructor is implicitly noexcept — be explicit anyway
    ~Vec3() noexcept = default;

    // ✅ trivial operations — mark noexcept for inlining hints
    float Length() const noexcept
    {
        return std::sqrt(m_X * m_X + m_Y * m_Y + m_Z * m_Z);
    }

private:
    float m_X, m_Y, m_Z;
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
    explicit AudioEngine(Config cfg)
        : m_Config(std::move(cfg))
    {
    }

    Config m_Config;
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
