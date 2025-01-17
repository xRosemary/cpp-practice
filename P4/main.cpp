#include <iostream>
#include "ConstStack.h"
#include "Stack.h"

class A
{
public:
    void Func(StackInfo& L)
    {
        std::cout << __func__ << " " << __LINE__ << std::endl;
    }
};

static void StaticFunc(StackInfo& L)
{
    std::cout << __func__ << " " << __LINE__ << std::endl;
}

void TestConstStack()
{
    constexpr auto container = Create(true, "abc", 3);
    constexpr auto newContainer = Push(123.5f, container);

    std::cout << std::get<0>(newContainer) << std::endl;
    std::cout << std::get<1>(newContainer) << std::endl;
    std::cout << std::get<2>(newContainer) << std::endl;
    std::cout << std::get<3>(newContainer) << std::endl;

    std::cout << "-----------------------" << std::endl;

    constexpr auto popContainer = Pop(newContainer);
    std::cout << std::get<0>(popContainer) << std::endl;
    std::cout << std::get<1>(popContainer) << std::endl;
    std::cout << std::get<2>(popContainer) << std::endl;

    std::cout << "-----------------------" << std::endl;

    constexpr auto popContainer2 = Pop(popContainer);
    std::cout << std::get<0>(popContainer2) << std::endl;
    std::cout << std::get<1>(popContainer2) << std::endl;

    std::cout << "-----------------------" << std::endl;

    constexpr auto popContainer3 = Pop(popContainer2);
    std::cout << std::get<0>(popContainer3) << std::endl;
}

void TestStack()
{
    StackInfo Info;
    Info.Push(A());
    Info.Push(&A::Func);
    Info.Call();
    Info.Push(&StaticFunc);
    Info.Call();

    int a = 1;
    Info.Push(a);
    int b = 0;
    Info.Pop(b);
    std::cout << b << std::endl;
}

int main()
{
    TestStack();
    TestConstStack();
    return 0;
}