#pragma once
#include <typeinfo>
#include "Util.h"

#ifndef RESTRICT
	#define RESTRICT __restrict						/* no alias hint */
#endif

template<typename InElementType, typename SizeType = int, SizeType DefaultAllocSize = 4>
class TArray
{
public:
	typedef InElementType ElementType;
    inline const static int INDEX_NONE = -1;

public:
	TArray()
		: ArrayNum(0)
		, ArrayMax(0)
	{
		ResizeTo(DefaultAllocSize);
	}

	TArray(const TArray& Other)
	{
		CopyToEmpty(Other.GetData(), Other.Num(), 0);
	}
    
	TArray& operator=(const TArray<ElementType>& Other)
	{
		DestructItems(GetData(), ArrayNum);
		CopyToEmpty(Other.GetData(), Other.Num(), ArrayMax);
		return *this;
	}

    TArray(TArray&& Other)
	{
		MoveOrCopy(*this, Other, 0);
	}

    TArray& operator=(TArray&& Other)
	{
		if (this != &Other)
		{
			DestructItems(GetData(), ArrayNum);
			MoveOrCopy(*this, Other, ArrayMax);
		}
		return *this;
	}

    virtual ~TArray()
	{
		DestructItems(GetData(), ArrayNum);
	}

private:
	template <typename FromArrayType, typename ToArrayType>
	static void MoveOrCopy(ToArrayType& ToArray, FromArrayType& FromArray, SizeType PrevMax)
	{
		if constexpr (CanMoveTArrayPointersBetweenArrayTypes<FromArrayType, ToArrayType>())
		{
			// Move
			static_assert(std::is_same_v<TArray, ToArrayType>, "MoveOrCopy is expected to be called with the current array type as the destination");
			ToArray  .ArrayData = FromArray.ArrayData;
			ToArray  .ArrayNum  = FromArray.ArrayNum;
			ToArray  .ArrayMax  = FromArray.ArrayMax;
			FromArray.ArrayData = nullptr;
			FromArray.ArrayNum  = 0;
			FromArray.ArrayMax  = DefaultAllocSize;
		}
		else
		{
			// Copy
			ToArray.CopyToEmpty(FromArray.GetData(), FromArray.Num(), PrevMax);
		}
	}

    template <typename T>
    typename std::remove_reference<T>::type&& MoveTempIfPossible(T&& Obj)
    {
        typedef typename std::remove_reference<T>::type CastType;
        return (CastType&&)Obj;
    }

public:

	ElementType* GetData()
	{
		return (ElementType*)ArrayData;
	}

	const ElementType* GetData() const
	{
		return (const ElementType*)ArrayData;
	}

	static constexpr std::uint32_t GetTypeSize()
	{
		return sizeof(ElementType);
	}

	SizeType GetAllocatedSize(void) const
	{
		return ArrayMax * sizeof(ElementType);
	}

	SizeType GetSlack() const
	{
		return ArrayMax - ArrayNum;
	}

	bool IsEmpty() const
	{
		return ArrayNum == 0;
	}

	SizeType Num() const
	{
		return ArrayNum;
	}

	SizeType Max() const
	{
		return ArrayMax;
	}

	ElementType& operator[](SizeType Index)
	{
		return GetData()[Index];
	}

	const ElementType& operator[](SizeType Index) const
	{
		return GetData()[Index];
	}

	ElementType Pop(bool bAllowShrinking = true)
	{
		ElementType Result = MoveTempIfPossible(GetData()[ArrayNum - 1]);
		RemoveAt(ArrayNum - 1, 1, bAllowShrinking);
		return Result;
	}

	void Push(ElementType&& Item)
	{
		Add(MoveTempIfPossible(Item));
	}

	void Push(const ElementType& Item)
	{
		Add(Item);
	}

	ElementType& Top()
	{
		return Last();
	}

	const ElementType& Top() const
	{
		return Last();
	}

	ElementType& Last(SizeType IndexFromTheEnd = 0)
	{
		return GetData()[ArrayNum - IndexFromTheEnd - 1];
	}

	const ElementType& Last(SizeType IndexFromTheEnd = 0) const
	{
		return GetData()[ArrayNum - IndexFromTheEnd - 1];
	}

