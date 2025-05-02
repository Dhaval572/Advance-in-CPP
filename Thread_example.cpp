// Thread example in c++
#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

mutex cout_mutex; // Protect cout from simultaneous access

void threadExample(int n)
{
    lock_guard<mutex> guard(cout_mutex);
    cout << "Hello thread! Received number: " << n << "\n";
}

void printTableUsingThread(int n)
{
    lock_guard<mutex> guard(cout_mutex);
    for (size_t i = 1; i <= 10; i++)
    {
        cout << n << " x " << i << " = " << n * i << "\n";
    }
}

int main()
{
    thread t1(threadExample, 42); 
    thread t2(printTableUsingThread, 25);

    t1.join();
    t2.join();

    return 0;
}
