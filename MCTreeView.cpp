#include "stdafx.h"
#include "MCTreeView.h"
#include "SWFFormatDataSource.h"

// Platform specific no-no, TOBE removed...
#include "WinMCTreeView.h"

// TOBE removed too...
#pragma warning( disable : 4541 )

#define NAME_LABEL		"Name"
#define VALUE_LABEL		"Value"

/******************************************************************************
 * ++
 *
 * Name:			TMCTreeView()
 *
 * Description:		Constructor
 *
 * --
 *****************************************************************************/
MCTreeView::MCTreeView( void *platformParam,
						long controlStyle,
						bool bColorDirtyRow,
						TreeData *pTreeData,
						DataSource *pDataSource )
{
	TString strName = NAME_LABEL;
	TString strValue = VALUE_LABEL;

	m_controlStyle = controlStyle;
	m_bAllowNameEdit = (m_controlStyle & kClearAll) ? TRUE : FALSE;
	m_bAutoPollActive = FALSE;
	m_bColorDirtyRow = bColorDirtyRow;
	m_bSortItemAscending = FALSE;

	// Initialization of indices
	m_iEditRow = -1;
	m_iEditColumn = 0;
	m_iRowSelected = -1;
	m_iRowLastChanged = -1;

	m_iMouseOverRow = -1;

	m_pTreeData = NULL;
	m_pDataSource = NULL;

	// Start creating native controls
	m_pNativeControl = new WinMCTreeView( this, strName, strValue ); // Platform specific no-no, TOBE removed..., use factory instead....
	m_pNativeControl->CreateControl( platformParam );

	// SetTabIndex( kTabIndex_Default );

	SetTreeData( pTreeData );
	SetDataSource( pDataSource );

	assert( m_pTreeData );
	assert( m_pDataSource );

	// What a hack, but for Windows98 on some systems, the os puts garbage where
	// empty strings are supposed to be, so we're forced to use a space...
	if ( m_pNativeControl->UseSpaceAsEmptyString() )
	{
		; // (dynamic_cast(LingoDataSource *)(m_pDataSource))->SetEmptyStringAsSpace();
	}

	ReloadAllItems();
}

/******************************************************************************
 * ++
 *
 * Name:			~TMCTreeView()
 *
 * Description:		Destructor
 *
 * --
 *****************************************************************************/
MCTreeView::~MCTreeView()
{
	// Take care of the sub controls for windows
	if ( m_pNativeControl )
	{
		m_pNativeControl->DestroyControl();
		delete m_pNativeControl;
	}

	// *** Note ***
	// Now that TreeData and DataSource are constructor parameters,
	// it's up to the creator to clean up the TreeData & DataSource
	//
	// And of course, the data source
	/*
	if ( m_pDataSource )
		delete m_pDataSource;
	*/
}

/******************************************************************************
 * ++
 *
 * Name:			SetDataSource()
 *
 * Description:		Sets a new data source
 *
 * --
 *****************************************************************************/
DataSource *MCTreeView::SetDataSource( DataSource *pDataSource )
{
	DataSource *pOldDataSource = m_pDataSource;
	m_pDataSource = pDataSource;
	return pOldDataSource;
}

/******************************************************************************
 * ++
 *
 * Name:			SetTreeData()
 *
 * Description:		Sets a new tree data
 *
 * --
 *****************************************************************************/
TreeData *MCTreeView::SetTreeData( TreeData *pTreeData )
{
	TreeData *pOldTreeData = m_pTreeData;
	m_pTreeData = pTreeData;
	return pOldTreeData;
}

/******************************************************************************
 * ++
 *
 * Name:			SetNameColumnWidth()
 *
 * Description:		Sets the width of the name column (ie, the first column).
 *
 * --
 *****************************************************************************/
void MCTreeView::SetNameColumnWidth( int iWidth )
{
	m_pNativeControl->SetNameColumnWidth( iWidth );
}

/******************************************************************************
 * ++
 *
 * Name:			ExpandTree()
 *
 * Description:		Expands a row in the mc-tree view. Index is zero based.
 *
 *					*** Note: This should probably not be called directly.
 *					Haven't verified if it's safe to call directly yet.
 *					Call ExpandSelection() instead for now. ***
 *
 * --
 *****************************************************************************/