	void Shrink()
	{
		if (ArrayMax != ArrayNum)
		{
			ResizeTo(ArrayNum);
		}
	}

	bool Find(const ElementType& Item, SizeType& Index) const
	{
		Index = this->Find(Item);
		return Index != INDEX_NONE;
	}

	SizeType Find(const ElementType& Item) const
	{
		const ElementType* RESTRICT Start = GetData();
		for (const ElementType* RESTRICT Data = Start, *RESTRICT DataEnd = Data + ArrayNum; Data != DataEnd; ++Data)
		{
			if (*Data == Item)
			{
				return static_cast<SizeType>(Data - Start);
			}
		}
		return INDEX_NONE;
	}

	bool FindLast(const ElementType& Item, SizeType& Index) const
	{
		Index = this->FindLast(Item);
		return Index != INDEX_NONE;
	}

	SizeType FindLast(const ElementType& Item) const
	{
		for (const ElementType* RESTRICT Start = GetData(), *RESTRICT Data = Start + ArrayNum; Data != Start; )
		{
			--Data;
			if (*Data == Item)
			{
				return static_cast<SizeType>(Data - Start);
			}
		}
		return INDEX_NONE;
	}

	template <typename KeyType>
	const ElementType* FindByKey(const KeyType& Key) const
	{
		return const_cast<TArray*>(this)->FindByKey(Key);
	}

	template <typename KeyType>
	ElementType* FindByKey(const KeyType& Key)
	{
		for (ElementType* RESTRICT Data = GetData(), *RESTRICT DataEnd = Data + ArrayNum; Data != DataEnd; ++Data)
		{
			if (*Data == Key)
			{
				return Data;
			}
		}

		return nullptr;
	}

	bool operator==(const TArray& OtherArray) const
	{
		SizeType Count = Num();

		return Count == OtherArray.Num() && CompareItems(GetData(), OtherArray.GetData(), Count);
	}
	
	SizeType AddUninitialized()
	{
		const SizeType OldNum = ArrayNum;
		const SizeType NewNum = OldNum + 1;
		ArrayNum = NewNum;
		if (NewNum > ArrayMax)
		{
			ResizeGrow(OldNum);
		}
		return OldNum;
	}

	SizeType AddUninitialized(SizeType Count)
	{
		const SizeType OldNum = ArrayNum;
		const SizeType NewNum = OldNum + Count;
		ArrayNum = NewNum;

		if (Count > ArrayMax - OldNum)
		{
			ResizeGrow(OldNum);
		}

		return OldNum;
	}

private:
	void InsertUninitializedImpl(SizeType Index)
	{
		const SizeType OldNum = ArrayNum;
		const SizeType NewNum = OldNum + 1;
		ArrayNum = NewNum;

		if (NewNum > ArrayMax)
		{
			ResizeGrow(OldNum);
		}
		ElementType* Data = GetData() + Index;
		RelocateConstructItems<ElementType>(Data + 1, Data, OldNum - Index);
	}

	template <typename OtherSizeType>
	void InsertUninitializedImpl(SizeType Index, OtherSizeType Count)
	{
		
		_ASSERT((Count >= 0) & (Index >= 0) & (Index <= ArrayNum));

		SizeType ConvertedCount = Count;
		_ASSERT_EXPR((OtherSizeType)ConvertedCount == Count, "Invalid number of elements to add to this array type");

		const SizeType OldNum = ArrayNum;
		const SizeType NewNum = OldNum + Count;
		ArrayNum = NewNum;

		if (Count > ArrayMax - OldNum)
		{
			ResizeGrow(OldNum);
		}

		ElementType* Data = GetData() + Index;
		RelocateConstructItems<ElementType>(Data + Count, Data, OldNum - Index);
	}

public:
	void InsertUninitialized(SizeType Index)
	{
		InsertUninitializedImpl(Index);
	}

	void InsertUninitialized(SizeType Index, SizeType Count)
	{
		InsertUninitializedImpl(Index, Count);
	}

	void InsertZeroed(SizeType Index)
	{
		InsertUninitializedImpl(Index);
		Memzero(GetData() + Index, sizeof(ElementType));
	}
	
