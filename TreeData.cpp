/******************************************************************************
 * ++
 *
 * Filename:	TreeData.cpp
 *
 * Description:	Manages tree layout for a generic listview. This class manages
 *				information so that data in a listview may be displayed as a
 *				tree.
 *
 * --
 *****************************************************************************/
#include "stdafx.h"
#include "TreeData.h"
#include "TString.h"

#define STRICMP			_stricmp
#define MALLOC			malloc
#define REALLOC			realloc
#define FREE			free

#define DEFAULT_INIT_SIZE		16
#define DEFAULT_GROW_SIZE		16

int CompareAscending( const void *arg1, const void *arg2 );
int CompareDescending( const void *arg1, const void *arg2 );
/******************************************************************************
 * ++
 *
 * Section:		Sorting routines
 *
 * --
 *****************************************************************************/
int CompareAscending( const void *arg1, const void *arg2 )
{
	TreeItem *item1 = (TreeItem *) arg1;
	TreeItem *item2 = (TreeItem *) arg2;
	return STRICMP( (const char *) (*((TString *) item1->data)), (const char *) (*((TString *) item2->data)) );
}

int CompareDescending( const void *arg1, const void *arg2 )
{
	TreeItem *item1 = (TreeItem *) arg1;
	TreeItem *item2 = (TreeItem *) arg2;
	return STRICMP( (const char *) (*((TString *) item2->data)), (const char *) (*((TString *) item1->data)) );
}

/******************************************************************************
 * ++
 *
 * Section:		Protected TreeData methods
 *
 * --
 *****************************************************************************/
void TreeData::Initialize( int initialSize, int growSize )
{
	if ( initialSize <= 0 )
		initialSize = DEFAULT_INIT_SIZE;
	if ( growSize <= 0 )
		growSize = DEFAULT_GROW_SIZE;

	if ( m_pData )
	{
		// Use smart heap dispose instead of free...
		FREE( m_pData );
		m_pData = NULL;
	}

	// Use smart heap allocator instead of malloc...
	m_pData = (TreeItem *) MALLOC( sizeof(TreeItem) * initialSize );
	assert( m_pData );
	if ( m_pData )
	{
		m_iCurrSize = initialSize;
		m_iGrowSize = growSize;
		m_iNumEntries = 0;
	}
	else
	{
		m_iCurrSize = 0;
		m_iGrowSize = 0;
		m_iNumEntries = 0;
	}
}

int TreeData::Grow( int growBase )
{
	// Use smart heap reallocator instead of realloc...
	TreeItem *pNewData = (TreeItem *) REALLOC( m_pData, 
											  (sizeof(TreeItem)*(growBase+m_iGrowSize)) + 
											  (sizeof(TreeItem)*m_iCurrSize) );

	assert( pNewData );
	if ( pNewData )
	{
		// Do we really need to do this or did realloc already do this for us?
		// memcpy( pNewData, m_pData, sizeof(TreeItem) * m_iCurrSize );

		m_pData = pNewData;
		m_iCurrSize += (growBase + m_iGrowSize);
		return m_iCurrSize;
	}

	return 0;
}

/******************************************************************************
 * ++
 *
 * Section:		TreeData Constructors & Destructors
 *
 * --
 *****************************************************************************/
TreeData::TreeData()
{
	m_pData = NULL;
	Initialize( DEFAULT_INIT_SIZE, DEFAULT_GROW_SIZE );
}

TreeData::TreeData( int initialSize, int growSize )
{
	m_pData = NULL;
	Initialize( initialSize, growSize );
}

TreeData::~TreeData()
{
	if ( m_pData )
	{
		// Use smart heap dispose instead of free...
		FREE( m_pData );
		m_pData = NULL;
	}
}

/******************************************************************************
 * ++
 *
 * Section:		Adding & Removing items to TreeData
 *
 * --
 *****************************************************************************/
/*
 * Adds a new item to the end of the list (or tree structure).
 * Returns the index added (0-based), or -1 if failed.
 */
int TreeData::AddItem( int parent, int depth, short hasChildren, short isLastItem, void *data )
{
	int success = 1;
	if ( m_iNumEntries == m_iCurrSize )
		success = Grow( 0 );

	assert( success );
	if ( success )
	{
		m_pData[m_iNumEntries].parent = parent;
		m_pData[m_iNumEntries].depth = depth;
		m_pData[m_iNumEntries].hasChildren = hasChildren;
		m_pData[m_iNumEntries].isExpanded = 0;
		m_pData[m_iNumEntries].isLastItem = isLastItem;
		m_pData[m_iNumEntries].isDirty = 0;
		m_pData[m_iNumEntries].data = data;
		m_iNumEntries++;
		return( m_iNumEntries-1 );
	}

	return -1;
}

/*
 * Inserting a new item into the list (or tree structure)
 * requires a two step process. First call PrepareAddChildren()
 * to relocate the data and make room for insertions.
 * For each insertion, call InsertChildAt().
 */