void MCTreeView::ExpandTree( int iRow )
{
	// TODO: Set cursor to wait...

	void *pParentData = m_pTreeData->GetData( iRow );
	int numChildren = m_pDataSource->GetNumChildren( pParentData );
	int childDepth = m_pTreeData->GetDepth( iRow ) + 1;
	bool bSuccess = (m_pTreeData->PrepareAddChildren( iRow, numChildren ) != -1);
	bool hasChildren = FALSE;
	void *data = NULL;

	assert( bSuccess );	// If this assert is hit then we probably weren't able to
						// grow the TreeData and ran out of memory...
	if ( bSuccess )
	{
		// Special clean-up for LingoDataSource (since we perform custom
		// bookkeeping and mapping of void* data pointers).
		((SWFFormatDataSource *) m_pDataSource)->RemoveAllChildren( pParentData );

		// Continue with our usual stuff...
		bool bDirty = m_pTreeData->IsDirty( iRow );
		for ( int i=0; i<numChildren; i++ )
		{
			data = m_pDataSource->GetChild( pParentData, i );
			hasChildren = m_pDataSource->HasChildren( data );
			m_pTreeData->InsertChildAt( iRow+1+i, iRow, childDepth, hasChildren, (i == (numChildren-1)), data );
			m_pTreeData->SetDirty( iRow+1+i, bDirty );

			m_pNativeControl->AddListItem( iRow+1+i, m_pDataSource->GetName(data), m_pDataSource->GetValue(data) );
		}

		// Adjust the selected row only if it's greater than the expanded row
		if ( m_iRowSelected > iRow )
			m_iRowSelected += numChildren;

		// Adjust the last modified row
		if ( m_iRowLastChanged > iRow )
			m_iRowLastChanged += numChildren;

		UpdateNonLeafDisplayValue( iRow );
	}

	// TODO: Reset cursor back...
}

/******************************************************************************
 * ++
 *
 * Name:			CollapseTree()
 *
 * Description:		Collapse a row in the mc-tree view. Index is zero based.
 *
 *					*** Note: This should probably not be called directly.
 *					Haven't verified if it's safe to call directly yet.
 *					Call CollapseSelection() instead for now. ***
 *
 * --
 *****************************************************************************/
void MCTreeView::CollapseTree( int iRow )
{
	if ( !TreeGetExpanded(iRow) || !TreeHasChildren(iRow) )
		return;
		
	// TODO: Set cursor to wait...

	int iCurrIdx = iRow + 1;

	int iEntriesRemoved = 0;
	int iTotalEntriesRemoved = 0;
	while ( m_pTreeData->GetParent( iCurrIdx ) == iRow )
	{
		iEntriesRemoved = m_pTreeData->RemoveItem( iCurrIdx );
		
		for ( int i=0; i<iEntriesRemoved; i++ )
		{
			m_pNativeControl->DeleteRow( iRow+1);
		}

		iTotalEntriesRemoved += iEntriesRemoved;
	}

	int iLastRowRemoved = iRow + iTotalEntriesRemoved;

	// Adjust the selected row
	if ( m_iRowSelected > iLastRowRemoved )
		m_iRowSelected -= iTotalEntriesRemoved;
	else if ( (m_iRowSelected>iRow) && (m_iRowSelected<=iLastRowRemoved) )
		m_iRowSelected = iRow;

	// Adjust the last modified row
	if ( m_iRowLastChanged > iLastRowRemoved )
		m_iRowLastChanged -= iTotalEntriesRemoved;
	else if ( (m_iRowLastChanged>iRow) && (m_iRowLastChanged<=iLastRowRemoved) )
		m_iRowLastChanged = -1;

	UpdateNonLeafDisplayValue( iRow );

	// TODO: Reset cursor back...
}

#define TOGGLE_NON_LEAF_VALUE
/******************************************************************************
 * ++
 *
 * Name:			UpdateNonLeafDisplayValue()
 *
 * Description:		New scheme for displaying non-leaf nodes requires that
 *					collapsed non-leaf display its value and when it's expanded,
 *					it values goes away.
 *
 * --
 *****************************************************************************/
bool MCTreeView::UpdateNonLeafDisplayValue( int iRow )
{
	bool bToggled = false;

	#ifdef TOGGLE_NON_LEAF_VALUE
	void *pData = TreeGetData( iRow );
	if ( pData )
	{
		bToggled = m_pDataSource->ToggleNonLeafDisplayValue( pData );
		if ( bToggled )
		{
			TString szNewValue = m_pDataSource->GetValue( pData );
			m_pNativeControl->SetItemText( iRow, 1, szNewValue );
		}
	}
	#endif

	return bToggled;
}

/******************************************************************************
 * ++
 *
 * Name:			SetSelection()
 *
 * Description:		Select a row. Only single selection is supported.
 *					Index is zero based.
 *
 * --
 *****************************************************************************/
