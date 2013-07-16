#pragma once

#include "TString.h"
#include "NativeMCTreeView.h"
#include "MCTreeView.h"

void InitializeMCTreeWindowClass( HINSTANCE hInstance );

// Multi-column tree view class.
class WinMCTreeView : public NativeMCTreeView
{
public:
	// ------------------------------------------------------------------------
	// * Construction & Destruction
	// ------------------------------------------------------------------------
	WinMCTreeView( MCTreeView *pMCTreeView, TString strName, TString strValue );
	~WinMCTreeView();

	// ------------------------------------------------------------------------
	// * Initialization
	// ------------------------------------------------------------------------
	virtual int CreateControl( void *param );
	virtual void DestroyControl();
	void InitializeDrawingDimensions();
	void SetSize( int iWidth, int iHeight );
	virtual void SetNameColumnWidth( int iWidth );

	// ------------------------------------------------------------------------
	// * MC-TreeView Data Management 
	// ------------------------------------------------------------------------
	virtual int AddListItem( int iRow, const TString &szCol0, const TString &szCol1 );
	virtual void SetItemText( int iRow, int iCol, const TString &szText );

	virtual void UpdateRow( int iRow );
	virtual void DeleteRow( int iRow );

	// ------------------------------------------------------------------------
	// * Edits
	// ------------------------------------------------------------------------
	virtual void EditName( int iRow );
	virtual void EditValue ( int iRow );
	virtual void GetEditText( char *szBuffer, unsigned int iBufferSize );

	// ------------------------------------------------------------------------
	// * Redraw Requests
	// ------------------------------------------------------------------------
	void RedrawVisibleItems();
	// void RedrawEditCtrl();
	virtual void BringEditCtrlToFront();
	virtual void ClearControl();

	// ------------------------------------------------------------------------
	// * Scrolling, etc.
	// ------------------------------------------------------------------------
	virtual void UpdateScrollPosition();

	// ------------------------------------------------------------------------
	// * Hacky whacky crap...
	// ------------------------------------------------------------------------
	virtual bool UseSpaceAsEmptyString() { return( m_osPlatform == Windows98 ); }

	WNDPROC m_wndProcListView;
	WNDPROC m_wndProcEditCtrl;

protected:
	// ------------------------------------------------------------------------
	// * Our root container and sub-controls
	// ------------------------------------------------------------------------
	HINSTANCE m_hAppInst;			// Application instance
	HWND m_pParentView;				// Our parent view
	HWND m_pRootContainer;			// This is TestView (us - our main view)
	HWND m_pListView;				// The ListView control that we encapsulate
	HWND m_pEditCtrl;				// EditControl for input
	MCTreeView *m_pMCTreeView;		// The platform independent MC Tree View

	// ------------------------------------------------------------------------
	// * Custom Draw - Whee!!! - draw our own tree view
	// ------------------------------------------------------------------------
	HPEN m_hPenBlack;				// Normally, text are written in black
	HPEN m_hPenWhite;				// Highlighted text color
	HPEN m_hPenRed;					// Last changed text color
	HPEN m_hPenLightRed;			// Last changed text color if highlighted
	HPEN m_hPenGrid;				// Grid pen - light blue :)
	HPEN m_hPenHierarchy;			// Hierarchy pen - some sort of grey
	HBRUSH m_hBrushWhite;			// White background brush
	HBRUSH m_hBrushHighlight;		// Default highlight color brush

	int m_iColumn0Width;			// Width of the name column
	int m_iColumn1Width;			// Width of the value column
	int m_iRowHeight;				// Height of a row
	int m_iBoxWidth;				// Width of the plus/minus box
	int m_iBoxHeight;				// Height of the plus/minus box
	int m_iHeaderHeight;			// Height of the header
	int m_iInitialIndent;			// The initial indentation from the name column before drawing the box
	int m_iBoxYOffset;				// The Y-Offset from the top of the row to the top of the box
	int m_iTextYOffset;				// The Y-Offset from the top of the row to the top of the text
	int m_iPaintYOffset;			// The Y-Offset from the top of the row to the top of the paint rect
	int m_iScrollPos;				// The Y-Scoll position of the scroll thumb
	int m_iVisibleRange;			// The number of possible items visible in the list

	TString m_strName;				// Column 0 label (Name Column)
	TString m_strValue;				// Column 1 label (Value Column)

	typedef enum OSPlatform {
		Windows,
		Windows98,
		WindowsXP
	} OSPlatform;

	OSPlatform m_osPlatform;
	char m_szEmptyString[2];

	int CreateControl( HINSTANCE hAppInst, HWND hParentWnd );
	int CreateListView( HWND pParentView );
	int CreateEditCtrl( HWND pParentView );

	// ------------------------------------------------------------------------
	// * Special state variable and methods for managing column resizing via
	// * interaction with the header control
	// ------------------------------------------------------------------------
	BOOL m_bAdjustingColumn;		// State variable to prevent recursive column adjustment
	BOOL m_bClampColumnWidth;		// State varialbe to prevent recursive column clamping

public: // temp hack making this public for now...
	void GetBoxRect( int iRow, RECT &rBoxRect );
	void GetTextRect( int iRow, int iCol, RECT &rTextRect );

	typedef enum DashType {
		SHORT_DASH,
		LONG_DASH } DashType;
	void DrawDash( HDC hdc, int iRow, DashType, RECT &box );
	void DrawVerticalParentChildLine( HDC hdc, int iRow, RECT &box );
	void DrawBox( HDC hdc, RECT &box );
	void DrawPlusMinus( HDC hdc, RECT &box, BOOL plus );
	void FillBackground( HDC hdc, int iRow );
	void DrawVerticalGridLine( HDC hdc, int iRow );

