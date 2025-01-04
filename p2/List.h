#pragma once
#include <typeinfo>

template <class NodeType, class ElementType>
class TDoubleLinkedListIterator
{
public:

	explicit TDoubleLinkedListIterator(NodeType* StartingNode)
		: CurrentNode(StartingNode)
	{ }

	/** conversion to "bool" returning true if the iterator is valid. */
	explicit operator bool() const
	{ 
		return CurrentNode != nullptr; 
	}

	TDoubleLinkedListIterator& operator++()
	{
		_ASSERT(CurrentNode);
		CurrentNode = CurrentNode->GetNextNode();
		return *this;
	}

	TDoubleLinkedListIterator operator++(int)
	{
		auto Tmp = *this;
		++(*this);
		return Tmp;
	}

	TDoubleLinkedListIterator& operator--()
	{
		_ASSERT(CurrentNode);
		CurrentNode = CurrentNode->GetPrevNode();
		return *this;
	}

	TDoubleLinkedListIterator operator--(int)
	{
		auto Tmp = *this;
		--(*this);
		return Tmp;
	}

	// Accessors.
	ElementType& operator->() const
	{
		_ASSERT(CurrentNode);
		return CurrentNode->GetValue();
	}

	ElementType& operator*() const
	{
		_ASSERT(CurrentNode);
		return CurrentNode->GetValue();
	}

	NodeType* GetNode() const
	{
		_ASSERT(CurrentNode);
		return CurrentNode;
	}

	bool operator==(const TDoubleLinkedListIterator& Rhs) const { return CurrentNode == Rhs.CurrentNode; }
	bool operator!=(const TDoubleLinkedListIterator& Rhs) const { return CurrentNode != Rhs.CurrentNode; }

private:
	NodeType* CurrentNode;
};

template <class ElementType>
class TDoubleLinkedList
{
public:
	class TDoubleLinkedListNode
	{
	public:
		friend class TDoubleLinkedList;

		/** Constructor */
		TDoubleLinkedListNode( const ElementType& InValue )
			: Value(InValue), NextNode(nullptr), PrevNode(nullptr)
		{ }

		const ElementType& GetValue() const
		{
			return Value;
		}

		ElementType& GetValue()
		{
			return Value;
		}

		TDoubleLinkedListNode* GetNextNode()
		{
			return NextNode;
		}

		const TDoubleLinkedListNode* GetNextNode() const
		{
			return NextNode;
		}

		TDoubleLinkedListNode* GetPrevNode()
		{
			return PrevNode;
		}

		const TDoubleLinkedListNode* GetPrevNode() const
		{
			return PrevNode;
		}

	protected:
		ElementType            Value;
		TDoubleLinkedListNode* NextNode;
		TDoubleLinkedListNode* PrevNode;
	};

	/**
	 * Used to iterate over the elements of a linked list.
	 */
	typedef TDoubleLinkedListIterator<TDoubleLinkedListNode,       ElementType> TIterator;
	typedef TDoubleLinkedListIterator<TDoubleLinkedListNode, const ElementType> TConstIterator;

	/** Constructors. */
	TDoubleLinkedList()
		: HeadNode(nullptr)
		, TailNode(nullptr)
		, ListSize(0)
	{ }

	/** Destructor */
	virtual ~TDoubleLinkedList()
	{
		Empty();
	}

	// Adding/Removing methods

	/**
	 * Add the specified value to the beginning of the list, making that value the new head of the list.
	 *
	 * @param	InElement	the value to add to the list.
	 * @return	whether the node was successfully added into the list.
	 * @see GetHead, InsertNode, RemoveNode
	 */
	TDoubleLinkedListNode* AddHead( const ElementType& InElement )
	{
		return AddHead(new TDoubleLinkedListNode(InElement));
	}

	TDoubleLinkedListNode* AddHead( TDoubleLinkedListNode* NewNode )
	{
		if (NewNode == nullptr)
		{
			return NewNode;
		}

		// have an existing head node - change the head node to point to this one
		if ( HeadNode != nullptr )
		{
			NewNode->NextNode = HeadNode;
			HeadNode->PrevNode = NewNode;
			HeadNode = NewNode;
		}
		else
		{
			HeadNode = TailNode = NewNode;
		}

		SetListSize(ListSize + 1);
		return NewNode;
	}

	TDoubleLinkedListNode* AddTail( const ElementType& InElement )
	{
		return AddTail(new TDoubleLinkedListNode(InElement));
	}

	TDoubleLinkedListNode* AddTail( TDoubleLinkedListNode* NewNode )
	{
		if ( NewNode == nullptr )
		{
			return NewNode;
		}

		if ( TailNode != nullptr )
		{
			TailNode->NextNode = NewNode;
			NewNode->PrevNode = TailNode;
			TailNode = NewNode;
		}
		else
		{
			HeadNode = TailNode = NewNode;
		}

		SetListSize(ListSize + 1);
		return NewNode;
	}

	TDoubleLinkedListNode* InsertNode( const ElementType& InElement, TDoubleLinkedListNode* NodeToInsertBefore=nullptr )
	{
		return InsertNode(new TDoubleLinkedListNode(InElement), NodeToInsertBefore);
	}

