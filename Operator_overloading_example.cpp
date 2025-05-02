// Example of operator overloading
#include <iostream>
using namespace std;

class Vector
{
public:
    int x, y;

    Vector operator+(const Vector &other)
    {
        return Vector(x + other.x, y + other.y);
    }
};

int
main()
{
    Vector v1(2, 3), v2(4, 5);
    Vector v3 = v1 + v2;

    cout << "v3:\nx : " << v3.x << "\ny : " << v3.y << '\n';

    return 0;
}
