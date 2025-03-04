// Function Pointer
#include <iostream>
using namespace std;

void hello() { cout << "Hello"; }
void (*funcPtr)() = hello;

int main()
{
    funcPtr(); 
    return 0;
}

/*
    Function pointers is used to call the function dynamically.

    Function pointers allow passing functions as arguments to other functions
*/
