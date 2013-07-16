/******************************************************************************
 * ++
 *
 * Filename:	TreeData.h
 *
 * Description:	Manages tree layout for a generic listview. This class manages
 *				information so that data in a listview may be displayed as a
 *				tree.
 *
 * --
 *****************************************************************************/
#pragma once

/******************************************************************************
 * ++
 *
 * Section:		The TreeItem structure
 *
 * --
 *****************************************************************************/
typedef struct {
	int parent;
	int depth;
	short hasChildren;
	short isExpanded;
	short isLastItem;
	short isDirty;
	void *data;
} TreeItem;

/******************************************************************************
 * ++
 *
 * Section:		The TreeData class
 *
 * Exposed APIs:	<Note: All indices are zero based. -1 signifies no parent,
 *					 ie, it's the root item. Also 0 signifies the root depth.>
 *
 *							TreeData()
 *							TreeData( int initialSize, int growSize )
 *
 *					int		GetNumEntries()
 *					BOOL	HasChildren( int index )
 *					int		GetDepth( int index )
 *					int		GetParent( int index )
 *					void *	GetData( int index )
 *
 *					BOOL	GetExpanded( int index )
 *					void	SetExpanded( int index, BOOL bExpand )
 *
 *					int		AddItem( int parent, 
 *									 int depth, 
 *									 short hasChildren,
 *									 short isLastItem,
 *									 void *data )
 *					int		PrepareAddChildren( int parent, int numChildren )
 *					int		InsertChildAt( int index, 
 *										   int parent, 
 *										   int depth, 
 *										   short hasChildren,
 *										   short isLastItem,
 *										   void *data )
 *					int		RemoveItem( int index )
 *
 * <=========================================================================>
 *
 * Example:
 *
 *		TreeView					parent	depth	hasChildren	isExpanded isLastItem isDirty
 *	==================				======	=====	===========	========== ========== =======
 *
 * [-] FirstItem					  -1	  0			1		    1			0		 0
 *  |
 *  ----[-] FirstChild				   0      1			1			1			0		 0
 *  |    |
 *  |    ----  FirstGrandChild		   1	  2			0			0			0		 0
 *  |    |
 *  |    ----  SecondGrandChild		   1	  2			0			0			1		 0
 *  |    
 *  ----    SecondChild				   0	  1			0			0			0		 0
 *  |
 *  ----[+] ThirdChild				   0	  1			1			0			1		 0
 *
 *	   SecondItem					  -1	  0			0			0			0		 0
 *
 *
 * <=========================================================================>
 * 
 * API Description:
 *
 *		+ GetNumEntries()	Returns the number of items in the tree view
 *		+ HasChildren()		Does the specified indenx have children?
 *		+ GetDepth()		What's the depth of the specified index?
 *		+ GetParent()		What's the parent index of the specified index?
 *		+ GetData()			What's the data pointer for the specified index?
 *		+ GetExpanded()		Is the specified index expanded?
 *		+ SetExpanded()		Sets whether or not the specified index is expanded
 *
 *		+ AddItem()			Appends an item to the end of the tree view
 *		+ RemoveItem()		Remove the specified index from the tree view
 *
 *		*** To add or insert a block of items, use the following APIs: ***
 *		+ PrepareAddChildren()	First call this api to make room for the add 
 *								or insertion
 *		+ InsertChildAt()		Next, call this api to do the actual add or
 *								insertion
 * --
 *****************************************************************************/
class TreeData
{
protected:
	int m_iGrowSize;
	int m_iCurrSize;
	int m_iNumEntries;
	TreeItem *m_pData;

	void Initialize( int initialSize, int growSize );
	int Grow( int growBase );

public:
	/*
	 * Constructors & Destructors
	 */
	TreeData();
	TreeData( int initialSize, int growSize );
	virtual ~TreeData();

	/*
	 * Adds a new item to the end of the list (or tree structure).
	 * Returns the index added (0-based), or -1 if failed.
	 */
	int AddItem( int parent, int depth, short hasChildren, short isLastItem, void *data );

	/*
	 * Inserting a new item into the list (or tree structure)
	 * requires a two step process. First call PrepareAddChildren()
	 * to relocate the data and make room for insertions.
	 * For each insertion, call InsertChildAt().
	 */
	int PrepareAddChildren( int parent, int numChildren );
	int InsertChildAt( int index, int parent, int depth, short hasChildren, short isLastItem, void *data );

	/*
	 * Removes any item from the list (or tree structure).
	 */
	int RemoveItem( int index );

	/*
	 *  Generic Gets & Sets methods
	 *
	 * <Everything below this line is pretty trivial>
	 */
	inline int GetNumEntries() { return m_iNumEntries; }

	void SetExpanded( int index, bool expanded );
	bool GetExpanded( int index );
	bool HasChildren( int index );
	int GetDepth( int index );
	int GetParent( int index );
	bool IsLastItem( int index );
	bool IsDirty( int index );
	void SetDirty( int index, bool bDirty );
	void *GetData( int index );

	bool  SetData(int index,void* newData);	

	typedef enum SortType
	{
		Ascending,
		Descending
	} SortType;

	void Sort( SortType sortType );
};