void MCTreeView::SetSelection( int iRow, bool bUpdateRow /* = true */ )
{
	int iPrevSelected = m_iRowSelected;
	m_iRowSelected = iRow;
	if ( bUpdateRow )
	{
		m_pNativeControl->UpdateRow( iPrevSelected );
		m_pNativeControl->UpdateRow( iRow );
	}
}

/******************************************************************************
 * ++
 *
 * Name:			MoveSelectionUp()
 *
 * Description:		Sets the selected row to be the one above the current
 *					selected row. Index is zero based.
 *
 * --
 *****************************************************************************/
void MCTreeView::MoveSelectionUp()
{
	if ( m_iRowSelected > 0 )
		SetSelection( m_iRowSelected-1 );
}

/******************************************************************************
 * ++
 *
 * Name:			MoveSelectionDown()
 *
 * Description:		Sets the selected row to be the one below the current
 *					selected row. Index is zero based.
 *
 * --
 *****************************************************************************/
void MCTreeView::MoveSelectionDown()
{
	if ( (m_iRowSelected > -1) && (m_iRowSelected < (m_pTreeData->GetNumEntries()-1)) )
		SetSelection( m_iRowSelected+1 );
}

/******************************************************************************
 * ++
 *
 * Name:			ExpandSelection()
 *
 * Description:		Expand the row that is currently selected. 
 *					Index is zero based.
 *
 * --
 *****************************************************************************/
void MCTreeView::ExpandSelection()
{
	if ( (m_iRowSelected > -1) && m_pTreeData->HasChildren(m_iRowSelected) )
	{
		bool isExpanded = m_pTreeData->GetExpanded( m_iRowSelected );
		if ( !isExpanded )
		{
			ExpandTree( m_iRowSelected );
			m_pTreeData->SetExpanded( m_iRowSelected, !isExpanded );
			m_pNativeControl->UpdateRow( m_iRowSelected );
		}
	}
}

/******************************************************************************
 * ++
 *
 * Name:			CollapseSelection()
 *
 * Description:		Collapse the row that is currently selected. 
 *					Index is zero based.
 *
 * --
 *****************************************************************************/
void MCTreeView::CollapseSelection()
{
	if ( m_iRowSelected > -1 )
	{
		bool isExpanded = m_pTreeData->GetExpanded( m_iRowSelected );
		if ( m_pTreeData->HasChildren(m_iRowSelected) && isExpanded )
		{
			CollapseTree( m_iRowSelected );
			m_pTreeData->SetExpanded( m_iRowSelected, !isExpanded );
			m_pNativeControl->UpdateRow( m_iRowSelected );
		}
		else
		{
			int iParent = m_pTreeData->GetParent( m_iRowSelected );
			if ( iParent > -1 )
			{
				SetSelection( iParent );
			}
		}
	}
}

/******************************************************************************
 * ++
 *
 * Name:			DeleteSelection()
 *
 * Description:		Delete the row that is currently selected. 
 *					Index is zero based.
 *					Returns TRUE if we actually *did* delete the item.
 *
 * --
 *****************************************************************************/
bool MCTreeView::DeleteSelection()
{
	bool bDeleted = false;

	// Only editable control allows deletion of items
	if ( m_bAllowNameEdit && (m_iRowSelected > - 1) && (m_iRowSelected < TreeGetNumEntries()) )
	{
		// Only allow deletion of root level items
		if ( TreeGetDepth( m_iRowSelected ) == 0 )
		{
			DeleteItem( m_iRowSelected );

			// Update the selection...
			if ( TreeGetNumEntries() <= m_iRowSelected )
				SetSelection( TreeGetNumEntries() - 1 );

			bDeleted = true;
		}
	}

	return bDeleted;
}

/******************************************************************************
 * ++
 *
 * Name:			EditName()
 *
 * Description:		Notify the control that the name column of the specified
 *					row is being edited.
 *
 * --
 *****************************************************************************/
void MCTreeView::EditName( int iRow )
{
	m_pNativeControl->EditName( iRow );
	m_iEditRow = iRow;
	m_iEditColumn = 0;
}

/******************************************************************************
 * ++
 *
 * Name:			EditValue()
 *
 * Description:		Notify the control that the value column of the specified
 *					row is being edited.
 *
 * --
 *****************************************************************************/
void MCTreeView::EditValue( int iRow )
{
	m_pNativeControl->EditValue( iRow );
	m_iEditRow = iRow;
	m_iEditColumn = 1;
}

