#pragma once
#include <cstdint>
#include <type_traits>
#include <cstdlib>
#include <string>

/**
 * TIsReferenceType
 */
template<typename T> struct TIsReferenceType      { enum { Value = false }; };
template<typename T> struct TIsReferenceType<T&>  { enum { Value = true  }; };
template<typename T> struct TIsReferenceType<T&&> { enum { Value = true  }; };

/**
 * Tests if a type T is bitwise-constructible from a given argument type U.  That is, whether or not
 * the U can be memcpy'd in order to produce an instance of T, rather than having to go
 * via a constructor.
 *
 * Examples:
 * TIsBitwiseConstructible<PODType,    PODType   >::Value == true  // PODs can be trivially copied
 * TIsBitwiseConstructible<const int*, int*      >::Value == true  // a non-const Derived pointer is trivially copyable as a const Base pointer
 * TIsBitwiseConstructible<int*,       const int*>::Value == false // not legal the other way because it would be a const-correctness violation
 * TIsBitwiseConstructible<int32,      uint32    >::Value == true  // signed integers can be memcpy'd as unsigned integers
 * TIsBitwiseConstructible<uint32,     int32     >::Value == true  // and vice versa
 */

template <typename T, typename Arg>
struct TIsBitwiseConstructible
{
	static_assert(
		!TIsReferenceType<T  >::Value &&
		!TIsReferenceType<Arg>::Value,
		"TIsBitwiseConstructible is not designed to accept reference types");

	static_assert(
		std::is_same_v<T,   std::remove_cv_t<T  >> &&
		std::is_same_v<Arg, std::remove_cv_t<Arg>>,
		"TIsBitwiseConstructible is not designed to accept qualified types");

	// Assume no bitwise construction in general
	enum { Value = false };
};

template <typename T>
struct TIsTriviallyCopyConstructible
{
	enum { Value = std::is_trivially_copy_constructible_v<T> };
};

template <typename T>
struct TIsBitwiseConstructible<T, T>
{
	// Ts can always be bitwise constructed from itself if it is trivially copyable.
	enum { Value = TIsTriviallyCopyConstructible<T>::Value };
};

template <typename T, typename U>
struct TIsBitwiseConstructible<const T, U> : TIsBitwiseConstructible<T, U>
{
	// Constructing a const T is the same as constructing a T
};

// Const pointers can be bitwise constructed from non-const pointers.
// This is not true for pointer conversions in general, e.g. where an offset may need to be applied in the case
// of multiple inheritance, but there is no way of detecting that at compile-time.
template <typename T>
struct TIsBitwiseConstructible<const T*, T*>
{
	// Constructing a const T is the same as constructing a T
	enum { Value = true };
};

// Unsigned types can be bitwise converted to their signed equivalents, and vice versa.
// (assuming two's-complement, which we are)
template <> struct TIsBitwiseConstructible< std::uint8_t, std::int8_t>   { enum { Value = true }; };
template <> struct TIsBitwiseConstructible<  std::int8_t, std::uint8_t>  { enum { Value = true }; };
template <> struct TIsBitwiseConstructible<std::uint16_t, std::int16_t>  { enum { Value = true }; };
template <> struct TIsBitwiseConstructible< std::int16_t, std::uint16_t> { enum { Value = true }; };
template <> struct TIsBitwiseConstructible<std::uint32_t, std::int32_t>  { enum { Value = true }; };
template <> struct TIsBitwiseConstructible< std::int32_t, std::uint32_t> { enum { Value = true }; };
template <> struct TIsBitwiseConstructible<std::uint64_t, std::int64_t>  { enum { Value = true }; };
template <> struct TIsBitwiseConstructible< std::int64_t, std::uint64_t> { enum { Value = true }; };

template <typename DestinationElementType, typename SourceElementType, typename SizeType>
void ConstructItems(void* Dest, const SourceElementType* Source, SizeType Count)
{
	if constexpr (TIsBitwiseConstructible<DestinationElementType, SourceElementType>::Value)
	{
		if (Count)
		{
			memcpy(Dest, Source, sizeof(SourceElementType) * Count);
		}
	}
	else
	{
		while (Count)
		{
			new (Dest) DestinationElementType(*Source);
			++(DestinationElementType*&)Dest;
			++Source;
			--Count;
		}
	}
}

