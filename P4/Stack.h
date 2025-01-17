#pragma once

#include <stack>
#include <type_traits>
#include <functional>

class StackInfo
{
	template<typename T, const void(T::* FuncPtr)(StackInfo&)>
	struct StackMemberFuncWrapper
	{
		static bool Invoke(StackInfo& L)
		{
			T* Inst = nullptr;
			if (!L.Pop<T*>(Inst) || !Inst) return false;
			if (!FuncPtr)				   return false;
			std::invoke(FuncPtr, Inst, L); return true;
		}
	};

public:
	template<typename T>
	void Push(T Val)
	{
		Data.push(new T(Val));
	}

	template<typename T>
	void Push(const void(T::* Val)(StackInfo&))
	{
		auto FuncPtr = &StackMemberFuncWrapper<T, Val>::Invoke;
		auto FuncPtrCallMember = new decltype(FuncPtr);
		*FuncPtrCallMember = FuncPtr;
		Data.push(FuncPtrCallMember);
	}

	 void Push(const void(*Val)(StackInfo&))
	 {
		  auto FuncPtrCallMember = new decltype(Val);
		  *FuncPtrCallMember = Val;
		  Data.push(FuncPtrCallMember);
	 }

	template<typename T>
	bool Pop(T& Val)
	{
		if (Data.empty()) return false;
		T* ValPtr = (T*)Data.top();
		Val = *ValPtr;
		Data.pop();
		delete ValPtr;
		return true;
	}

	bool Call()
	{
		using FuncType = bool(*)(StackInfo&);
		FuncType FuncPtr = nullptr;
		if (!Pop<FuncType>(FuncPtr) || !FuncPtr) return false;
		return FuncPtr(*this);
	}

private:
	std::stack<void*> Data;
};