/******************************************************************************
 * ++
 *
 * Name:			EditCompleted()
 *
 * Description:		Edit was completed, so we need to take the input and 
 *					update the data in the multi-column tree control.
 *
 *					Possibilities are	1) Row was deleted with "" entry
 *											into the name column.
 *										2) Row was changed with "xyz" entry
 *											into the name column. Clean up
 *											previous entry & any expansions.
 *										3) Row was added with "xyz" entry 
 *											into the name column that was
 *											originally empty.
 *										4) Value was changed with "xyz" entry
 *											into the value column.
 *
 * --
 *****************************************************************************/
void MCTreeView::EditCompleted()
{
	bool bValueChanged = FALSE;
	int iPrevLastChanged = m_iRowLastChanged;

	char szText[256];
	GetEditText( szText, 256 );

	// Strip out leading spaces
	// *** Note ***
	// We assume that szText is null terminated! This is the case on Windows,
	// may want to double check on OSX. Checked. OSX is a-okay.
	char szTmp[256];
	char *pFirstChar = szText;
	while ( *pFirstChar == ' ' )
		pFirstChar++;
	if ( pFirstChar != szText )
	{
		strcpy( szTmp, pFirstChar );
		strcpy( szText, szTmp );
	}

	// Did the name or value column get edited???
	if ( m_iEditColumn == 0 )
	{
		// Get rid of the previous item
		DeleteItem( m_iEditRow);
		if ( strcmp(szText, "") )
		{
			// Text is populated, so we're adding a new entry...
			// Only root items can be edited, so add a new item
			// at root level with no parent.
			AddItem( m_iEditRow, TString(szText), TString::Null(), -1, 0, 0, true );
			m_pTreeData->SetDirty( m_iEditRow, TRUE );
		}

		m_pNativeControl->UpdateScrollPosition();
	}
	else
	{
		// Only do something if there was input, eg, anything but ""
		if ( strcmp(szText, "") )
		{
			void *pData = m_pTreeData->GetData( m_iEditRow );
			bool bUsedToHaveChildren = m_pDataSource->HasChildren( pData );

			// Set the value in the lingo engine
			m_pDataSource->SetValue( pData, szText );
		
			// Are we leaking memory here in LingoDataSource by calling FindItemByName(), which
			// creates a new data item. One already exists... Now we create another one to evaluate
			// its current value... Need to check this logic in LingoDataSource. 
			// Director MX - 20Nov02 - dsb... **** TO DO ****  %%%<Red Flag>%%%
			TString szLingoRep = (dynamic_cast<SWFFormatDataSource *>(m_pDataSource))->GetLingoName( pData );
			void *pTmpItem = m_pDataSource->FindItemByName( szLingoRep, TString::Null() );
			assert( pTmpItem );
			bool bWillHaveChildren = m_pDataSource->HasChildren( pTmpItem );
			(dynamic_cast<SWFFormatDataSource *>(m_pDataSource))->RemoveItem( pTmpItem );

			// Update if
			//		1. old value used to be a list
			//		2. new value is a list
			if ( bUsedToHaveChildren || bWillHaveChildren )
			{
				// Update the whole structure
				UpdateItem( m_iEditRow, true );
			}
			else
			{
				// Don't just update the text in the value column, reevaluate it
				// in the case that a user types in an expression...
				TString szNewValue = m_pDataSource->GetValue( pData, true );
				m_pNativeControl->SetItemText( m_iEditRow, m_iEditColumn, szNewValue );
				m_iRowLastChanged = m_iEditRow;

				m_pTreeData->SetDirty( m_iEditRow, TRUE );
				m_pNativeControl->UpdateRow( m_iEditRow ); 
			}

			// A Value has been edited...
			bValueChanged = TRUE;
		}
	}

	if ( iPrevLastChanged != m_iRowLastChanged )
		m_pNativeControl->UpdateRow( iPrevLastChanged );

	SetSelection( m_iRowLastChanged );

	// If someone's value has changed, update any entries with the
	// same lingo name in the Name column and also *notify our supervisor*
	if ( bValueChanged )
	{
		// UpdateSimilarItems( m_iEditRow );
		// Because we can have references in Lingo, UpdateSimilarItems() won't
		// cover all the bases. We must call UpdateAllValues(). Consider the 
		// following:		l1 = s3d.light(1)
		//					l2 = s3d.light(1)
		// Updating l1's property will not update l2's same property with the
		// UpdateSimilarItems() scheme...
		UpdateAllValues( m_iEditRow );

		/*
		TCommandValue arg(this);
		DispatchCommand( kCID_ValueChanged, &arg, GetSupervisor() );
		*/
	}
}

/******************************************************************************
 * ++
 *
 * Name:			EditReset()
 *
 * Description:		Once edit is completed, this must be call to reset
 *					the edit indices.
 *
 * --
 *****************************************************************************/