template <typename T>
struct TIsTriviallyDestructible
{
	enum { Value = std::is_trivially_destructible_v<T> };
};

/**
 * Destructs a single item in memory.
 *
 * @param	Elements	A pointer to the item to destruct.
 *
 * @note: This function is optimized for values of T, and so will not dynamically dispatch destructor calls if T's destructor is virtual.
 */
template <typename ElementType>
void DestructItem(ElementType* Element)
{
	if constexpr (!TIsTriviallyDestructible<ElementType>::Value)
	{
		// We need a typedef here because VC won't compile the destructor call below if ElementType itself has a member called ElementType
		typedef ElementType DestructItemsElementTypeTypedef;

		Element->DestructItemsElementTypeTypedef::~DestructItemsElementTypeTypedef();
	}
}

/**
 * Destructs a range of items in memory.
 *
 * @param	Elements	A pointer to the first item to destruct.
 * @param	Count		The number of elements to destruct.
 *
 * @note: This function is optimized for values of T, and so will not dynamically dispatch destructor calls if T's destructor is virtual.
 */
template <typename ElementType, typename SizeType>
void DestructItems(ElementType* Element, SizeType Count)
{
	if constexpr (!TIsTriviallyDestructible<ElementType>::Value)
	{
		while (Count)
		{
			// We need a typedef here because VC won't compile the destructor call below if ElementType itself has a member called ElementType
			typedef ElementType DestructItemsElementTypeTypedef;

			Element->DestructItemsElementTypeTypedef::~DestructItemsElementTypeTypedef();
			++Element;
			--Count;
		}
	}
}

template <typename FromArrayType, typename ToArrayType>
constexpr bool CanMoveTArrayPointersBetweenArrayTypes()
{
	typedef typename FromArrayType::ElementType   FromElementType;
	typedef typename ToArrayType::ElementType   ToElementType;

	// Allocators must be equal or move-compatible...
	return  std::is_same_v         <ToElementType, FromElementType> ||      // The element type of the container must be the same, or...
			TIsBitwiseConstructible<ToElementType, FromElementType>::Value; // ... the element type of the source container must be bitwise constructible from the element type in the destination container
}

template <typename... Types>
struct TAnd;

template <bool LHSValue, typename... RHS>
struct TAndValue
{
	static constexpr bool Value = TAnd<RHS...>::value;
	static constexpr bool value = TAnd<RHS...>::value;
};

template <typename... RHS>
struct TAndValue<false, RHS...>
{
	static constexpr bool Value = false;
	static constexpr bool value = false;
};

template <typename LHS, typename... RHS>
struct TAnd<LHS, RHS...> : TAndValue<LHS::Value, RHS...>
{
};

template <>
struct TAnd<>
{
	static constexpr bool Value = true;
	static constexpr bool value = true;
};

template <typename DestinationElementType, typename SourceElementType>
struct TCanBitwiseRelocate
{
	enum
	{
		Value =
			std::is_same_v<DestinationElementType, SourceElementType> ||
			TAnd<
				TIsBitwiseConstructible<DestinationElementType, SourceElementType>,
				TIsTriviallyDestructible<SourceElementType>
			>::Value
	};
};

