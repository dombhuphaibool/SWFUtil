#pragma once

#include "NativeMCTreeView.h"
#include "TreeData.h"
#include "DataSource.h"

// Column numbers
#define NAME_COLUMN					0
#define VALUE_COLUMN				1

// Color definitions
#define INVERTED_SELECTION_COLOR	RGB( 225, 225, 225 )
#define LAST_CHANGED_COLOR			RGB( 180, 0, 0 )
#define LAST_CHANGED_HIGHLIGHT		RGB( 255, 100, 100 )
#define GRID_COLOR					RGB( 200, 220, 255 )
#define HIERARCHY_COLOR				RGB( 180, 180, 180 )

// Maximum length for displaying the name or value text
#define LIST_MAX_DISPLAY_LEN		512

// -----------------------------------------------------------------------------
// TTreeView Declaration
//
//		* Positions are 0 based
//
//		* Note: Windows multi-column tree views are composed of a container
//				window which contains a native win-32 custom drawn list view
//				and a native win-32 edit control. The custom drawn list view
//				internally contains a header control but to be able to hook
//				the win32 messages to our framework, we needed a corresponding
//				framework class instance, which is where m_pListViewHeader comes
//				into play.
//
//				Macs on the other hand just uses a DataBrowser, so any code that 
//				refers to sub controls should be windows specific and 
//				encapsulated with WINONLY() macros or #ifdef WINVER.
//
//		* Since with windows platform specific code, we're dealing with
//				three identical class tree hierachy structures (one being 
//				redundant because we're mapping a view to a window being
//				that windows controls are windows - and not so for the macs?
//				so says the director framework comments), the mapping class
//				aka TNativeTreeView, TNativeCustomListView, 
//				TNativeCustomEditCtrl, etc. will be skeletons passing the 
//				method calls to its delegate TWin... derived classes.
//
//				Also, from prototyping for mac OSX data browser control, it
//				seems that it needs a tree management structure, so that's the
//				reason that TreeData is left at this level and anything
//				dealing with tree management is done at this level (and not
//				lower down the food chain).
//
// -----------------------------------------------------------------------------

class MCTreeView {

public:
	typedef enum ControlStyle
	{
		kStandard		= 0x0000,
		kClearAll		= 0x0001,
		kAutoPoll		= 0x0002,
		kValueTooltip	= 0x0004
	} ControlStyle;

protected:
	// ------------------------------------------------------------------------
	// * Member variables
	// ------------------------------------------------------------------------
	long m_controlStyle;			// style of multi-column tree view control
	bool m_bAllowNameEdit;			// can the name column be editable?
	bool m_bAutoPollActive;			// should we auto poll on idle?
	bool m_bColorDirtyRow;			// should we color the dirty row?
	bool m_bSortItemAscending;		// sort the root items ascending?

	int m_iRowSelected;				// index of the row currently selected (zero-base)
	int m_iEditRow;					// index of the row being editted (zero-based)
	int m_iEditColumn;				// index of the column in the row being editted (zero-based)
	int m_iRowLastChanged;			// index of the last row changed (zero-based)

	int m_iMouseOverRow;			// the row in which the mouse is hovering over

	TreeData *m_pTreeData;			// tree hierarchy manager
	DataSource *m_pDataSource;		// access to a data source
	NativeMCTreeView *m_pNativeControl;	// the platform specific representation

public:
	// ------------------------------------------------------------------------
	// * Construction & Destruction
	// ------------------------------------------------------------------------
	MCTreeView( void *platformParam,
				long controlStyle = kStandard,
				bool bColorDirtyRow = FALSE,
				TreeData *pTreeData = NULL,
				DataSource *pDataSource = NULL );

	virtual ~MCTreeView();

	// ------------------------------------------------------------------------
	// * Misc methods
	// ------------------------------------------------------------------------
	DataSource *SetDataSource( DataSource *pDataSource );
	TreeData *SetTreeData( TreeData *pTreeData );
	inline bool AllowNameEdit()			{ return m_bAllowNameEdit; }
	inline bool ColorDirtyRow()			{ return m_bColorDirtyRow; }
	void SetNameColumnWidth( int iWidth );