void MCTreeView::EditReset()
{
	if ( m_iEditRow > -1 ) {
		m_pNativeControl->UpdateRow( m_iEditRow );
		m_iEditRow = -1;
		m_iEditColumn = 0;
	}
}

/******************************************************************************
 * ++
 *
 * Name:			GetEditText()
 *
 * Description:		Retrieve the text from the edit control
 *
 * --
 *****************************************************************************/
void MCTreeView::GetEditText( char *szBuffer, unsigned int iBufferSize )
{
	m_pNativeControl->GetEditText( szBuffer, iBufferSize );
}

/******************************************************************************
 * ++
 *
 * Name:			IsDebugRow()
 *
 * Description:		Debug rows are rows that correspond to the Variable pane
 *					while debugging *and* the user has clicked on a call
 *					stack, on a method that is not the current context.
 *
 * --
 *****************************************************************************/
bool MCTreeView::IsDebugRow( int iRow )
{
	return ((SWFFormatDataSource *) (m_pDataSource))->IsDebugRow( TreeGetData(iRow) );
}

/******************************************************************************
 * ++
 *
 * Name:			AddItem()
 *
 * Description:		Currently appends an item to the mc-tree view.
 *					The default is to add the new item at the root level
 *					with no parent.
 *
 * --
 *****************************************************************************/
void MCTreeView::AddItem( int iRow, 
						   const TString &szName,
						   const TString &szInternalName,	/* =  0		*/
						   int iParent,						/* = -1		*/
						   int iDepth,						/* =  0		*/
						   short sLastItem,					/* =  0		*/
						   bool bUpdateLastChangedRow,		/* = false	*/
						   long lHandlerIdx,				/* = -1		*/
						   long lVariableIdx				/* = -1		*/ )
{
	short sHasChildren = 0;
	void *pNewItem = m_pDataSource->FindItemByName( szName, (szInternalName.IsEmpty() ? szName : szInternalName), lHandlerIdx, lVariableIdx );
	if ( pNewItem )
	{
		sHasChildren = m_pDataSource->HasChildren( pNewItem );
		bool bSuccess = false;	// This check is added here to ensure that we can grow
								// the tree data and don't run out of memory.

		if ( iRow == m_pTreeData->GetNumEntries() )
		{
			bSuccess = (m_pTreeData->AddItem( iParent, iDepth, sHasChildren, sLastItem, pNewItem ) != -1 );
		}
		else
		{
			// These routines should work for inserting a new item... (may want to double check)
			bSuccess = (m_pTreeData->PrepareAddChildren( iRow-1, 1 ) != -1);
			if ( bSuccess )
				m_pTreeData->InsertChildAt( iRow, iParent, iDepth, sHasChildren, sLastItem, pNewItem );
		}

		assert( bSuccess );	// If this assert is hit, then more likely, we weren't
							// able to grow the TreeData and ran out of memory...
		if ( bSuccess )
		{
			m_pNativeControl->AddListItem( iRow, szName, m_pDataSource->GetValue(pNewItem) );

			if ( bUpdateLastChangedRow )
				m_iRowLastChanged = iRow;
		}
	}
	else
	{
		assert( 0 ); // We should never reach here. If so, check LingoDataSource.h
					 // FindItemByName() should always return something valid even
					 // if there was a lingo error.
	}
}

void MCTreeView::AddItem( int iRow, 
						   void *pData,
						   int iParent,						/* = -1		*/
						   int iDepth,						/* =  0		*/
						   short sLastItem,					/* =  0		*/
						   bool bUpdateLastChangedRow,		/* = false	*/
						   long lHandlerIdx,				/* = -1		*/
						   long lVariableIdx				/* = -1		*/ )
{
	short sHasChildren = 0;
	void *pNewItem = pData;
	if ( pNewItem )
	{
		sHasChildren = m_pDataSource->HasChildren( pNewItem );
		bool bSuccess = false;	// This check is added here to ensure that we can grow
								// the tree data and don't run out of memory.

		if ( iRow == m_pTreeData->GetNumEntries() )
		{
			bSuccess = (m_pTreeData->AddItem( iParent, iDepth, sHasChildren, sLastItem, pNewItem ) != -1 );
		}
		else
		{
			// These routines should work for inserting a new item... (may want to double check)
			bSuccess = (m_pTreeData->PrepareAddChildren( iRow-1, 1 ) != -1);
			if ( bSuccess )
				m_pTreeData->InsertChildAt( iRow, iParent, iDepth, sHasChildren, sLastItem, pNewItem );
		}

		assert( bSuccess );	// If this assert is hit, then more likely, we weren't
							// able to grow the TreeData and ran out of memory...
		if ( bSuccess )
		{
			m_pNativeControl->AddListItem( iRow, m_pDataSource->GetName(pNewItem), m_pDataSource->GetValue(pNewItem) );

			if ( bUpdateLastChangedRow )
				m_iRowLastChanged = iRow;
		}
	}
	else
	{
		assert( 0 ); // We should never reach here. If so, check LingoDataSource.h
					 // FindItemByName() should always return something valid even
					 // if there was a lingo error.
	}
}