template <typename DestinationElementType, typename SourceElementType, typename SizeType>
void RelocateConstructItems(void* Dest, const SourceElementType* Source, SizeType Count)
{
	if constexpr (TCanBitwiseRelocate<DestinationElementType, SourceElementType>::Value)
	{
		/* All existing UE containers seem to assume trivial relocatability (i.e. memcpy'able) of their members,
		 * so we're going to assume that this is safe here.  However, it's not generally possible to assume this
		 * in general as objects which contain pointers/references to themselves are not safe to be trivially
		 * relocated.
		 *
		 * However, it is not yet possible to automatically infer this at compile time, so we can't enable
		 * different (i.e. safer) implementations anyway. */

		memmove(Dest, Source, sizeof(SourceElementType) * Count);
	}
	else
	{
		while (Count)
		{
			// We need a typedef here because VC won't compile the destructor call below if SourceElementType itself has a member called SourceElementType
			typedef SourceElementType RelocateConstructItemsElementTypeTypedef;

			new (Dest) DestinationElementType(*Source);
			++(DestinationElementType*&)Dest;
			(Source++)->RelocateConstructItemsElementTypeTypedef::~RelocateConstructItemsElementTypeTypedef();
			--Count;
		}
	}
}

static void* Memzero(void* Dest, size_t Count)
{
	return memset( Dest, 0, Count );
}

template <typename FuncType, typename... ArgTypes>
auto Invoke(FuncType&& Func, ArgTypes&&... Args)
	-> decltype(std::forward<FuncType>(Func)(std::forward<ArgTypes>(Args)...))
{
	return std::forward<FuncType>(Func)(std::forward<ArgTypes>(Args)...);
}

void ResizeAllocation(void*& Data, std::size_t PreviousNumElements, std::size_t NumElements, std::size_t NumBytesPerElement)
{
	// Avoid calling FMemory::Realloc( nullptr, 0 ) as ANSI C mandates returning a valid pointer which is not what we want.
	if (NumElements)
	{
		// Check for under/overflow
		if (NumElements < 0 || NumBytesPerElement < 1 || NumBytesPerElement > INT32_MAX)
		{
			return;
		}

		void* NewRealloc = ::realloc(Data, NumElements*NumBytesPerElement);
		Data = NewRealloc;
	}
	else
	{
		::free(Data);
		Data = nullptr;
	}
}

template <typename SizeType>
SizeType DefaultCalculateSlackShrink(SizeType NumElements, SizeType NumAllocatedElements, std::size_t BytesPerElement)
{
	SizeType Retval;
	_ASSERT(NumElements < NumAllocatedElements);

	// If the container has too much slack, shrink it to exactly fit the number of elements.
	const SizeType CurrentSlackElements = NumAllocatedElements - NumElements;
	const std::size_t CurrentSlackBytes = (NumAllocatedElements - NumElements)*BytesPerElement;
	const bool bTooManySlackBytes = CurrentSlackBytes >= 16384;
	const bool bTooManySlackElements = 3 * NumElements < 2 * NumAllocatedElements;
	if ((bTooManySlackBytes || bTooManySlackElements) && (CurrentSlackElements > 64 || !NumElements)) //  hard coded 64 :-(
	{
		Retval = NumElements;
	}
	else
	{
		Retval = NumAllocatedElements;
	}

	return Retval;
}

template <typename SizeType>
SizeType DefaultCalculateSlackGrow(SizeType NumElements, SizeType NumAllocatedElements, std::size_t BytesPerElement)
{
	const std::size_t FirstGrow = 4;
	const std::size_t ConstantGrow = 16;

	SizeType Retval;
	_ASSERT(NumElements > NumAllocatedElements && NumElements > 0);

	std::size_t Grow = FirstGrow; // this is the amount for the first alloc

	if (NumAllocatedElements || std::size_t(NumElements) > Grow)
	{
		// Allocate slack for the array proportional to its size.
		Grow = std::size_t(NumElements) + 3 * std::size_t(NumElements) / 8 + ConstantGrow;
	}
	
	Retval = (SizeType)Grow;

	// NumElements and MaxElements are stored in 32 bit signed integers so we must be careful not to overflow here.
	if (NumElements > Retval)
	{
		Retval = INT32_MAX;
	}

	return Retval;
}

template <typename SizeType>
SizeType DefaultCalculateSlackReserve(SizeType NumElements, std::size_t BytesPerElement)
{
	return NumElements;
}

template <typename T>
typename std::remove_reference<T>::type&& MoveTempIfPossible(T&& Obj)
{
	typedef typename std::remove_reference<T>::type CastType;
	return (CastType&&)Obj;
}