	void InsertZeroed(SizeType Index, SizeType Count)
	{
		InsertUninitializedImpl(Index, Count);
		Memzero(GetData() + Index, Count * sizeof(ElementType));
	}
	
	ElementType& InsertZeroed_GetRef(SizeType Index)
	{
		InsertUninitializedImpl(Index, 1);
		ElementType* Ptr = GetData() + Index;
		Memzero(Ptr, sizeof(ElementType));
		return *Ptr;
	}
	
	SizeType Insert(std::initializer_list<ElementType> InitList, const SizeType InIndex)
	{
		SizeType NumNewElements = InitList.size();

		InsertUninitializedImpl(InIndex, NumNewElements);
		ConstructItems<ElementType>(GetData() + InIndex, InitList.begin(), NumNewElements);

		return InIndex;
	}

	template <typename OtherAllocator>
	SizeType Insert(const TArray<ElementType, OtherAllocator>& Items, const SizeType InIndex)
	{
		_ASSERT((const void*)this != (const void*)&Items);

		auto NumNewElements = Items.Num();

		InsertUninitializedImpl(InIndex, NumNewElements);
		ConstructItems<ElementType>(GetData() + InIndex, Items.GetData(), NumNewElements);

		return InIndex;
	}
	
	template <typename OtherAllocator>
	SizeType Insert(TArray<ElementType, OtherAllocator>&& Items, const SizeType InIndex)
	{
		_ASSERT((const void*)this != (const void*)&Items);

		auto NumNewElements = Items.Num();

		InsertUninitializedImpl(InIndex, NumNewElements);
		RelocateConstructItems<ElementType>(GetData() + InIndex, Items.GetData(), NumNewElements);
		Items.ArrayNum = 0;

		return InIndex;
	}

	SizeType Insert(const ElementType* Ptr, SizeType Count, SizeType Index)
	{
		_ASSERT(Ptr != nullptr);

		InsertUninitializedImpl(Index, Count);
		ConstructItems<ElementType>(GetData() + Index, Ptr, Count);

		return Index;
	}

	SizeType Insert(ElementType&& Item, SizeType Index)
	{
		// construct a copy in place at Index (this new operator will insert at 
		// Index, then construct that memory with Item)
		InsertUninitializedImpl(Index);
		new(GetData() + Index) ElementType(MoveTempIfPossible(Item));
		return Index;
	}

	SizeType Insert(const ElementType& Item, SizeType Index)
	{
		// construct a copy in place at Index (this new operator will insert at 
		// Index, then construct that memory with Item)
		InsertUninitializedImpl(Index);
		new(GetData() + Index) ElementType(Item);
		return Index;
	}

private:
	void RemoveAtImpl(SizeType Index, SizeType Count, bool bAllowShrinking)
	{
		if (Count)
		{
			
			_ASSERT((Count >= 0) & (Index >= 0) & (Index + Count <= ArrayNum));

			DestructItems(GetData() + Index, Count);

			// Skip memmove in the common case that there is nothing to move.
			SizeType NumToMove = ArrayNum - Index - Count;
			if (NumToMove)
			{
				memmove
				(
					(std::uint8_t*)GetData() + (Index)* sizeof(ElementType),
					(std::uint8_t*)GetData() + (Index + Count) * sizeof(ElementType),
					NumToMove * sizeof(ElementType)
				);
			}
			ArrayNum -= Count;

			if (bAllowShrinking)
			{
				ResizeShrink();
			}
		}
	}

public:
	void RemoveAt(SizeType Index)
	{
		RemoveAtImpl(Index, 1, true);
	}

	template <typename CountType>
	void RemoveAt(SizeType Index, CountType Count, bool bAllowShrinking = true)
	{
		static_assert(!std::is_same_v<CountType, bool>, "TArray::RemoveAt: unexpected bool passed as the Count argument");
		RemoveAtImpl(Index, Count, bAllowShrinking);
	}

	void Reset(SizeType NewSize = 0)
	{
		// If we have space to hold the excepted size, then don't reallocate
		if (NewSize <= ArrayMax)
		{
			DestructItems(GetData(), ArrayNum);
			ArrayNum = 0;
		}
		else
		{
			Empty(NewSize);
		}
	}