/******************************************************************************
 * ++
 *
 * Name:			AppendItem()
 *
 * Description:		Currently appends an item to the mc-tree view.
 *					If the handlerIdx and variableIdx is specified, then
 *					the LingoDataSource will retrieve its value differently.
 *					Different API to access variable while debugging in Director.
 *
 * --
 *****************************************************************************/
void MCTreeView::AppendItem( const TString &szName, 
							  long handlerIdx,	/* = -1   */
							  long variableIdx	/* = -1   */ )
{
	AddItem( TreeGetNumEntries(), szName, TString::Null(), -1, 0, 0, false, handlerIdx, variableIdx );
}

void MCTreeView::AppendItem( void *pData )
{
	AddItem( TreeGetNumEntries(), pData );
}

/******************************************************************************
 * ++
 *
 * Name:			DeleteItem()
 *
 * Description:		Currently deletes an item from the mc-tree view by index.
 *					Index is zero based.
 *
 * --
 *****************************************************************************/
void MCTreeView::DeleteItem( int iRow )
{
	if ( iRow < m_pTreeData->GetNumEntries() )
	{
		if ( m_pTreeData->GetData( iRow ) )
		{
			// Special clean-up for LingoDataSource (since we perform custom
			// bookkeeping and mapping of void* data pointers).
			void *pParentData = m_pTreeData->GetData( iRow );
			((SWFFormatDataSource *) m_pDataSource)->RemoveAllChildren( pParentData );
			((SWFFormatDataSource *) m_pDataSource)->RemoveItem( pParentData );
			
			//LingoDataSource deleted the item ... so set the data in the tree to NULL to be in sync
			m_pTreeData->SetData(iRow,NULL);
		}

		CollapseTree( iRow );
		m_pTreeData->RemoveItem( iRow );
		m_pNativeControl->DeleteRow( iRow );
		// Adjust the last modified row
		if ( m_iRowLastChanged > iRow )
			m_iRowLastChanged--;
		else if ( m_iRowLastChanged == iRow )
			m_iRowLastChanged = -1;
	}
}

/******************************************************************************
 * ++
 *
 * Name:			DeleteAllItems()
 *
 * Description:		Deletes all items in the mc tree view.
 *
 * --
 *****************************************************************************/
void MCTreeView::DeleteAllItems()
{
	int iNumEntries = m_pTreeData->GetNumEntries();
	for ( int i=0; i<iNumEntries; i++ )
		DeleteItem( 0 );
	m_pNativeControl->UpdateScrollPosition();
}

/******************************************************************************
 * ++
 *
 * Name:			CollapseAllItems()
 *
 * Description:		Collapses all items in the mc tree view to its root level.
 *					(Only root level items are shown).
 *
 * --
 *****************************************************************************/
void MCTreeView::CollapseAllItems()
{
	bool isExpanded = false; 
	int iCurrRow = 0;
	while ( iCurrRow < m_pTreeData->GetNumEntries() )
	{
		isExpanded = m_pTreeData->GetExpanded( iCurrRow );
		if ( m_pTreeData->HasChildren(iCurrRow) && isExpanded )
		{
			CollapseTree( iCurrRow );
			m_pTreeData->SetExpanded( iCurrRow, !isExpanded );
		}
		iCurrRow++;
	}
}

/******************************************************************************
 * ++
 *
 * Name:			UpdateItem()
 *
 * Description:		Currently updates an item from the mc-tree view by index.
 *					Index is zero based. Note, this method updates the whole
 *					data structure. i.e., it may have initially been a list
 *					and now it's only an integer (or vice-versa). In this case
 *					the + and - signs are updated appropriately. If it's no
 *					longer a list, then we get rid of the +/- signs, etc.
 *
 *					*** Read this ***
 *					Note: Updating a row that is expanded will cause it to 
 *					be collapsed. This is because the  structure needs to be
 *					updated in the process.
 * --
 *****************************************************************************/