int TreeData::PrepareAddChildren( int parent, int numChildren )
{
	int success = 1;
	if ( m_iCurrSize < m_iNumEntries+numChildren )
		success = Grow( numChildren );

	assert( success );
	int iNumToMove = m_iNumEntries - (parent+1);
	if ( success )
	{
		TreeItem *pSrc = m_pData + (parent+1);
		TreeItem *pDest = pSrc + numChildren;
//		memcpy( pDest, pSrc, sizeof(TreeItem)*iNumToMove );
		memmove(pDest, pSrc, sizeof(TreeItem)*iNumToMove );
		m_iNumEntries += numChildren;

		// Update parent indices of relocated blocks
		int iStart = parent + numChildren + 1;
		int iEnd = iStart + iNumToMove;
		for ( int i=iStart; i<iEnd; i++ )
		{
			if ( m_pData[i].parent > parent )
				m_pData[i].parent += numChildren;
		}

		return( parent+1 );
	}

	return -1;
}

int TreeData::InsertChildAt( int index, int parent, int depth, short hasChildren, short isLastItem, void *data )
{
	m_pData[index].parent = parent;
	m_pData[index].depth = depth;
	m_pData[index].hasChildren = hasChildren;
	m_pData[index].isExpanded = 0;
	m_pData[index].isLastItem = isLastItem;
	m_pData[index].isDirty = 0;
	m_pData[index].data = data;
	return( index+1 );
}

/*
 * Removes any item from the list (or tree structure).
 */
int TreeData::RemoveItem( int index )
{
	int iEntriesRemoved = 0;
	if ( index < (m_iNumEntries-1) )
	{
		iEntriesRemoved = 1;
			
		if ( m_pData[index].hasChildren && m_pData[index].isExpanded )
		{
			int iRemoveDepth = m_pData[index].depth;
			int iCurrIdx = index+1;
			while( (m_pData[iCurrIdx].depth > iRemoveDepth) && (iCurrIdx<m_iNumEntries) )
			{
				iEntriesRemoved++;
				iCurrIdx++;
			}
		}

		TreeItem *pDest = m_pData + index;
		TreeItem *pSrc = pDest + iEntriesRemoved;
		memcpy( pDest, pSrc, sizeof(TreeItem)*(m_iNumEntries-(index+iEntriesRemoved)) );

		// Update parent indices of relocated blocks
		int iStart = index + 1;
		int iEnd = iStart + (m_iNumEntries - (index+iEntriesRemoved));
		for ( int i=index+1; i<iEnd; i++ )
		{
			if ( m_pData[i].parent > index )
				m_pData[i].parent -= iEntriesRemoved;
		}				

	}
	else if ( index == (m_iNumEntries-1) )
	{
		iEntriesRemoved = 1;
	}
	// ignore the case where index > (m_iNumEntries-1) 
	// - this is an invalid case

	m_iNumEntries -= iEntriesRemoved;

	return iEntriesRemoved;
}

/******************************************************************************
 * ++
 *
 * Section:		Gets/Sets Public TreeData methods
 *
 * --
 *****************************************************************************/
void TreeData::SetExpanded( int index, bool expanded )
{
	if ( index < m_iNumEntries )
		m_pData[index].isExpanded = (short) expanded;
}

bool TreeData::GetExpanded( int index )
{
	bool expanded = false;

	if ( index < m_iNumEntries )
		expanded = (m_pData[index].isExpanded != 0);

	return expanded;
}

bool TreeData::HasChildren( int index )
{
	bool hasChildren = false;
		
	if ( index < m_iNumEntries )
		hasChildren = (m_pData[index].hasChildren != 0);

	return hasChildren;
}

int TreeData::GetDepth( int index )
{
	int depth = 0;
		
	if ( index < m_iNumEntries )
		depth = m_pData[index].depth;

	return depth;
}

int TreeData::GetParent( int index )
{
	int parent = -1;

	if ( index < m_iNumEntries )
		parent = m_pData[index].parent;

	return parent;
}

bool TreeData::IsLastItem( int index )
{
	bool isLastItem = false;

	if ( index < m_iNumEntries )
		isLastItem = (m_pData[index].isLastItem != 0);

	return isLastItem;
}

bool TreeData::IsDirty( int index )
{
	bool isDirty = false;

	if ( index < m_iNumEntries )
		isDirty = (m_pData[index].isDirty != 0);

	return isDirty;
}

void TreeData::SetDirty( int index, bool bDirty )
{
	if ( index < m_iNumEntries )
		m_pData[index].isDirty = (short) bDirty;
}

void *TreeData::GetData( int index )
{
	void *data = NULL;

	if ( index < m_iNumEntries )
		data = m_pData[index].data;

	return data;
}

bool  TreeData::SetData(int index,void* newData)
{
	bool retVal	=	false;
	if ( index >= 0 && index < m_iNumEntries )
	{
		m_pData[index].data		=	newData;
		retVal	=	true;
	}
	return retVal;	
}

void TreeData::Sort( SortType sortType )
{
	if ( sortType == Ascending )
	{
		qsort( m_pData, m_iNumEntries, sizeof(TreeItem), CompareAscending );
	}
	else if ( sortType == Descending )
	{
		qsort( m_pData, m_iNumEntries, sizeof(TreeItem), CompareDescending );
	}
}