	void Empty(SizeType Slack = 0)
	{
		DestructItems(GetData(), ArrayNum);

		_ASSERT(Slack >= 0);
		ArrayNum = 0;

		if (ArrayMax != Slack)
		{
			ResizeTo(Slack);
		}
	}

	template <typename... ArgsType>
	SizeType Emplace(ArgsType&&... Args)
	{
		const SizeType Index = AddUninitialized();
		new(GetData() + Index) ElementType(std::forward<ArgsType>(Args)...);
		return Index;
	}

	template <typename... ArgsType>
	void EmplaceAt(SizeType Index, ArgsType&&... Args)
	{
		InsertUninitializedImpl(Index, 1);
		new(GetData() + Index) ElementType(std::forward<ArgsType>(Args)...);
	}

	SizeType Add(ElementType&& Item)
	{
		return Emplace(MoveTempIfPossible(Item));
	}

	SizeType Add(const ElementType& Item)
	{
		return Emplace(Item);
	}

	SizeType AddZeroed()
	{
		const SizeType Index = AddUninitialized();
		Memzero((std::uint8_t*)GetData() + Index * sizeof(ElementType), sizeof(ElementType));
		return Index;
	}
	
	SizeType AddZeroed(SizeType Count)
	{
		const SizeType Index = AddUninitialized(Count);
		Memzero((std::uint8_t*)GetData() + Index*sizeof(ElementType), Count*sizeof(ElementType));
		return Index;
	}

public:
	/**
	 * Reserves memory such that the array can contain at least Number elements.
	 *
	 * @param Number The number of elements that the array should be able to contain after allocation.
	 * @see Shrink
	 */
	void Reserve(SizeType Number)
	{
		_ASSERT(Number >= 0);
		if (Number > ArrayMax)
		{
			ResizeTo(Number);
		}
	}

	/**
	 * Removes the first occurrence of the specified item in the array,
	 * maintaining order but not indices.
	 *
	 * @param Item The item to remove.
	 * @returns The number of items removed. For RemoveSingleItem, this is always either 0 or 1.
	 * @see Add, Insert, Remove, RemoveAll, RemoveAllSwap
	 */
	SizeType RemoveSingle(const ElementType& Item)
	{
		SizeType Index = Find(Item);
		if (Index == INDEX_NONE)
		{
			return 0;
		}

		auto* RemovePtr = GetData() + Index;

		// Destruct items that match the specified Item.
		DestructItems(RemovePtr, 1);
		const SizeType NextIndex = Index + 1;
		RelocateConstructItems<ElementType>(RemovePtr, RemovePtr + 1, ArrayNum - (Index + 1));

		// Update the array count
		--ArrayNum;

		// Removed one item
		return 1;
	}

	/**
	 * Removes as many instances of Item as there are in the array, maintaining
	 * order but not indices.
	 *
	 * @param Item Item to remove from array.
	 * @returns Number of removed elements.
	 * @see Add, Insert, RemoveAll, RemoveAllSwap, RemoveSingle, RemoveSwap
	 */
	SizeType Remove(const ElementType& Item)
	{
		// Element is non-const to preserve compatibility with existing code with a non-const operator==() member function
		return RemoveAll([&Item](ElementType& Element) { return Element == Item; });
	}