void MCTreeView::UpdateItem( int iRow, 
							  bool bUpdateLastChangedRow,	/* = false	*/
							  bool bPreserveDirtyBit		/* = false  */ )
{
	if ( iRow < m_pTreeData->GetNumEntries() )
	{
		void *pData = m_pTreeData->GetData( iRow );
		if ( pData )
		{
			// 1. Save all old infomation for bookkeeping - Lingo representation and tree data structures
			TString strName = m_pDataSource->GetName( pData );
			TString strLingoRep = (dynamic_cast<SWFFormatDataSource *>(m_pDataSource))->GetLingoName( pData );
			void *pOldSourceParent = (dynamic_cast<SWFFormatDataSource*>(m_pDataSource))->GetParent( pData );
			int iOldParent = m_pTreeData->GetParent( iRow );
			int iOldDepth = m_pTreeData->GetDepth( iRow );
			short sOldLastItem = m_pTreeData->IsLastItem( iRow );
			bool bOldDirty = m_pTreeData->IsDirty( iRow );

			// 2. Delete the old item
			DeleteItem( iRow );

			// 3. Insert the new item where the deleted one used to be.
			//    Note that the new item will have the old tree data states
			AddItem( iRow, 
					 strName, 
					 strLingoRep, 
					 iOldParent, 
					 iOldDepth, 
					 sOldLastItem,
					 bUpdateLastChangedRow );

			// 4. Get the new data pointer and restore LingoDataSource parent so that tree is consistent.
			pData = m_pTreeData->GetData( iRow );
			(dynamic_cast<SWFFormatDataSource*>(m_pDataSource))->SetParent( pData, pOldSourceParent );

			// 5. Update the scroll position
			// This is in the case that a collapse decreases the scroll range
			m_pNativeControl->UpdateScrollPosition();

			// 6. Set it as dirty, if the bPreserveDirtyBit is not set
			m_pTreeData->SetDirty( iRow, bPreserveDirtyBit ? bOldDirty : TRUE );
		}
	}
}

/******************************************************************************
 * ++
 *
 * Name:			UpdateAllItems()
 *
 * Description:		Updates all items in the mc tree view. 
 *
 *					*** Read this ***
 *					Note: Please read the UpdateItem() description above.
 *					It's very important :)
 *
 * --
 *****************************************************************************/
void MCTreeView::UpdateAllItems( bool bPreserveDirtyBit	/* = false  */ )
{
	int iCurrRow = 0;
	while ( iCurrRow < m_pTreeData->GetNumEntries() )
	{
		UpdateItem( iCurrRow, false, bPreserveDirtyBit );
		iCurrRow++;
	}
}

/******************************************************************************
 * ++
 *
 * Name:			UpdateAllValues()
 *
 * Description:		Updates the value column of the mc-treeview. This routine
 *					uses the smart-update scheme in which only dirty values
 *					are updated and redrawn.
 *
 * --
 *****************************************************************************/
void MCTreeView::UpdateAllValues( int iExcludedRow /* = -1 */ )
{
	void *pData = NULL;

	int iCurrIdx = 0;
	long dirty = DataSource::NOT_DIRTY;
	while ( iCurrIdx < m_pTreeData->GetNumEntries() )
	{
		if ( iCurrIdx != iExcludedRow )
		{
			pData = m_pTreeData->GetData( iCurrIdx );
			dirty = m_pDataSource->IsDirty( pData );
			if ( dirty == DataSource::NOT_DIRTY )
			{
				// It's *NOT* dirty, it's clean!!!
				if ( m_pTreeData->IsDirty(iCurrIdx) )
				{
					m_pTreeData->SetDirty( iCurrIdx, FALSE );
					m_pNativeControl->UpdateRow( iCurrIdx );
				}
			}
			else if ( dirty & DataSource::STRUCT_DIRTY )
			{
				if ( dirty & DataSource::NAME_DIRTY )
				{
					TString szNewName = m_pDataSource->GetName( pData, true );
					m_pNativeControl->SetItemText( iCurrIdx, 0, szNewName );
				}

				// Update the whole structure
				UpdateItem( iCurrIdx );
			}
			else // the NAME or the VALUE or both are dirty ...
			{
				if ( dirty & DataSource::NAME_DIRTY )
				{
					TString szNewName = m_pDataSource->GetName( pData, true );
					m_pNativeControl->SetItemText( iCurrIdx, 0, szNewName );
				}

				if ( dirty & DataSource::VALUE_DIRTY )
				{
					// Don't just update the text in the value column, reevaluate it
					// in the case that a user types in an expression...
					TString szNewValue = m_pDataSource->GetValue( pData, true );
					m_pNativeControl->SetItemText( iCurrIdx, 1, szNewValue );
				}

				// Must call UpdateRow() here so that the color of the name column also changes...
				m_pTreeData->SetDirty( iCurrIdx, TRUE );
				m_pNativeControl->UpdateRow( iCurrIdx ); 
			}
		}
		iCurrIdx++;
	}

	// Redraw the edit control if it's active
	if ( m_iEditRow > -1 )
		m_pNativeControl->BringEditCtrlToFront();
}

