// Function Pointer
#include <iostream>
using namespace std;

void hello() { cout << "Hello"; }

int main()
{
    void (*funcPtr)() = hello;
    funcPtr(); 
    return 0;
}

/*
    Function pointers is used to call the function dynamically.

    Function pointers allow passing functions as arguments to other functions
*/
