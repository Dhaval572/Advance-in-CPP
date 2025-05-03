// Variadic example in c++
#include <iostream>
using namespace std;

// Works as base case
void print(){ cout << '\n'; }

template <typename T, typename... Args>
void print(T first, Args... rest)
{
    cout << first << " ";
    print(rest...);
}

int main()
{
    print(324,23.3344f,'g', "Xyz");
    return 0;
}
