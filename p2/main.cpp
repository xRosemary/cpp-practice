#include "Array.h"
#include <iostream>

class A
{
public:
	A() { std::cout << "A()" << std::endl; }
	~A() { std::cout << "~A()" << std::endl; }
	A(const A& Other) { std::cout << "A(const A& Other)" << std::endl; }
	A(A&& Other) noexcept { std::cout << "A(A&& Other)" << std::endl; }
	A& operator=(const A& Other) { std::cout << "operator=(const A& Other)" << std::endl; }
	bool operator==(const A& other) { return true; }
};

void ArrayAddAndRemove()
{
	TArray<A> arr;
	A a_inst;

	arr.Add(a_inst);
	arr.Add(a_inst);
	arr.Add(a_inst);
	arr.Empty();
	arr.Add(a_inst);
	arr.Add(a_inst);
	arr.Add(a_inst);
	arr.Remove(a_inst);
}

void ArrayNormalTest()
{
	TArray<int> arr;
	arr.Reserve(8);
	arr.Add(2);
	arr.Add(1);
	std::cout << arr.Find(1) << std::endl;
	arr.Insert(3, 1);
	arr.Remove(1);
	arr.Pop();
	arr.Empty();
	std::cout << arr.Find(1) << std::endl;
}

void ArrayTest()
{
	ArrayAddAndRemove();
	ArrayNormalTest();
}

int main()
{
	ArrayTest();
}