/******************************************************************************
 * ++
 *
 * Name:			ReloadAllItems()
 *
 * Description:		Reloads all data from the data source.
 *
 * --
 *****************************************************************************/
void MCTreeView::ReloadAllItems()
{	
	TString szName;
	TString szValue;
	void *pData = NULL;
	char szNullValue[2];
	szNullValue[0] = 0;
	szNullValue[1] = 0;

	// Special handling for Windows 98 (use a space instead of a nullstring)
	if ( m_pNativeControl->UseSpaceAsEmptyString() )
		szNullValue[0] = ' ';

	// Clear the control
	m_pNativeControl->ClearControl();

	// Populate it with the data
	int iNumRows = m_pTreeData->GetNumEntries();
	for ( int i=0; i<iNumRows; i++ )
	{
		pData = m_pTreeData->GetData( i );
		assert( pData );
		if ( pData )
		{
			szName = m_pDataSource->GetName( pData );
			if ( m_pDataSource->HasStringRepValue( pData ) )
				szValue = m_pDataSource->GetValue( pData, true );
			else
				szValue.SetEmpty();

			m_pNativeControl->AddListItem( i, szName, szValue.IsEmpty() ? TString(szNullValue) : szValue );
		}
	}
}

/******************************************************************************
 * ++
 *
 * Name:			UpdateSimilarItems()
 *
 * Description:		Updates all items with the same Lingo name as the passed
 *					in row. (ie, if there are entries which accesses the object,
 *					when one is updated, this method will update the others).
 *
 *					*** Note *** Caveat, if you are referencing an object in
 *					two different ways, this method will not take care of it.
 *					There's no way to do object equalities because our lingo
 *					interface is a string query. So it's a no go for stuff
 *					like ===> shockwave3Dmember.model(1)
 *							  shockwave3Dmember.model("first_model")
 *					Although they may be the same object, we do not have an
 *					easy way to figure this out (in a generic sense - ie,
 *					not Shockwave3D specific). 
 *
 * --
 *****************************************************************************/
void MCTreeView::UpdateSimilarItems( int iItemReference )
{
	int iCurrRow = 0;
	assert( iItemReference < m_pTreeData->GetNumEntries() );

	void *pReferenceData = m_pTreeData->GetData( iItemReference );
	assert( pReferenceData );
	TString szReferenceLingoName = (dynamic_cast<SWFFormatDataSource *>(m_pDataSource))->GetLingoName( pReferenceData );

	void *pCurrData = NULL;
	TString szCurrLingoName;

	while ( iCurrRow < m_pTreeData->GetNumEntries() )
	{
		pCurrData = m_pTreeData->GetData( iCurrRow );
		assert( pCurrData );
		if ( pCurrData && (pCurrData != pReferenceData) )
		{
			szCurrLingoName = (dynamic_cast<SWFFormatDataSource *>(m_pDataSource))->GetLingoName( pCurrData );
			if ( szReferenceLingoName == szCurrLingoName )
				UpdateItem( iCurrRow );
		}
		iCurrRow++;
	}
}

/******************************************************************************
 * ++
 *
 * Name:			SortRootItems()
 *
 * Description:		Collapses the tree hierarchy and sorts all the root items.
 *					This is usually invoked by clicking on the name column
 *					of the header bar.
 *
 * --
 *****************************************************************************/
void MCTreeView::SortRootItems()
{
	// Toggle the sorting from ascending to descending and back
	m_bSortItemAscending = m_bSortItemAscending ? FALSE : TRUE;

	CollapseAllItems();
	m_pTreeData->Sort( m_bSortItemAscending ? TreeData::Ascending : TreeData::Descending );
	UpdateAllItems( true );  // Refresh the listview data & preserve the dirty bits
}

/******************************************************************************
 * ++
 *
 * Name:			OnLoseFocus()
 *
 * Description:		Calls by the framework when we lose focus. Clear the 
 *					selection.
 *
 * --
 *****************************************************************************/
void MCTreeView::OnLoseFocus()
{
	SetSelection( -1 );
}

void MCTreeView::Resize( int iWidth, int iHeight )
{
	m_pNativeControl->Resize( iWidth, iHeight );
}


