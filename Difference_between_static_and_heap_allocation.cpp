/*
Difference between static allocation and heap allocation
	+----------------+-------------------------------+------------------------------+
	| 	   FEATURE   |       Static allocation       |      Dynamic Allocation      |
	+----------------+-------------------------------+------------------------------+
	|Allocation Time | Allocated at compile time  	 | Allocated at runtime         |
	+----------------+-------------------------------+------------------------------+
	| Memory Location| Stored in the stack or data   | Stored in the heap           |
	|                | segment                       |                              |
	+----------------+-------------------------------+------------------------------+
	| Size           | Fixed and determined at       | Dynamic size (can grow or    |
	|                | compile time                  | shrink during runtime)       |
	+----------------+-------------------------------+------------------------------+
	| Access Speed   | Faster (due to stack memory   | Slower (requires heap memory |
	|                | management)                   | management and pointer deref)|
	+----------------+-------------------------------+------------------------------+
	| Lifetime       | Exists until the variable     | Exists until manually freed  |
	|                | goes out of scope or the      | (using delete or free)       |
	|                | program ends                  |                              |
	+----------------+-------------------------------+------------------------------+
	| Memory         | Automatically managed by the  | Must be manually managed     |
	| Management     | compiler/OS                   | by the programmer (using new,|
	|                |                               | malloc, delete, free)        |
	+----------------+-------------------------------+------------------------------+
	| Example        | Local variables, global       | Dynamically allocated memory |
	|                | variables                     | (e.g., new, malloc)          |
	+----------------+-------------------------------+------------------------------+
	| Fragmentation  | No fragmentation              | Can lead to fragmentation    |
	|                |                               | if not managed properly      |
	+----------------+-------------------------------+------------------------------+
	| Thread Safety  | Thread-safe for local         | Not thread-safe unless       |
	|                | variables (stack)             | synchronized or manually     |
	|                |                               | managed                      |
	+----------------+-------------------------------+------------------------------+
	| Allocation     | Limited by stack size or      | Limited by total available   |
	| Limit          | data segment                  | memory (RAM)                 |
	+----------------+-------------------------------+------------------------------+
	| Deallocation   | Automatically deallocated when| Must be manually deallocated |
	|                | out of scope                  | using delete or free         |
	+----------------+-------------------------------+------------------------------+
	| Example Func   | int x = 5;                    | int* ptr = new int;          |
	|                |                               | delete ptr;                  |
	+----------------+-------------------------------+------------------------------+
*/
