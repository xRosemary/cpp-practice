#include <iostream>
#include "CircularBuffer.h"

#define ENABLE_CLASS_PRINT(expr) 

class A
{
public:
    A() : m_a(0)
    {
        ENABLE_CLASS_PRINT(std::cout << "A()" << std::endl);
    }
    A(int a) : m_a(a)
    {
        ENABLE_CLASS_PRINT(std::cout << "A()" << a << std::endl);
    }
    ~A()
    {
        ENABLE_CLASS_PRINT(std::cout << "~A()" << std::endl);
    }
    A(const A& Other)
    {
        ENABLE_CLASS_PRINT(std::cout << "A(const A& Other)" << Other.m_a << std::endl);
        this->m_a = Other.m_a;
    }
    A(A&& Other) noexcept
    {
        ENABLE_CLASS_PRINT(std::cout << "A(A&& Other)" << Other.m_a << std::endl);
        this->m_a = Other.m_a;
    }
    A& operator=(const A& Other)
    {
        ENABLE_CLASS_PRINT(std::cout << "operator=(const A& Other)" << Other.m_a << std::endl);
        this->m_a = Other.m_a;
        return *this;
    }
    A& operator=(A&& Other) noexcept
    {
        ENABLE_CLASS_PRINT(std::cout << "operator=(const A& Other)" << Other.m_a << std::endl);
        this->m_a = Other.m_a;
        return *this;
    }
    bool operator==(const A& other) { return true; }
    int m_a;
};

void Test()
{
    CircularBuffer<int> cb;

    cb.Push(1);
    cb.Push(2);
    cb.Push(3);
    cb.Push(1);
    cb.Push(2);
    cb.Push(3);
    cb.Push(1);
    cb.Push(2);
    cb.Push(3);

    for (int i = 0; i < 5; i++)
    {
        std::cout
            << "Size: " << cb.GetSize()
            << " \tPopped: " << cb.Pop()
            << " \tCapacity: " << cb.GetCapacity()
            << std::endl;
    }

    cb.Push(2);
    cb.Push(3);
    cb.Push(2);
    cb.Push(3);
    cb.Push(2);
    cb.Push(3);

    while (!cb.IsEmpty())
    {
        std::cout
            << "Size: " << cb.GetSize()
            << " \tPopped: " << cb.Pop()
            << " \tCapacity: " << cb.GetCapacity()
            << std::endl;
    }
}

void Test2()
{
    CircularBuffer<A> cb;

    cb.Push(1);
    cb.Push(2);
    cb.Push(3);
    cb.Push(1);
    cb.Push(2);
    cb.Push(3);
    cb.Push(1);
    cb.Push(2);
    cb.Push(3);

    for (int i = 0; i < 5; i++)
    {
        std::cout
            << "Size: " << cb.GetSize()
            << " \tPopped: " << cb.Pop().m_a
            << " \tCapacity: " << cb.GetCapacity()
            << std::endl;
    }

    cb.Push(2);
    cb.Push(3);
    cb.Push(2);
    cb.Push(3);
    cb.Push(2);
    cb.Push(3);

    while (!cb.IsEmpty())
    {
        std::cout
            << "Size: " << cb.GetSize()
            << " \tPopped: " << cb.Pop().m_a
            << " \tCapacity: " << cb.GetCapacity()
            << std::endl;
    }
}

int main()
{
    Test();
    std::cout << "--------------\n";
    Test2();
}