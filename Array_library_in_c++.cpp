// Array In digital c++ 
#include <iostream>
#include <array>                  // array library including
using namespace std;

int main()
{
	array<int, 4> myArr = {34,35,67,78};

	// Traversing array using for each loop 
	for(const int &element : myArr) 
	{
		cout << element << ' ';
	}

	return 0;
}

/*
Explaination: 

	Syntax of initialize array using 'array':

		array<data_type, size> array_name = {
			Array_element1, Array_element1, Array_element1, ... Array_element(n)
		}

	Syntax of for-each loop: 
	
		for(const data_type, &variable_name : array_name)
		{
			statement to print
		}
		
Why we use this: 

	1. Type Safety: Ensures the array size is part of the type.

	2. Fixed Size: Size is known at compile time, preventing errors.

	3. STL Compatibility: Works seamlessly with STL algorithms.

	4. Cleaner Syntax: Supports range-based for loops.

	5. Automatic Size Calculation: Easily get the size with `.size()`.

	6. Better Initialization: Clearer syntax for initialization with initializer lists.

	7. Avoids Pointer Issues: Reduces the risk of pointer arithmetic errors.

*/
