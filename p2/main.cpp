#include "Array.h"
#include "List.h"
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


void ListTest()
{
	TDoubleLinkedList<int> list1;
	auto node1 = list1.AddTail(1);
	auto node2 = list1.AddTail(2);
	auto node3 = list1.InsertNode(3, node2);
	auto ret   = list1.FindNode(2);
	list1.RemoveNode(node2, true);

	for (auto It = TDoubleLinkedList<int>::TConstIterator(list1.GetHead()); It; It++)
	{
		std::cout << *It << std::endl;
	}

	list1.Empty();
}

int main()
{
	ArrayTest();
	ListTest();
}