	/**
	 * Remove all instances that match the predicate, maintaining order but not indices
	 * Optimized to work with runs of matches/non-matches
	 *
	 * @param Predicate Predicate class instance
	 * @returns Number of removed elements.
	 * @see Add, Insert, RemoveAllSwap, RemoveSingle, RemoveSwap
	 */
	template <class PREDICATE_CLASS>
	SizeType RemoveAll(const PREDICATE_CLASS& Predicate)
	{
		const SizeType OriginalNum = ArrayNum;
		if (!OriginalNum)
		{
			return 0; // nothing to do, loop assumes one item so need to deal with this edge case here
		}

		SizeType WriteIndex = 0;
		SizeType ReadIndex = 0;
		bool NotMatch = !Invoke(Predicate, GetData()[ReadIndex]); // use a ! to guarantee it can't be anything other than zero or one
		do
		{
			SizeType RunStartIndex = ReadIndex++;
			while (ReadIndex < OriginalNum && NotMatch == !Invoke(Predicate, GetData()[ReadIndex]))
			{
				ReadIndex++;
			}
			SizeType RunLength = ReadIndex - RunStartIndex;
			_ASSERT(RunLength > 0);
			if (NotMatch)
			{
				// this was a non-matching run, we need to move it
				if (WriteIndex != RunStartIndex)
				{
					memmove(&GetData()[WriteIndex], &GetData()[RunStartIndex], sizeof(ElementType)* RunLength);
				}
				WriteIndex += RunLength;
			}
			else
			{
				// this was a matching run, delete it
				DestructItems(GetData() + RunStartIndex, RunLength);
			}
			NotMatch = !NotMatch;
		} while (ReadIndex < OriginalNum);

		ArrayNum = WriteIndex;
		return OriginalNum - ArrayNum;
	}

private:
	void AllocatorResizeAllocation(SizeType CurrentArrayNum, SizeType NewArrayMax)
	{
		ResizeAllocation(ArrayData, CurrentArrayNum, NewArrayMax, sizeof(ElementType));
	}

	SizeType AllocatorCalculateSlackShrink(SizeType CurrentArrayNum, SizeType NewArrayMax)
	{
		return DefaultCalculateSlackShrink(CurrentArrayNum, NewArrayMax, sizeof(ElementType));
	}

	SizeType AllocatorCalculateSlackGrow(SizeType CurrentArrayNum, SizeType NewArrayMax)
	{
		return DefaultCalculateSlackGrow(CurrentArrayNum, NewArrayMax, sizeof(ElementType));
	}

	SizeType AllocatorCalculateSlackReserve(SizeType NewArrayMax)
	{
		return DefaultCalculateSlackReserve(NewArrayMax, sizeof(ElementType));
	}

	void ResizeGrow(SizeType OldNum)
	{
		SizeType LocalArrayNum = ArrayNum;
		ArrayMax = AllocatorCalculateSlackGrow(LocalArrayNum, ArrayMax);
		AllocatorResizeAllocation(OldNum, ArrayMax);
	}
	void ResizeShrink()
	{
		const SizeType NewArrayMax = AllocatorCalculateSlackShrink(ArrayNum, ArrayMax);
		if (NewArrayMax != ArrayMax)
		{
			ArrayMax = NewArrayMax;
			_ASSERT(ArrayMax >= ArrayNum);
			AllocatorResizeAllocation(ArrayNum, ArrayMax);
		}
	}
	void ResizeTo(SizeType NewMax)
	{
		if (NewMax)
		{
			NewMax = AllocatorCalculateSlackReserve(NewMax);
		}
		if (NewMax != ArrayMax)
		{
			ArrayMax = NewMax;
			AllocatorResizeAllocation(ArrayNum, ArrayMax);
		}
	}
	void ResizeForCopy(SizeType NewMax, SizeType PrevMax)
	{
		if (NewMax)
		{
			NewMax = AllocatorCalculateSlackReserve(NewMax);
		}
		if (NewMax > PrevMax)
		{
			AllocatorResizeAllocation(0, NewMax);
			ArrayMax = NewMax;
		}
		else
		{
			ArrayMax = PrevMax;
		}
	}


	/**
	 * Copies data from one array into this array. Uses the fast path if the
	 * data in question does not need a constructor.
	 *
	 * @param Source The source array to copy
	 * @param PrevMax The previous allocated size
	 */
	template <typename OtherElementType, typename OtherSizeType>
	void CopyToEmpty(const OtherElementType* OtherData, OtherSizeType OtherNum, SizeType PrevMax)
	{
		SizeType NewNum = OtherNum;
		ArrayNum = NewNum;
		if (OtherNum || PrevMax)
		{
			ResizeForCopy(NewNum, PrevMax);
			ConstructItems<ElementType>(GetData(), OtherData, OtherNum);
		}
		else
		{
			ArrayMax = DefaultAllocSize;
		}
	}

protected:
	void*                ArrayData;
	SizeType             ArrayNum;
	SizeType             ArrayMax;
};