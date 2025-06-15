// Unique pointer example
#include <iostream>
#include <memory>

class MyClass
{
public:
    MyClass(int x) : val(x) 
    { 
        std::cout << "Constructed with value " << val << "\n"; 
    }

    ~MyClass() 
    { 
        std::cout << "Destructed\n"; 
    }

    void show() 
    { 
        std::cout << "Value: " << val << "\n"; 
    }

private:
    int val;
};

int main()
{
    std::unique_ptr<MyClass> ptr = std::make_unique<MyClass>(42);
    ptr->show();

    // Transferring ownership
    std::unique_ptr<MyClass> ptr2 = std::move(ptr);

    if (!ptr)
    {
        std::cout << "ptr is now empty.\n";
    }

    ptr2->show();

    return 0;
}
