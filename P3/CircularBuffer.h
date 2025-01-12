#pragma once

#include "../p2/Array.h"

template<typename T, int DefaultSize = 4>
class CircularBuffer
{
	using ElementType = T;
	static_assert(DefaultSize > 0, "buffer size must be greater than 0");

public:
	CircularBuffer() : StartIndex(0), EndIndex(0), ElementNum(0)
	{
		Data.AddUninitialized(DefaultSize);
	}

	int GetCapacity() const { return Data.Num(); }

	int GetSize() const { return ElementNum; }

	bool IsFull() { return ElementNum == Data.Num(); }

	bool IsEmpty() { return ElementNum == 0; }

	void Push(const ElementType& Item)
	{
		PushImpl(Item);
	}

	void Push(ElementType&& Item)
	{
		PushImpl(MoveTempIfPossible(Item));
	}

	ElementType Pop()
	{
		_ASSERT(!IsEmpty());

		// 如果本次Pop会导致缓冲区空余空间过大，则先考虑收缩大小
		if (GetSize() <= Data.Num() / 2)
		{
			TryShrink();
		}

		ElementType Item = MoveTempIfPossible(Data[StartIndex]);
		DestructItem(&Data[StartIndex]);
		StartIndex = IncreaseIndex(StartIndex);
		ElementNum--;
		return Item;
	}

private:
	int IncreaseIndex(int Index)
	{
		if (Index + 1 < Data.Num())
		{
			return Index + 1;
		}

		_ASSERT(Index + 1 - Data.Num() == 0);
		return 0;
	}

	// 扩容并将元素重新排布
	void TryExpandSize()
	{
		if (GetSize() < Data.Num())
		{
			return;
		}

		if (StartIndex < EndIndex) // 直接2倍扩容
		{
			Data.AddUninitialized(Data.Num());
			return;
		}

		// 如果出现翻转，则将StartIndex到数组末尾的元素放到新的末尾
		Data.AddUninitialized(Data.Num()); // 2倍扩容

		ElementType* RawData = Data.GetData() + ElementNum - 1;
		ElementType* NewRawData = Data.GetData() + Data.Num() - 1;
		for (int i = StartIndex; i < ElementNum; i++)
		{
			new (NewRawData) ElementType(*RawData);
			DestructItem(RawData);
			NewRawData--;
			RawData--;
		}

		StartIndex += ElementNum;
	}

	// 收缩大小并将元素重新排布
	void TryShrink()
	{
		if (GetSize() > Data.Num() / 2 || Data.Num() <= DefaultSize)
		{
			return;
		}

		// 如果元素都在前半段则直接移除后面一半
		if (StartIndex < Data.Num() / 2 && EndIndex < Data.Num() / 2)
		{
			Data.RemoveAt(Data.Num() / 2, Data.Num() - Data.Num() / 2);
			return;
		}

		TArray<T> NewData;
		NewData.AddUninitialized(GetSize());
		for (int i = 0; i < NewData.Num(); i++)
		{
			NewData[i] = MoveTempIfPossible(Data[StartIndex]);
			StartIndex = IncreaseIndex(StartIndex);
		}

		StartIndex = 0;
		EndIndex = NewData.Num() - 1;
		Data = MoveTempIfPossible(NewData);
	}

	template<typename ArgType>
	void PushImpl(ArgType&& Item)
	{
		// 如果本次Push会导致缓冲区填满，则先考虑扩容
		TryExpandSize();
		Data[EndIndex] = Item;
		EndIndex = IncreaseIndex(EndIndex);
		ElementNum++;
	}

private:
	TArray<T> Data;
	int StartIndex;
	int EndIndex;
	int ElementNum;
};