	// ------------------------------------------------------------------------
	// * Tree Managament
	// ------------------------------------------------------------------------
	inline int	TreeGetDepth( int iRow )	{ return m_pTreeData->GetDepth( iRow );		}
	inline int  TreeGetParent( int iRow )	{ return m_pTreeData->GetParent( iRow );	}
	inline void*TreeGetData( int iRow )		{ return m_pTreeData->GetData( iRow );		}
	inline bool TreeHasChildren( int iRow )	{ return m_pTreeData->HasChildren( iRow );	}
	inline bool TreeIsLastItem( int iRow )	{ return m_pTreeData->IsLastItem( iRow );	}
	inline bool TreeIsDirty( int iRow )		{ return m_pTreeData->IsDirty( iRow );		}
	inline int	TreeGetNumEntries()			{ return m_pTreeData->GetNumEntries();		}
	inline bool TreeGetExpanded( int iRow )	{ return m_pTreeData->GetExpanded( iRow );	}
	inline void TreeSetExpanded( int iRow, bool bExpand )
											{ m_pTreeData->SetExpanded( iRow, bExpand );}

	void ExpandTree( int iRow );
	void CollapseTree( int iRow );
	bool UpdateNonLeafDisplayValue( int iRow );

	// ------------------------------------------------------------------------
	// * Selection
	// ------------------------------------------------------------------------
	void SetSelection( int iRow, bool bUpdateRow = true );
	inline int GetSelectedRow()			{ return m_iRowSelected; }
	void MoveSelectionUp();
	void MoveSelectionDown();
	void ExpandSelection();
	void CollapseSelection();
	bool DeleteSelection();

	// ------------------------------------------------------------------------
	// * Edits
	// ------------------------------------------------------------------------
	void EditName( int iRow );
	void EditValue ( int iRow );
	void EditCompleted();
	void EditReset();
	void GetEditText( char* szBuffer, unsigned int iBufferSize );
	bool IsDebugRow( int iRow );	// Debug rows are rows that correspond to
									// variables in the Varible pane, while
									// debugging & the user has clicked up the call stack.

	inline int GetEditedRow()			{ return m_iEditRow; }
	inline int GetEditedCol()			{ return m_iEditColumn; }
	inline int GetLastChangedRow()		{ return m_iRowLastChanged; }
	inline void SetLastChangedRow( int iRow ) { m_iRowLastChanged = iRow; }

	// ------------------------------------------------------------------------
	// * Auto Polling
	// ------------------------------------------------------------------------
	inline bool GetAutoPoll()			{ return m_bAutoPollActive; }
	inline void SetAutoPoll( bool active ) { m_bAutoPollActive = active; }

	// ------------------------------------------------------------------------
	// * Actual Data
	// ------------------------------------------------------------------------
	// Specifying the handlerIdx & variableIdx tells the LingoDataSource to retrieve its value differently...
	void AddItem( int iRow, 
				  const TString &szName, 
				  const TString &szInternalName,
				  int iParent = -1, 
				  int iDepth = 0, 
				  short sLastItem = 0,
				  bool bUpdateLastChangedRow = false,
				  long lHandlerIdx = -1,
				  long lVariableIdx = -1 );
	void AppendItem( const TString &szName, long handlerIdx = -1, long variableIdx = -1 );	
	void AddItem( int iRow, 
				  void *pData,
				  int iParent = -1, 
				  int iDepth = 0, 
				  short sLastItem = 0,
				  bool bUpdateLastChangedRow = false,
				  long lHandlerIdx = -1,
				  long lVariableIdx = -1 );
	void AppendItem( void *pData );	

	void DeleteItem( int iRow );
	void DeleteAllItems();
	void CollapseAllItems();
	void UpdateItem( int iRow, bool bUpdateLastChangedRow = false, bool bPreserveDirtyBit = false );
	void UpdateAllItems( bool bPreserveDirtyBit = false );	// Re-evaluate the whole data structure - it may no longer be a list, etc.
	void UpdateAllValues( int iExcludedRow = -1 );			// Only re-evaluate it's value text representation
	void ReloadAllItems();									// Reloads all data from the data source. Used for closing/reopening the Obj Insp. with the data source being persistent.
	void UpdateSimilarItems( int iItemReference );
	void SortRootItems();
	// void SetText( int iRow, int iCol, const char *szText );

public:
	// ------------------------------------------------------------------------
	// * Public overrided methods
	// ------------------------------------------------------------------------
	// void Activate();
	// void Deactivate();
	// virtual bool OnMouseDown( const TPoint& ptMouse, const TModifiers& modifiers );
	void Resize( int iWidth, int iHeight );

protected:
	// ------------------------------------------------------------------------
	// * Protected overrided methods
	// ------------------------------------------------------------------------
	virtual void OnLoseFocus();
};