	void DrawNameColumnItem( HDC hdc, int iRow, RECT &box );
	void DrawValueColumnItem( HDC hdc, int iRow );
	void DrawListSubItem( HDC hdc, int iRow, int iColumn );
	void UpdateVisibleRange( int iNewHeight );

	void SetScrollPosition( int vPos );
	void ListViewVScroll( HWND hWnd, UINT code, int pos );
	BOOL MouseWheel( HWND hWnd, WORD fwKeys, short zDelta, int xPos, int yPos );

	void AdjustColumnWidth();
	void ClampColumnWidth();
	void StoreCurrColumnWidths();
	int GetListViewWidth();

	// ------------------------------------------------------------------------
	// * Data Access
	// ------------------------------------------------------------------------
	int GetDepth( int iRow )			{ return m_pMCTreeView->TreeGetDepth( iRow ); }
	int GetParent( int iRow )			{ return m_pMCTreeView->TreeGetParent( iRow ); }
	void *GetData( int iRow )			{ return m_pMCTreeView->TreeGetData( iRow ); }
	BOOL HasChildren( int iRow )		{ return m_pMCTreeView->TreeHasChildren( iRow ); }
	BOOL IsLastItem( int iRow )			{ return m_pMCTreeView->TreeIsLastItem( iRow ); }
	BOOL GetExpanded( int iRow )		{ return m_pMCTreeView->TreeGetExpanded( iRow ); }
	void SetExpanded( int iRow, BOOL bExpand ) { m_pMCTreeView->TreeSetExpanded( iRow, (bExpand != 0) ); }
	BOOL IsDirty( int iRow )			{ return m_pMCTreeView->TreeIsDirty( iRow ); }

	// ------------------------------------------------------------------------
	// * Edits
	// ------------------------------------------------------------------------
	void DispatchEditName( int iRow )	{ m_pMCTreeView->EditName( iRow ); }
	void DispatchEditValue( int iRow )	{ m_pMCTreeView->EditValue( iRow ); }
	int GetLastChangedRow()				{ return m_pMCTreeView->GetLastChangedRow(); }
	void DispatchEditCompleted()		{ m_pMCTreeView->EditCompleted(); }
	void DispatchEditReset()			{ m_pMCTreeView->EditReset(); }
	bool IsDebugRow( int iRow )			{ return m_pMCTreeView->IsDebugRow( iRow ); }
	void DestroyEditCtrl();
	void MakeEditCtrlActive( const RECT &rect, const char *szText );
	void SetEditCtrlModified( bool bModified );
	BOOL GetEditCtrlModified();
	void HideEditCtrl();
	void CopySelection();
	void CutSelection();
	void PasteSelection();
	void EditCompleted();
	void InsertText( char *szText, long lSize );
	int GetEditText( char *szText, long lMaxSize, bool bCut );
	int GetSelectedText( char *szText, long lMaxSize );
	int CutSelectedText( char *szText, long lMaxSize );

	// ------------------------------------------------------------------------
	// * Selection management
	// ------------------------------------------------------------------------
	int GetSelectedRow()				{ return m_pMCTreeView->GetSelectedRow(); }
	void DispatchSetSelection( int iRow ) { m_pMCTreeView->SetSelection( iRow ); }
	void DispatchMoveSelectionUp()		{ m_pMCTreeView->MoveSelectionUp(); } 
	void DispatchMoveSelectionDown()	{ m_pMCTreeView->MoveSelectionDown();}
	void DispatchExpandSelection()		{ m_pMCTreeView->ExpandSelection();}
	void DispatchCollapseSelection()	{ m_pMCTreeView->CollapseSelection(); }
	BOOL DispatchDeleteSelection()		{ return m_pMCTreeView->DeleteSelection(); }

	// ------------------------------------------------------------------------
	// * Tree management
	// ------------------------------------------------------------------------
	void DispatchExpandTree( int iRow ) { m_pMCTreeView->ExpandTree( iRow ); }
	void DispatchCollapseTree( int iRow ) { m_pMCTreeView->CollapseTree( iRow ); }
	void DispatchSortRootItems()		{ m_pMCTreeView->SortRootItems(); }

	// ------------------------------------------------------------------------
	// * Mouse input
	// ------------------------------------------------------------------------
	bool IsPointInExpandCollapseBox( int iRow, int xPos, int yPos );
	int LeftMouseClick( int xPos, int yPos );
	int LeftMouseDblClick( int xPos, int yPos );

	// ------------------------------------------------------------------------
	// * Misc. methods
	// ------------------------------------------------------------------------
	BOOL AllowNameEdit()				{ return m_pMCTreeView->AllowNameEdit(); }
	BOOL ColorDirtyRow()				{ return m_pMCTreeView->ColorDirtyRow(); }
//	void DispatchTakeFocus()			{ m_pMCTreeView->TakeFocus(); }
	inline HWND GetListView()			{ return m_pListView; }
	inline HWND GetEditCtrl()			{ return m_pEditCtrl; }

	// ------------------------------------------------------------------------
	// * Event Handling Overridables ...
	// ------------------------------------------------------------------------
	virtual void OnKey( UINT vk, BOOL fDown, int cRepeat, UINT flags );

	virtual void Resize( int iWidht, int iHeight );

}; // MCTreeView