	TDoubleLinkedListNode* InsertNode( TDoubleLinkedListNode* NewNode, TDoubleLinkedListNode* NodeToInsertBefore=nullptr )
	{
		if ( NewNode == nullptr )
		{
			return NewNode;
		}

		if ( NodeToInsertBefore == nullptr || NodeToInsertBefore == HeadNode )
		{
			return AddHead(NewNode);
		}

		NewNode->PrevNode = NodeToInsertBefore->PrevNode;
		NewNode->NextNode = NodeToInsertBefore;

		NodeToInsertBefore->PrevNode->NextNode = NewNode;
		NodeToInsertBefore->PrevNode = NewNode;

		SetListSize(ListSize + 1);
		return NewNode;
	}

	/**
	 * Remove the node corresponding to InElement.
	 *
	 * @param InElement The value to remove from the list.
	 * @see Empty, InsertNode
	 */
	void RemoveNode( const ElementType& InElement )
	{
		TDoubleLinkedListNode* ExistingNode = FindNode(InElement);
		RemoveNode(ExistingNode);
	}

	/**
	 * Removes the node specified.
	 *
	 * @param NodeToRemove The node to remove.
	 * @see Empty, InsertNode
	 */
	void RemoveNode( TDoubleLinkedListNode* NodeToRemove, bool bDeleteNode = true )
	{
		if ( NodeToRemove != nullptr )
		{
			// if we only have one node, just call Clear() so that we don't have to do lots of extra checks in the code below
			if ( Num() == 1 )
			{
				_ASSERT(NodeToRemove == HeadNode);
				if (bDeleteNode)
				{
					Empty();
				}
				else
				{
					NodeToRemove->NextNode = NodeToRemove->PrevNode = nullptr;
					HeadNode = TailNode = nullptr;
					SetListSize(0);
				}
				return;
			}

			if ( NodeToRemove == HeadNode )
			{
				HeadNode = HeadNode->NextNode;
				HeadNode->PrevNode = nullptr;
			}

			else if ( NodeToRemove == TailNode )
			{
				TailNode = TailNode->PrevNode;
				TailNode->NextNode = nullptr;
			}
			else
			{
				NodeToRemove->NextNode->PrevNode = NodeToRemove->PrevNode;
				NodeToRemove->PrevNode->NextNode = NodeToRemove->NextNode;
			}

			if (bDeleteNode)
			{
				delete NodeToRemove;
			}
			else
			{
				NodeToRemove->NextNode = NodeToRemove->PrevNode = nullptr;
			}
			SetListSize(ListSize - 1);
		}
	}

	/** Removes all nodes from the list. */
	void Empty()
	{
		TDoubleLinkedListNode* Node;
		while ( HeadNode != nullptr )
		{
			Node = HeadNode->NextNode;
			delete HeadNode;
			HeadNode = Node;
		}

		HeadNode = TailNode = nullptr;
		SetListSize(0);
	}

	// Accessors.

	/**
	 * Returns the node at the head of the list.
	 *
	 * @return Pointer to the first node.
	 * @see GetTail
	 */
	TDoubleLinkedListNode* GetHead() const
	{
		return HeadNode;
	}

	/**
	 * Returns the node at the end of the list.
	 *
	 * @return Pointer to the last node.
	 * @see GetHead
	 */
	TDoubleLinkedListNode* GetTail() const
	{
		return TailNode;
	}

	/**
	 * Finds the node corresponding to the value specified
	 *
	 * @param	InElement	the value to find
	 * @return	a pointer to the node that contains the value specified, or nullptr of the value couldn't be found
	 */
	TDoubleLinkedListNode* FindNode( const ElementType& InElement )
	{
		TDoubleLinkedListNode* Node = HeadNode;
		while ( Node != nullptr )
		{
			if ( Node->GetValue() == InElement )
			{
				break;
			}

			Node = Node->NextNode;
		}

		return Node;
	}

	bool Contains( const ElementType& InElement )
	{
		return (FindNode(InElement) != nullptr);
	}

	/**
	 * Returns true if the list is empty and contains no elements. 
	 *
	 * @returns True if the list is empty.
	 * @see Num
	 */
	bool IsEmpty() const
	{
		return ListSize == 0;
	}

	/**
	 * Returns the number of items in the list.
	 *
	 * @return Item count.
	 */
	std::int32_t Num() const
	{
		return ListSize;
	}

protected:

	/**
	 * Updates the size reported by Num().  Child classes can use this function to conveniently
	 * hook into list additions/removals.
	 *
	 * @param	NewListSize		the new size for this list
	 */
	virtual void SetListSize( std::int32_t NewListSize )
	{
		ListSize = NewListSize;
	}

private:
	TDoubleLinkedListNode* HeadNode;
	TDoubleLinkedListNode* TailNode;
	std::int32_t ListSize;

	TDoubleLinkedList(const TDoubleLinkedList&);
	TDoubleLinkedList& operator=(const TDoubleLinkedList&);

	friend TIterator      begin(      TDoubleLinkedList& List) { return TIterator     (List.GetHead()); }
	friend TConstIterator begin(const TDoubleLinkedList& List) { return TConstIterator(List.GetHead()); }
	friend TIterator      end  (      TDoubleLinkedList& List) { return TIterator     (nullptr); }
	friend TConstIterator end  (const TDoubleLinkedList& List) { return TConstIterator(nullptr); }
};