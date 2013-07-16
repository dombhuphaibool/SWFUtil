#include "stdafx.h"
#include "WinMCTreeView.h"

// supress 'type cast' warnings of type
// C4311 pointer truncation from <a> to <b>
// C4312 conversion from <a> to <b> of greater size
// C4244 conversion from <a> to <b> possible loss of data
#pragma warning( disable : 4311 4312 4244 )

typedef struct WIN_CREATE_CTRL_PARAM {
	HINSTANCE appInst;
	HWND parent;
} WIN_CREATE_CTRL_PARAM;

#define WC_MCTREEVIEW				"WinMCTreeViewClass"

#define HEADER_DIVIDER_REGION		6

// Temp: TODO: This is currently hard-coded. Need to change this...
#define DEFAULT_LISTVIEW_WIDTH		400
#define DEFAULT_LISTVIEW_HEIGHT		300
#define DEFAULT_ROW_HEIGHT			12
#define DEFAULT_HEADER_HEIGHT		17

// Custom handling of key inputs
#define MCTREE_ENTER_KEY					13			// ASCII value of enter key ???
#define MCTREE_BACKSPACE_KEY				8			// ASCII value of backspace key ???
#define MCTREE_DELETE_KEY					46			// ASCII value of delete key ???
#define MCTREE_LEFT_ARROW_KEY				37
#define MCTREE_UP_ARROW_KEY					38
#define MCTREE_RIGHT_ARROW_KEY				39
#define MCTREE_DOWN_ARROW_KEY				40

#define ADJUST_COLUMN0_WIDTH( x )	{ if ( (x) > 0 ) x--; }
#define ADJUST_COLUMN1_WIDTH( x )	// { x += 50; }

LRESULT CALLBACK WinMCTreeViewWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK ListViewWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK EditCtrlWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

/******************************************************************************
 * ++
 *
 * Name:			TWinMCTreeView()
 *
 * Description:		Constructor
 *
 * --
 *****************************************************************************/
WinMCTreeView::WinMCTreeView( MCTreeView *pMCTreeView, TString strName, TString strValue )
{
	m_pMCTreeView = pMCTreeView;
	m_pListView = NULL;
	m_pEditCtrl = NULL;

	// Other initialization
	m_iColumn0Width = 0;
	m_iColumn1Width = 0;
	m_iRowHeight = 0;
	m_iBoxWidth = 0;
	m_iBoxHeight = 0;
	m_iHeaderHeight = 0;
	m_iInitialIndent = 0;
	m_iBoxYOffset = 0;
	m_iTextYOffset = 0;
	m_iPaintYOffset = 0;
	m_iScrollPos = 0;
	m_iVisibleRange = 0;

	// Initialization of column labels
	m_strName = strName;
	m_strValue = strValue;

	// Initialization of state variables
	m_bAdjustingColumn = FALSE;
	m_bClampColumnWidth = FALSE;

	// Create graphics related resources
	m_hPenBlack = CreatePen( PS_SOLID, 1, RGB(0,0,0) );
	m_hPenWhite = CreatePen( PS_SOLID, 1, INVERTED_SELECTION_COLOR );
	m_hPenRed = CreatePen( PS_SOLID, 1, LAST_CHANGED_COLOR );
	m_hPenLightRed = CreatePen( PS_SOLID, 1, LAST_CHANGED_HIGHLIGHT );
	m_hPenGrid = CreatePen( PS_SOLID, 1, GRID_COLOR );
	m_hPenHierarchy = CreatePen( PS_SOLID, 1, HIERARCHY_COLOR );

	m_hBrushWhite = CreateSolidBrush( RGB(255,255,255) );
	m_hBrushHighlight = CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT) );

	// Get Windows OS version, since we must adjust for 2 pixel offset
	// on Windows XP.
	// Note: Windows 2000 is VER_PLATFORM_WIN32_NT version 5.0
	//		 and Windows XP is VER_PLATFORM_WIN32_NT version 5.1
	m_osPlatform = Windows;
	OSVERSIONINFO osVersion;
	osVersion.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	if ( ::GetVersionEx( &osVersion ) )
	{
		if ( osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT )
		{
			if ( (osVersion.dwMajorVersion > 5) ||
				 ( (osVersion.dwMajorVersion == 5) &&
				   (osVersion.dwMinorVersion > 0) ) )
				m_osPlatform = WindowsXP;
		}
		else if ( osVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
		{
			if ( (osVersion.dwMajorVersion < 4) ||
				 ( (osVersion.dwMajorVersion == 4) &&
				   (osVersion.dwMinorVersion <= 10) ) )
				m_osPlatform = Windows98;
		}
	}

	// On Windows98 platform, if we pass in a null string as the
	// empty content, the OS puts in funky random characters...
	// So, instead we put in a dummy space character.
	if ( m_osPlatform == Windows98 )
	{
		m_szEmptyString[0] = ' ';
		m_szEmptyString[1] = 0;
	}
	else
	{
		m_szEmptyString[0] = 0;
	}

	// InitializeWindowClasses();
}

/******************************************************************************
 * ++
 *
 * Name:			~TWinMCTreeView()
 *
 * Description:		Destructor
 *
 * --
 *****************************************************************************/
WinMCTreeView::~WinMCTreeView()
{
	/*
	 * Clean up graphics resources that we've allocated
	 */
	if ( m_hPenBlack )
		DeleteObject( m_hPenBlack );

	if ( m_hPenWhite )
		DeleteObject( m_hPenWhite );

	if ( m_hPenRed )
		DeleteObject( m_hPenRed );

	if ( m_hPenLightRed )
		DeleteObject( m_hPenLightRed );

	if ( m_hPenGrid )
		DeleteObject( m_hPenGrid );

	if ( m_hPenHierarchy )
		DeleteObject( m_hPenHierarchy );

	if ( m_hBrushWhite )
		DeleteObject( m_hBrushWhite );

	if ( m_hBrushHighlight )
		DeleteObject( m_hBrushHighlight );
}

/*
 * Register our main window container for this control. We will return the 
 * HWND of the created window of this class to the control user. From the
 * user perspective, this HWND will refer to the multi-column tree control.
 */
void InitializeMCTreeWindowClass( HINSTANCE hInstance )
{
	WNDCLASSEX wc;

	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= (WNDPROC) WinMCTreeViewWndProc;
	wc.cbClsExtra		= NULL;
	wc.cbWndExtra		= NULL;
	wc.hInstance		= hInstance;
	wc.hIcon			= NULL;
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= WC_MCTREEVIEW;
	wc.hIconSm			= NULL;

	RegisterClassEx( &wc );
}

LRESULT CALLBACK WinMCTreeViewWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		case WM_NOTIFY:
			LPNMHDR lpnmhdr = (LPNMHDR) lParam;
			HWND hwndFrom = lpnmhdr->hwndFrom;
			UINT idFrom = lpnmhdr->idFrom;
			UINT actionCode = lpnmhdr->code;
			WinMCTreeView *pWinMCTreeView = NULL;

			switch ( actionCode )
			{
				// Changing column widths by dragging the header divider
				case HDN_ITEMCHANGED:
					{
						pWinMCTreeView = (WinMCTreeView *) GetWindowLong( hWnd, GWL_USERDATA );
						if ( pWinMCTreeView ) 
						{
							pWinMCTreeView->AdjustColumnWidth();
							pWinMCTreeView->DestroyEditCtrl();
						}
					}
					break;
				// End of dragging the header divider. Store the column widths.
				case HDN_ENDTRACK:
					{
						pWinMCTreeView = (WinMCTreeView *) GetWindowLong( hWnd, GWL_USERDATA );
						if ( pWinMCTreeView ) 
						{
							pWinMCTreeView->ClampColumnWidth();
						}
					}
/*
				// Double clicking on a listview item (either column)
				case LVN_ITEMACTIVATE:
					{
						LPNMITEMACTIVATE pnmIa = (LPNMITEMACTIVATE) lParam;
						int iItem = pnmIa->iItem;
						int iSubItem = pnmIa->iSubItem;

						pTestView = (TestView *) GetWindowLong( hWnd, GWL_USERDATA );
						if ( pTestView )
							pTestView->EditRow( iItem );
					}
					break;
*/
				// Custom drawing of the list view control.
				case NM_CUSTOMDRAW:
					pWinMCTreeView = (WinMCTreeView *) GetWindowLong( hWnd, GWL_USERDATA );
					if ( pWinMCTreeView && (hwndFrom == pWinMCTreeView->GetListView()) )
					{
						LPNMLVCUSTOMDRAW plvCD = (LPNMLVCUSTOMDRAW) lParam;
						DWORD dwDrawingStage = plvCD->nmcd.dwDrawStage;

						if ( dwDrawingStage & CDDS_PREPAINT )
						{
							if ( dwDrawingStage & CDDS_SUBITEM )
							{
								// 3. Draw each subitem
								pWinMCTreeView->DrawListSubItem( plvCD->nmcd.hdc, (int)plvCD->nmcd.dwItemSpec, plvCD->iSubItem );
								return CDRF_SKIPDEFAULT;
							}
							else
							{
								// 1. Request prepaint notification for each item
								pWinMCTreeView->StoreCurrColumnWidths();
								return CDRF_NOTIFYITEMDRAW;
							}
						} 
						else if ( dwDrawingStage & CDDS_ITEMPREPAINT )
						{
							// 2. Request prepaint notification for each subitem
							return CDRF_NOTIFYSUBITEMDRAW;
						}
					}
					break;

			}
			break;

	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

int WinMCTreeView::CreateControl( void *param )
{
	WIN_CREATE_CTRL_PARAM *pWinParam = (WIN_CREATE_CTRL_PARAM *) param;
	return( CreateControl(pWinParam->appInst, pWinParam->parent) );
}

/******************************************************************************
 * ++
 *
 * Name:			CreateControl()
 *
 * Description:		Create the window container for the multi-column tree 
 *					control. Then add our subviews as neccessary (in our case,
 *					the listview and the edit control).
 *
 * --
 *****************************************************************************/
int WinMCTreeView::CreateControl( HINSTANCE hAppInst, HWND hParentWnd )
{
	/*
	static bool bFirstTime = true;
	if (bFirstTime) {
		InitCommonControls();
		InitializeWindowClasses();
		bFirstTime = false;
	}
	*/

	m_hAppInst = hAppInst;
	m_pParentView = hParentWnd;

	RECT parentRect;
	GetClientRect( m_pParentView, &parentRect );

	int parentWidth = parentRect.right - parentRect.left;
	int parentHeight = parentRect.bottom - parentRect.top;
	
	m_pRootContainer = CreateWindowEx( 0,
									   WC_MCTREEVIEW,
									   "",
									   WS_CHILD | WS_VISIBLE | WS_GROUP,
									   0,
									   0,
									   DEFAULT_LISTVIEW_WIDTH,
									   parentHeight,
									   m_pParentView,
									   NULL,
									   m_hAppInst,
									   NULL );

	if ( !m_pRootContainer )
		return 0;

	SetWindowLong( m_pRootContainer, GWL_USERDATA, (LONG) this );

	if ( !CreateListView(m_pRootContainer) || !CreateEditCtrl(m_pListView) )
	{
		DestroyControl();
		return 0;
	}

	return 1;
}

/*
 * Clean-up and thing that we may have created in the Initialize routine.
 */
void WinMCTreeView::DestroyControl()
{
	if ( m_pRootContainer )
	{
		DestroyWindow( m_pRootContainer );
		m_pRootContainer = NULL;
	}

	if ( m_pListView )
	{
		DestroyWindow( m_pListView );
		m_pListView = NULL;
	}

	if ( m_pEditCtrl )
	{
		DestroyWindow( m_pEditCtrl );
		m_pEditCtrl = NULL;
	}
}

void WinMCTreeView::ListViewVScroll( HWND hWnd, UINT code, int pos )
{
	// If we were in edit mode, get out of it. We have to do this here because
	// Director Framework doesn't notify us of a LoseFocus() when the user clicks
	// on the scroll bar. 
	DestroyEditCtrl();

	int vPos = (code == SB_THUMBPOSITION) ? (pos) : (GetScrollPos( hWnd, SB_VERT ));
	SetScrollPosition( vPos );

	// Force a draw on visible items 
	RedrawVisibleItems();
}

#define WHEEL_DELTA		120		// Microsoft predefined (see docs)
BOOL WinMCTreeView::MouseWheel( HWND hWnd, WORD fwKeys, short zDelta, int xPos, int yPos )
{
	bool bPostMsg = TRUE;

	int iScrollCode = (zDelta > 0) ? SB_LINEUP : SB_LINEDOWN;

	// Figure out how much to scroll
	int minPos, maxPos;
	::GetScrollRange( hWnd, SB_VERT, &minPos, &maxPos );
	int vPos = GetScrollPos( hWnd, SB_VERT );
	int vWheel = zDelta / WHEEL_DELTA;
	vPos -= vWheel;

	// Clamp the new verticle scroll position
	if ( vPos < minPos )
	{
		vPos = minPos;
		bPostMsg = FALSE;
	}
	if ( vPos > maxPos )
	{
		vPos = maxPos;
		bPostMsg = FALSE;
	}

	// A post will cause a redraw through the VSCROLL handling code, so
	// we perform a smart post (ie, post only if the scroll pos actually changes)
	if ( bPostMsg )
	{
		int wParam = MAKEWPARAM( iScrollCode, vPos );
		::PostMessage( hWnd, WM_VSCROLL, wParam, 0 );
	}

	return TRUE;
}

LRESULT CALLBACK ListViewWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	bool bHandled = true;
	bool bCallWndProc = true;
	WinMCTreeView *pWinMCTreeView = (WinMCTreeView *) GetWindowLong( hWnd, GWL_USERDATA );
	int stop = 0;

	switch( message )
	{
		case WM_SIZE:
			ShowScrollBar( hWnd, SB_HORZ, FALSE );
			break;

		case WM_LBUTTONDOWN:
			{
				int xPos = LOWORD( lParam );
				int yPos = HIWORD( lParam );
				if ( pWinMCTreeView )
					pWinMCTreeView->LeftMouseClick( xPos, yPos );
			}
			break;

		case WM_LBUTTONDBLCLK:
			{
				int xPos = LOWORD( lParam );
				int yPos = HIWORD( lParam );
				if ( pWinMCTreeView )
					pWinMCTreeView->LeftMouseDblClick( xPos, yPos );
			}
			break;

		case WM_MOUSEWHEEL:
			{
				WORD fwKeys = LOWORD( wParam ); // GET_KEYSTATE_WPARAM( wParam );
				short zDelta = HIWORD( wParam ); // GET_WHEEL_DELTA_WPARAM( wParam );
				int xPos = LOWORD( lParam );
				int yPos = HIWORD( lParam );
				if ( pWinMCTreeView )
					pWinMCTreeView->MouseWheel( hWnd, fwKeys, zDelta, xPos, yPos );
			}
			break;

		case WM_VSCROLL:
			{
				UINT code = LOWORD( wParam );
				int pos = HIWORD( wParam );
				if ( pWinMCTreeView )
				{
					CallWindowProc( (WNDPROC)pWinMCTreeView->m_wndProcListView, hWnd, message, wParam, lParam );
					pWinMCTreeView->ListViewVScroll( hWnd, code, pos );
					bCallWndProc = false;
				}
			}
			break;

		case WM_COMMAND:
			{
				WORD wNotifyCode = HIWORD( wParam );
				WORD wID = LOWORD( wParam );
				HWND hwndCtl = (HWND) lParam;

				if ( wNotifyCode == EN_KILLFOCUS )
				{
					if ( pWinMCTreeView && (pWinMCTreeView->GetEditCtrl() == hwndCtl) )
					{
						pWinMCTreeView->DestroyEditCtrl();
					}
				}
				else if ( wNotifyCode == EN_CHANGE )
				{
					int stop = 1;
				}
			}
			break;

		case WM_KEYDOWN:
			if ( pWinMCTreeView )
			{
				pWinMCTreeView->OnKey( wParam, true, 0, 0 );
				bCallWndProc = false;
			}
			break;

		default:
			bHandled = false;

	}

	if ( pWinMCTreeView && bCallWndProc )
	{
		return CallWindowProc( (WNDPROC)pWinMCTreeView->m_wndProcListView, hWnd, message, wParam, lParam );			
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

int WinMCTreeView::CreateListView( HWND pParentView )
{
	m_pListView = CreateWindowEx( 0,
								  WC_LISTVIEW,
								  "",
								  WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_BORDER |
								  LVS_REPORT | LVS_SINGLESEL | // LVS_EDITLABELS | 
								  LVS_ALIGNLEFT,
								  0,
								  0,
								  DEFAULT_LISTVIEW_WIDTH,
								  DEFAULT_LISTVIEW_HEIGHT,
								  pParentView,
								  NULL,
								  m_hAppInst,
								  NULL );
	
	if ( !m_pListView )
		return 0;

	ListView_SetExtendedListViewStyleEx( m_pListView, /* LVS_EX_GRIDLINES | */ LVS_EX_FULLROWSELECT,
													  /* LVS_EX_GRIDLINES | */ LVS_EX_FULLROWSELECT );

	SetWindowLong( m_pListView, GWL_USERDATA, (LONG) this );
	m_wndProcListView = (WNDPROC) SetWindowLong( m_pListView, GWL_WNDPROC, (LONG) ListViewWndProc );

	// TODO ---> Clean up this....
	m_iColumn0Width = DEFAULT_LISTVIEW_WIDTH / 2;
	m_iColumn1Width = DEFAULT_LISTVIEW_WIDTH - m_iColumn0Width;

	int idx;
	char szText[256];
	LV_COLUMN	lvC;
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_TEXT;
	lvC.fmt = LVCFMT_LEFT;
	
	strncpy( szText, (const char*) m_strName, 256 );
	szText[255] = 0;
	lvC.pszText = szText;
	lvC.cchTextMax = lstrlen( szText );
	lvC.iSubItem = 0;
	lvC.cx = m_iColumn0Width;
	idx = ListView_InsertColumn( m_pListView, 0, &lvC );

	strncpy( szText, (const char*) m_strValue, 256 );
	szText[255] = 0;
	lvC.pszText = szText;
	lvC.cchTextMax = lstrlen( szText );
	lvC.iSubItem = 1;
	lvC.cx = m_iColumn1Width;
	idx = ListView_InsertColumn( m_pListView, 1, &lvC );

	ADJUST_COLUMN0_WIDTH( m_iColumn0Width );
	ADJUST_COLUMN1_WIDTH( m_iColumn1Width );

	InitializeDrawingDimensions();

	return 1;
}

LRESULT CALLBACK EditCtrlWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	WinMCTreeView *pWinMCTreeView = (WinMCTreeView *) GetWindowLong( hWnd, GWL_USERDATA );

	switch( message )
	{
		// case WM_KEYDOWN:

		case WM_CHAR:
			{
				if ( wParam == MCTREE_ENTER_KEY )
					ShowWindow( hWnd, SW_HIDE );
			}
			break;

	}
	if ( pWinMCTreeView )
	{
		return CallWindowProc( (WNDPROC)pWinMCTreeView->m_wndProcEditCtrl, hWnd, message, wParam, lParam );			
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

int WinMCTreeView::CreateEditCtrl( HWND pParentView )
{
	m_pEditCtrl = CreateWindowEx( 0,
								  "EDIT",
								  "",
								  WS_CHILD | /* WS_VISIBLE | */ WS_BORDER |
								  ES_AUTOHSCROLL,
								  0,
								  0,
								  0,
								  0,
								  pParentView,
								  NULL,
								  m_hAppInst,
								  NULL );
//	HFONT hFont = (HFONT) SendMessage( hwndEdit, WM_GETFONT, 0, 0 );
//	SendMessage( hwndEdit, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0) );

	if ( !m_pEditCtrl )
		return 0;

	SetWindowLong( m_pEditCtrl, GWL_USERDATA, (LONG) this );
	m_wndProcEditCtrl = (WNDPROC) SetWindowLong( m_pEditCtrl, GWL_WNDPROC, (LONG) EditCtrlWndProc );

	return 1;
}

/******************************************************************************
 * ++
 * 
 * Section:			Column Widths management
 *
 *
 * --
 *****************************************************************************/
void WinMCTreeView::AdjustColumnWidth() 
{
	int iListViewWidth = GetListViewWidth();
	if ( !m_bAdjustingColumn )
	{
		m_bAdjustingColumn = TRUE;
		int iNameColWidth = ListView_GetColumnWidth( GetListView(), 0 );

		if ( m_bClampColumnWidth )
		{
			// Clamp the name column width
			// int iColOneWidth = ListView_GetColumnWidth( GetListView(), 0 );
			if ( iNameColWidth > iListViewWidth )
				ListView_SetColumnWidth( GetListView(), 0, iListViewWidth );
			m_bClampColumnWidth = FALSE;
		}
		// Adjust the value column width
		m_iColumn1Width = (iListViewWidth > iNameColWidth) ? (iListViewWidth-iNameColWidth) : 0;
		ADJUST_COLUMN1_WIDTH( m_iColumn1Width );
		ListView_SetColumnWidth( GetListView(), 1, m_iColumn1Width );
		m_bAdjustingColumn = FALSE;
	}
}

void WinMCTreeView::ClampColumnWidth() 
{
	m_bClampColumnWidth = TRUE;
}

void WinMCTreeView::StoreCurrColumnWidths() 
{
	int iListViewWidth = GetListViewWidth();
	m_iColumn0Width = ListView_GetColumnWidth( GetListView(), 0 );
	m_iColumn1Width = (iListViewWidth > m_iColumn0Width) ? 
					   iListViewWidth - m_iColumn0Width : 0;

	ADJUST_COLUMN0_WIDTH( m_iColumn0Width );
	ADJUST_COLUMN1_WIDTH( m_iColumn1Width );
}

int WinMCTreeView::GetListViewWidth()
{
	int iWidth = 0;
	RECT rect;
	if ( GetWindowRect(m_pListView, &rect) )
		iWidth = rect.right - rect.left;
	return iWidth;
}

/******************************************************************************
 * ++
 * 
 * Section:			Custom Draw Routines
 *
 *
 * --
 *****************************************************************************/
void WinMCTreeView::InitializeDrawingDimensions()
{
	// Add a dummy list item so that a user may click on the last row,
	// or double-click on the last row to add new entries...
	// *** Also, this will allow us to initialize drawing dimensions
	AddListItem( 0, TString(m_szEmptyString), TString(m_szEmptyString) );

	RECT rRect;
	if ( ListView_GetItemRect(GetListView(), 0, &rRect, LVIR_BOUNDS) )
		m_iRowHeight = rRect.bottom - rRect.top;
	else
		m_iRowHeight = DEFAULT_ROW_HEIGHT;

	m_iBoxHeight = (int) (0.58 * ((float)m_iRowHeight));
	m_iBoxWidth = m_iBoxHeight;

	m_iTextYOffset = 2; // 0; // 1;		// Should always be 1 ???
	m_iPaintYOffset = 1; // -1; // 0; // 1;	// Should always be 1 ???

	// On WindowsXP there is no 2 pixel gap between the header bar and
	// the actual list view. So, get rid of the 2 pixel gap
	/* ***** >> This was for Director.exe only!!! Don't know why!!! << *****
	if ( m_osPlatform == WindowsXP )
	{
		m_iTextYOffset -= 2;
		m_iPaintYOffset -= 2;
	}
	*/

	m_iInitialIndent = (int) (0.36 * ((float)m_iRowHeight));
	m_iBoxYOffset = (int) (0.29 * ((float)m_iRowHeight));
	m_iBoxYOffset += m_iPaintYOffset;

	HWND hwndHeader = ListView_GetHeader( GetListView() );
	if ( hwndHeader )
	{
		if ( ::GetWindowRect(hwndHeader, &rRect) )
			m_iHeaderHeight = rRect.bottom - rRect.top;
	}

	if ( !m_iHeaderHeight )
		m_iHeaderHeight = DEFAULT_HEADER_HEIGHT;

	if ( !AllowNameEdit() )
		DeleteRow( 0 );
}

void WinMCTreeView::DrawNameColumnItem( HDC hdc, int iRow, RECT &box )
{
	if ( m_iColumn0Width )
	{
		BOOL bSucceeded = FALSE;
		RECT origBox = box;

		/*
		 * Clip the box if neccessary
		 */
		if ( m_iColumn0Width < box.left )
			box.left = m_iColumn0Width;
		if ( m_iColumn0Width < box.right )
			box.right = m_iColumn0Width;

		BOOL bNotRootItem = (GetDepth( iRow ) > 0);

		/*
		 * Draw expand or collapse box
		 */
		if ( HasChildren( iRow ) )
		{
			if ( bNotRootItem )
				DrawDash( hdc, iRow, SHORT_DASH, origBox );
			DrawBox( hdc, box );
			BOOL bDrawPlus = ! /* m_TreeData. */ GetExpanded( iRow );
			DrawPlusMinus( hdc, box, bDrawPlus );
		}
		else
		{
			if ( bNotRootItem )
				DrawDash( hdc, iRow, LONG_DASH, origBox );
		}

		/*
		 * Draw tree hierarchy lines
		 */
/*		if ( bNotRootItem && IsLastItem(iRow) )
		{
			DrawVerticalParentChildLine( hdc, iRow, origBox );
		}
*/
		/*
		 * Draw Text
		 */
		RECT rTextRect;
		GetTextRect( iRow, 0, rTextRect );
		char szText[LIST_MAX_DISPLAY_LEN];
		ListView_GetItemText( GetListView(), iRow, 0, szText, LIST_MAX_DISPLAY_LEN );
		szText[LIST_MAX_DISPLAY_LEN-1] = 0;
		// strcpy( szText, m_DataSource->GetName( m_TreeData.GetData(iRow) ) );
		int success = DrawText( hdc, szText, lstrlen(szText), &rTextRect, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX );
	}
}

void WinMCTreeView::DrawValueColumnItem( HDC hdc, int iRow )
{
	RECT rTextRect;
	GetTextRect( iRow, VALUE_COLUMN, rTextRect );
	char szText[LIST_MAX_DISPLAY_LEN];
	ListView_GetItemText( GetListView(), iRow, VALUE_COLUMN, szText, LIST_MAX_DISPLAY_LEN );
	szText[LIST_MAX_DISPLAY_LEN-1] = 0;
	int success = DrawText( hdc, szText, lstrlen(szText), &rTextRect, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX );
}

void WinMCTreeView::DrawDash( HDC hdc, int iRow, DashType dashType, RECT &box )
{
	int yPos = box.top + (m_iBoxHeight / 2);
	int xLeft = box.left - (m_iBoxWidth / 2);
	int xRight = 0;
	switch( dashType )
	{
		case SHORT_DASH:
			xRight = xLeft + (m_iBoxWidth / 2);
			break;

		case LONG_DASH:
			xRight = xLeft + m_iBoxWidth;
			break;
	}

	// Clip if neccessary
	if ( m_iColumn0Width < xLeft )
		xLeft = m_iColumn0Width;
	if ( m_iColumn0Width < xRight )
		xRight = m_iColumn0Width;
	
	/*
	 * Draw the sucker...
	 */
	HPEN hOldPen = (HPEN) SelectObject( hdc, m_hPenHierarchy );
	BOOL bSucceeded = FALSE;

	// Draw the horizontal line - "The Dash"
	bSucceeded = MoveToEx( hdc, xLeft, yPos, NULL );
	bSucceeded = LineTo( hdc, xRight, yPos );

	// Draw the vertical line(s)
	// First draw the first level line (if we're the last item then draw
	// an angle bracket '- otherwise draw a sideway T bracket |- 
	int yTop = box.top - m_iBoxYOffset;
	int yBottom = 0;
	if ( IsLastItem(iRow) )
		yBottom = yTop + m_iBoxYOffset + (m_iBoxHeight/2);
	else
		yBottom = yTop + m_iRowHeight;
	bSucceeded = MoveToEx( hdc, xLeft, yTop, NULL );
	bSucceeded = LineTo( hdc, xLeft, yBottom );

	// Draw the rest of the |'(s)
	yBottom = yTop + m_iRowHeight;
	// If this method is called, we must have a parent
	int iParentRow = GetParent( iRow );
	int iCurrDepth = GetDepth( iParentRow );
	for ( int i=0; i<iCurrDepth; i++ )
	{
		xLeft -= m_iBoxWidth;
		if ( !IsLastItem(iParentRow) )
		{
			bSucceeded = MoveToEx( hdc, xLeft, yTop, NULL );
			bSucceeded = LineTo( hdc, xLeft, yBottom );
		}
		iParentRow = GetParent( iParentRow );
	}
	SelectObject( hdc, hOldPen );
}

void WinMCTreeView::DrawVerticalParentChildLine( HDC hdc, int iRow, RECT &box )
{
	// If this method is called then we can assume that we definitely have a parent
	int iParentRow = GetParent( iRow );
	assert( iParentRow != -1 );

	if ( iParentRow < m_iScrollPos )
		iParentRow = m_iScrollPos;

	RECT parentBox;
	GetBoxRect( iParentRow, parentBox );

	int xPos = parentBox.left + (m_iBoxWidth / 2);
	int yTop = parentBox.bottom;
	int yBottom = box.top + (m_iBoxHeight / 2);

	// Clip if neccessary
	if ( m_iColumn0Width < xPos )
		xPos = m_iColumn0Width;

	// Draw the sucker...
	HPEN hOldPen = (HPEN) SelectObject( hdc, m_hPenHierarchy );
	BOOL bSucceeded = FALSE;
	bSucceeded = MoveToEx( hdc, xPos, yTop, NULL );
	bSucceeded = LineTo( hdc, xPos, yBottom );
	SelectObject( hdc, hOldPen );
}

void WinMCTreeView::DrawBox( HDC hdc, RECT &box )
{
	BOOL bSucceeded = FALSE;
	bSucceeded = MoveToEx( hdc, box.left, box.top, NULL );
	bSucceeded = LineTo( hdc, box.right, box.top );
	bSucceeded = LineTo( hdc, box.right, box.bottom );
	bSucceeded = LineTo( hdc, box.left, box.bottom );
	bSucceeded = LineTo( hdc, box.left, box.top );
	// TODO: Error Checking & Recovery ???
}

void WinMCTreeView::DrawPlusMinus( HDC hdc, RECT &box, BOOL plus )
{
	int plusY = box.top + (m_iBoxHeight/2);
	int iStartLeft = (m_iColumn0Width < (box.left+2)) ? m_iColumn0Width : (box.left+2);
	int iEndRight = (m_iColumn0Width < (box.right-1)) ? m_iColumn0Width : (box.right-1);

	BOOL bSucceeded = FALSE;
	bSucceeded = MoveToEx( hdc, iStartLeft, plusY, NULL );
	bSucceeded = LineTo( hdc, iEndRight, plusY );

	int plusX = box.left + (m_iBoxWidth/2);
	// Clip the plus X if neccessary
	plusX = ( m_iColumn0Width < plusX ) ? m_iColumn0Width : plusX;

	if ( plus )
	{
		bSucceeded = MoveToEx( hdc, plusX, box.top+2, NULL );
		bSucceeded = LineTo( hdc, plusX, box.bottom-1 );
	}
	else
	{
		// it's expanded so draw the vertical line underneath
		int bottom = box.bottom + (m_iRowHeight - (m_iBoxYOffset + m_iBoxHeight));
		HPEN hOldPen = (HPEN) SelectObject( hdc, m_hPenHierarchy );
		bSucceeded = MoveToEx( hdc, plusX, box.bottom, NULL );
		bSucceeded = LineTo( hdc, plusX, bottom );
		SelectObject( hdc, hOldPen );
	}
	// TODO: Error Checking & Recovery ???
}

void WinMCTreeView::FillBackground( HDC hdc, int iRow )
{
	RECT rect;
	rect.left = 0;
	rect.right = rect.left + GetListViewWidth();
	rect.top = m_iHeaderHeight + ((iRow-m_iScrollPos) * m_iRowHeight) + m_iPaintYOffset;
	rect.bottom = rect.top + m_iRowHeight;

	HBRUSH currBrush = (GetSelectedRow() == iRow) ? m_hBrushHighlight : m_hBrushWhite;
	HBRUSH hOldBrush = (HBRUSH) SelectObject( hdc, currBrush );
	FillRect( hdc, &rect, currBrush );
	SelectObject( hdc, hOldBrush );

	// Draw the grid
	HPEN hOldPen = (HPEN) SelectObject( hdc, m_hPenGrid );
	BOOL bSucceeded = FALSE;
	bSucceeded = MoveToEx( hdc, rect.left, rect.top, NULL );			// Draw the top horizontal line
	bSucceeded = LineTo( hdc, rect.right, rect.top );
	bSucceeded = MoveToEx( hdc, rect.left, rect.bottom, NULL );			// Draw the bottom horizontal line
	bSucceeded = LineTo( hdc, rect.right, rect.bottom );
//	bSucceeded = MoveToEx( hdc, m_iColumn0Width/*-1*/, rect.top, NULL );		// Draw the verticle line
//	bSucceeded = LineTo( hdc, m_iColumn0Width/*-1*/, rect.bottom );
	SelectObject( hdc, hOldPen );
}

void WinMCTreeView::DrawVerticalGridLine( HDC hdc, int iRow )
{
	int top = m_iHeaderHeight + ((iRow-m_iScrollPos) * m_iRowHeight) + m_iPaintYOffset;
	int bottom = top + m_iRowHeight;

	// Draw the grid
	HPEN hOldPen = (HPEN) SelectObject( hdc, m_hPenGrid );
	BOOL bSucceeded = FALSE;
	bSucceeded = MoveToEx( hdc, m_iColumn0Width, top, NULL );		// Draw the verticle line
	bSucceeded = LineTo( hdc, m_iColumn0Width, bottom );
	SelectObject( hdc, hOldPen );
}

void WinMCTreeView::GetBoxRect( int iRow, RECT &rBoxRect )
{
	int iIndent = GetDepth( iRow ) * (m_iBoxWidth);

	rBoxRect.left = m_iInitialIndent + iIndent;
	rBoxRect.right = rBoxRect.left + m_iBoxWidth;
	rBoxRect.top = m_iHeaderHeight + ((iRow-m_iScrollPos) * m_iRowHeight) + m_iBoxYOffset;
	rBoxRect.bottom = rBoxRect.top + m_iBoxHeight;
}

void WinMCTreeView::GetTextRect( int iRow, int iCol, RECT &rTextRect )
{
	// Note: For some reason we have to add 1 to left and right to 
	// correctly align the text rect.
	if ( iCol == 0 )
	{
		int iIndent = (/* m_TreeData. */ GetDepth(iRow) + 1) * (m_iBoxWidth);
		rTextRect.left = m_iInitialIndent + iIndent + m_iInitialIndent + 1;
		rTextRect.right = m_iColumn0Width + 1;
	}
	else  // iCol == 1
	{
		rTextRect.left = m_iColumn0Width + m_iInitialIndent; // + 1;
		rTextRect.right = GetListViewWidth() + 1;
	}

	rTextRect.top = m_iHeaderHeight + ((iRow-m_iScrollPos) * m_iRowHeight) + m_iTextYOffset;
	rTextRect.bottom = rTextRect.top + m_iRowHeight;
}

void WinMCTreeView::DrawListSubItem( HDC hdc, int iRow, int iColumn )
{
	// A hack for now. Since with Director, after you expand
	// a selection, the initial mouse over causes the framework to
	// pass a redraw request *just for the name column*. So, let's
	// only draw on value column redraw requests. :P Hacky hack hack :(
	if ( iColumn == 0 )
		return;

	RECT box;
	GetBoxRect( iRow, box );

	/* 
	 * Clear the background with the appropriate color
	 */
	FillBackground( hdc, iRow );

	/*
	 * Set the appropriate pen color (for drawing the +/- box)
	 * and the appropriate text color for the name/value labels.
	 */
	COLORREF oldTextColor = GetTextColor( hdc );

	HPEN currPen = NULL;
	// if ( iRow == GetLastChangedRow() )
	if ( ColorDirtyRow() && IsDirty(iRow) )
	{
		if ( iRow == GetSelectedRow() )
		{
			currPen = m_hPenLightRed;
			SetTextColor( hdc, LAST_CHANGED_HIGHLIGHT );
		}
		else 
		{
			currPen = m_hPenRed;
			SetTextColor( hdc, LAST_CHANGED_COLOR );
		}
	}
	else if ( iRow == GetSelectedRow() )
	{
		currPen = m_hPenWhite;
		SetTextColor( hdc, INVERTED_SELECTION_COLOR );		
	}
	else
		currPen = m_hPenBlack;

	HPEN hOldPen = (HPEN) SelectObject( hdc, currPen );

	/*
	 * Draw the graphics
	 */
	DrawNameColumnItem( hdc, iRow, box );
	DrawValueColumnItem( hdc, iRow );

	/*
	 * Draw the verticle grid line last so that it draws
	 * over the plus/minus sign, etc.
	 */
	DrawVerticalGridLine( hdc, iRow );

	/*
	 * Revert back to the original pen & text colors
	 */
	SetTextColor( hdc, oldTextColor );
	SelectObject( hdc, hOldPen );

}

void WinMCTreeView::RedrawVisibleItems()
{
	int iFirstRedraw = m_iScrollPos; // - 1;
//	iFirstRedraw = (iFirstRedraw < 0) ? 0 : iFirstRedraw;

	int iNumItems = ListView_GetItemCount( GetListView() );
	int iLastRedraw = (iNumItems <= m_iVisibleRange) ? iNumItems - 1 : m_iScrollPos + m_iVisibleRange;
	if ( iLastRedraw > (iNumItems-1) )
		iLastRedraw = iNumItems - 1;

	ListView_RedrawItems( GetListView(), iFirstRedraw, iLastRedraw );
}

void WinMCTreeView::UpdateVisibleRange( int iNewHeight )
{
	m_iVisibleRange = ((iNewHeight-m_iHeaderHeight) / m_iRowHeight);
	int iRemainder = ((iNewHeight-m_iHeaderHeight) % m_iRowHeight);
	if ( iRemainder > (m_iRowHeight/2) )
		m_iVisibleRange++;
	
	if ( m_iVisibleRange < 0 )
		m_iVisibleRange = 0;
}

void WinMCTreeView::SetScrollPosition( int vPos )
{
	m_iScrollPos = vPos;
}

void WinMCTreeView::UpdateScrollPosition()
{
	SetScrollPosition( GetScrollPos( GetListView(), SB_VERT ) );
}

void WinMCTreeView::ClearControl()
{
	int iNumItems = ListView_GetItemCount( GetListView() );
	if ( AllowNameEdit() )
		iNumItems--;
	for ( int i=0; i<iNumItems; i++ )
		DeleteRow( 0 );
}

/******************************************************************************
 * ++
 * 
 * Section:			Mouse Interaction
 *
 *
 * --
 *****************************************************************************/
bool WinMCTreeView::IsPointInExpandCollapseBox( int iRow, int xPos, int yPos )
{
	bool bPointInBox = false;

	RECT box;
	GetBoxRect( iRow, box );

	// Grow the hotspot click test by 1 on each side...
	box.left = box.left - 2;
	box.right = box.right + 2;
	box.top = box.top - 2;
	box.bottom = box.bottom + 2;

	if ( (xPos>box.left) && (xPos<box.right) && (yPos>box.top) && (yPos<box.bottom) )
	{
		bPointInBox = true;
	}

	return bPointInBox;
}

int WinMCTreeView::LeftMouseClick( int xPos, int yPos )
{
	LVHITTESTINFO lvHitTestInfo;
	lvHitTestInfo.pt.x = xPos;
	lvHitTestInfo.pt.y = yPos;
	lvHitTestInfo.flags = LVHT_ONITEM;

	int itemIndex = ListView_HitTest( GetListView(), &lvHitTestInfo );
	if ( itemIndex > -1 )
	{
		if ( HasChildren( itemIndex ) )
		{
			if ( IsPointInExpandCollapseBox( itemIndex, xPos, yPos ) )
			{
				BOOL isExpanded = GetExpanded( itemIndex );
				if ( isExpanded )
				{
					DispatchCollapseTree( itemIndex );
					// In the case that the view has a scroll bar and it shrinks
					// because items are collapsed. And we are looking at the bottom
					// of the list. The items will be reorganized (ie, everything 
					// pushed up, so we need to do a redraw.
					UpdateScrollPosition();
					RedrawVisibleItems();
				}
				else
				{
					DispatchExpandTree( itemIndex );
				}

				SetExpanded( itemIndex, !isExpanded );
			}
			else
			{
				DispatchSetSelection( itemIndex );
			}
		}
		else
		{
			DispatchSetSelection( itemIndex );
		}
		ListView_Update( GetListView(), itemIndex );
	}
	else 
	{
		DispatchSetSelection( -1 );
	}

	DestroyEditCtrl();

	return 1;
}

int WinMCTreeView::LeftMouseDblClick( int xPos, int yPos )
{
	LVHITTESTINFO lvHitTestInfo;
	lvHitTestInfo.pt.x = xPos;
	lvHitTestInfo.pt.y = yPos;
	lvHitTestInfo.flags = LVHT_ONITEM;

	int itemIndex = ListView_HitTest( GetListView(), &lvHitTestInfo );
	if ( (itemIndex > -1) && (!IsDebugRow(itemIndex)) )
	{
		// Exclude the plus/minus (expand/collapse) box from double clicking to
		// enter edit mode...
		if ( IsPointInExpandCollapseBox( itemIndex, xPos, yPos ) )
		{
			return LeftMouseClick( xPos, yPos );
		}

		// If we're here, then the double-click did not occur in the plus/minus
		// (expand/collapse) box, so continue with our double-clicking logic.
		if ( xPos > m_iColumn0Width )
		{
			// The user is trying to edit the value column
			// Allow user to dbl-click anywhere in the last row for
			// a Name Editable tree view to append item for input
			if ( AllowNameEdit() )
			{
				if ( itemIndex == (ListView_GetItemCount(GetListView())-1) )
					DispatchEditName( itemIndex );
				else
					DispatchEditValue( itemIndex );
			}
			else
			{
				DispatchEditValue( itemIndex );	
			}
		}
		else
		{
			// The user is trying to edit the name column
			// We only allow it if Name Edit is allowed *AND* 
			// (it's the last row or it's a root item)
			if ( AllowNameEdit() && 
				  
				 ( (itemIndex == (ListView_GetItemCount(GetListView())-1)) ||
				   (GetDepth(itemIndex) == 0) ) 
				 )

			{
				DispatchEditName( itemIndex );
			}
		}

	}

	return 1;
}

/******************************************************************************
 * ++
 * 
 * Section:			Tree Data Management
 *
 *
 * --
 *****************************************************************************/
int WinMCTreeView::AddListItem( int iRow, const TString &szCol0, const TString &szCol1 )
{
	int retIdx = -1;

	char szColumn0[LIST_MAX_DISPLAY_LEN];
	char szColumn1[LIST_MAX_DISPLAY_LEN];

	strncpy( szColumn0, (const char *) szCol0, LIST_MAX_DISPLAY_LEN );
	strncpy( szColumn1, (const char *) szCol1, LIST_MAX_DISPLAY_LEN );
	szColumn0[LIST_MAX_DISPLAY_LEN-1] = 0;
	szColumn1[LIST_MAX_DISPLAY_LEN-1] = 0;

	LV_ITEM lvI;

	lvI.mask = LVIF_TEXT | LVIF_STATE;
	lvI.state = 0;
	lvI.stateMask = 0;
	lvI.iItem = iRow;
	lvI.iSubItem = 0;
	lvI.pszText = szColumn0;
	lvI.cchTextMax = lstrlen( szColumn0 );

	retIdx = ListView_InsertItem( GetListView(), &lvI );
	ListView_SetItemText( GetListView(), iRow, 1, szColumn1 );

	return retIdx;
}

/******************************************************************************
 * ++
 *
 * Section:			Edits
 *
 * --
 *****************************************************************************/
void WinMCTreeView::EditName( int iRow )
{
	if ( m_pEditCtrl )
	{
		RECT rTextRect;
		GetTextRect( iRow, 0, rTextRect );

		char szText[LIST_MAX_DISPLAY_LEN];
		ListView_GetItemText( GetListView(), iRow, 0, szText, LIST_MAX_DISPLAY_LEN );
		szText[LIST_MAX_DISPLAY_LEN-1] = 0;
		MakeEditCtrlActive( rTextRect, szText );
	}
}

void WinMCTreeView::EditValue( int iRow )
{
	if ( m_pEditCtrl && GetData(iRow) )
	{
		RECT rTextRect;
		GetTextRect( iRow, 1, rTextRect );
		rTextRect.left -= m_iInitialIndent;

		char szText[LIST_MAX_DISPLAY_LEN];
		ListView_GetItemText( GetListView(), iRow, 1, szText, LIST_MAX_DISPLAY_LEN );
		szText[LIST_MAX_DISPLAY_LEN-1] = 0;
		MakeEditCtrlActive( rTextRect, szText );
	}
}

void WinMCTreeView::BringEditCtrlToFront()
{
	if ( m_pEditCtrl )
		BringWindowToTop( m_pEditCtrl );
}

void WinMCTreeView::GetEditText( char *szBuffer, unsigned int iBufferSize )
{
	GetWindowText( m_pEditCtrl, szBuffer, iBufferSize );
}

void WinMCTreeView::DestroyEditCtrl()
{
	if ( m_pEditCtrl )
	{
		BOOL bModified = GetEditCtrlModified(); 
		if ( bModified )
		{
			DispatchEditCompleted();
			SetEditCtrlModified( FALSE );
		}

		HideEditCtrl();
		DispatchEditReset();
	}
}

/******************************************************************************
 * ++
 *
 * Section:			Row updates, etc.
 *
 * --
 *****************************************************************************/
void WinMCTreeView::UpdateRow( int iRow )
{
	// ListView_Update( GetListView(), iRow );
	ListView_RedrawItems( GetListView(), iRow, iRow );
}

void WinMCTreeView::DeleteRow( int iRow )
{
	ListView_DeleteItem( GetListView(), iRow );
}

void WinMCTreeView::SetItemText( int iRow, int iCol, const TString &szText )
{
	char szColText[LIST_MAX_DISPLAY_LEN];
	strncpy( szColText, (const char*) szText, LIST_MAX_DISPLAY_LEN );
	szColText[LIST_MAX_DISPLAY_LEN-1] = 0;

	ListView_SetItemText( GetListView(), iRow, iCol, szColText );
}

/******************************************************************************
 * ++
 *
 * Name:			SetSize()
 *
 * Description:		If the container size changes, only resize the value 
 *					column to fit the additional space.
 *
 * --
 *****************************************************************************/
void WinMCTreeView::SetSize( int iWidth, int iHeight )
{
	// In the case that a scroll bar is showing, we are scrolled all the way
	// at the bottom (ie, the scroll thumb is at the bottom). The user then
	// resizes the window by stretching it down. The data should then update
	// reflecting to the new scroll range.
	UpdateScrollPosition();

	m_iColumn1Width = iWidth - m_iColumn0Width;
	m_iColumn1Width = (m_iColumn1Width < 0) ? 0 : m_iColumn1Width;
	ADJUST_COLUMN1_WIDTH( m_iColumn1Width );

	// When we change size & maximize we don't need to adjust the column widths
	// The column width routine queries the list view for its size and will get
	// the old size and not the new size...
	m_bAdjustingColumn = TRUE;
	ListView_SetColumnWidth( GetListView(), 1, m_iColumn1Width );
	m_bAdjustingColumn = FALSE;
}

/******************************************************************************
 * ++
 *
 * Name:			SetNameColumnWidth()
 *
 * Description:		Resize the name column width, then resize the value column
 *					accordingly.
 *
 * --
 *****************************************************************************/
void WinMCTreeView::SetNameColumnWidth( int iWidth )
{
	m_iColumn0Width = iWidth;
	ListView_SetColumnWidth( GetListView(), 0, m_iColumn0Width );
	ADJUST_COLUMN0_WIDTH( m_iColumn0Width );
}

/******************************************************************************
 * ++
 *
 * Name:			OnKey()
 *
 * Description:		Handle key events when we have the input focus. The
 *					only key inputs we handle right now is moving the selected
 *					row with the arrow keys.
 *
 * -- 
 *****************************************************************************/
void WinMCTreeView::OnKey( UINT vk, BOOL fDown, int cRepeat, UINT flags )
{
	// Only respond to key event if there is a row selected...
	if ( fDown && (GetSelectedRow() > -1) )
	{
		BOOL bCheckScroll = FALSE;
		switch ( vk )
		{
			case MCTREE_LEFT_ARROW_KEY:
				DispatchCollapseSelection();
				// In the case that the view has a scroll bar and it shrinks
				// because items are collapsed. And we are looking at the bottom
				// of the list. The items will be reorganized (ie, everything 
				// pushed up, so we need to do a redraw.
				UpdateScrollPosition();
				RedrawVisibleItems();
				break;

			case MCTREE_UP_ARROW_KEY:
				DispatchMoveSelectionUp();
				bCheckScroll = TRUE;
				break;
				
			case MCTREE_RIGHT_ARROW_KEY:
				DispatchExpandSelection();
				break;
				
			case MCTREE_DOWN_ARROW_KEY:
				DispatchMoveSelectionDown();
				bCheckScroll = TRUE;
				break;

			case MCTREE_BACKSPACE_KEY:
			case MCTREE_DELETE_KEY:
				// DeleteSelection returns TRUE if the selected item was deletable and
				// we actually deleted it. If that happened, then update & redraw...
				if ( DispatchDeleteSelection() )
				{
					UpdateScrollPosition();
					RedrawVisibleItems();
				}
				break;
		}

		if ( bCheckScroll )
		{
			int iScrollUp = m_iScrollPos - GetSelectedRow();
			int iScrollDown = GetSelectedRow() - (m_iScrollPos + m_iVisibleRange - 1);
			if ( iScrollUp > 0 )
			{
				ListView_Scroll( GetListView(), 0, -(iScrollUp * m_iRowHeight) );
				m_iScrollPos-= iScrollUp;
				RedrawVisibleItems();
			}
			else if ( iScrollDown > 0 )
			{
				ListView_Scroll( GetListView(), 0, (iScrollDown * m_iRowHeight) );
				m_iScrollPos+= iScrollDown;
				RedrawVisibleItems();
			}

		}
	}
}


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%% -------------------------------- %%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%	You are now entering TWinCustomListView territory... %%%%%%%%%%	
// %%%%%%%%%%%%%%%%%%%%% -------------------------------- %%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


/******************************************************************************
 * ++
 *
 * Section:			TWinCustomListView
 *
 * --
 *****************************************************************************/
/*
void TWinCustomListView::OnSize( HWND hWnd, UINT state, int cx, int cy )
{
	ShowScrollBar( hWnd, SB_HORZ, FALSE );
	m_pWinMCTreeView->UpdateVisibleRange( cy );
}
*/

/*
void TWinCustomListView::UpdateItemUnderMouse( int xPos, int yPos )
{
	LVHITTESTINFO lvHitTestInfo;
	lvHitTestInfo.pt.x = xPos;
	lvHitTestInfo.pt.y = yPos;
	lvHitTestInfo.flags = LVHT_ONITEM;

	int itemIndex = ListView_HitTest( GetHWND(), &lvHitTestInfo );
	if ( itemIndex >= 0 )
		m_pWinMCTreeView->UpdateRow( itemIndex );
}
*/

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%% -------------------------------- %%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%  You are now entering TWinCustomEditCtrl territory...  %%%%%%%%%%	
// %%%%%%%%%%%%%%%%%%%%% -------------------------------- %%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


/******************************************************************************
 * ++
 *
 * Section:			EditCtrl
 *
 * --
 *****************************************************************************/
void WinMCTreeView::MakeEditCtrlActive( const RECT &rect, const char *szText )
{
	// Note we assume that we already took focus :)
	SetWindowPos( m_pEditCtrl, 
				  HWND_TOP, 
				  rect.left, 
				  rect.top, 
				  rect.right - rect.left, 
				  rect.bottom - rect.top + 1, 
				  SWP_SHOWWINDOW );

	BringWindowToTop( m_pEditCtrl );
	SetWindowText( m_pEditCtrl, szText );
}

/*
void TWinCustomEditCtrl::Redraw()
{
	BringToFront();
	// ::RedrawWindow( GetHWND(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW );
	fpNativeView->GetFrameworkView()->SetVisible( true );
	fpNativeView->GetFrameworkView()->DrawNow();	// Remove this line to remove the flickering...
													// But then you'll have no bottom or top line
													// Don't know why the system is clearing the 
													// background once it gets a ListView_SetText() msg...
}
*/

BOOL WinMCTreeView::GetEditCtrlModified()
{
	return( (BOOL) SendMessage( m_pEditCtrl, EM_GETMODIFY, 0, 0 ) );
}

void WinMCTreeView::SetEditCtrlModified( bool bModified )
{
	SendMessage( m_pEditCtrl, EM_SETMODIFY, bModified ? TRUE : FALSE, 0 );
}

void WinMCTreeView::HideEditCtrl()
{
	ShowWindow( m_pEditCtrl, SW_HIDE );
}

void WinMCTreeView::CopySelection()
{
	SendMessage( m_pEditCtrl, WM_COPY, 0, 0 );
}

void WinMCTreeView::CutSelection()
{
	SendMessage( m_pEditCtrl, WM_CUT, 0, 0 );
}

void WinMCTreeView::PasteSelection()
{
	SendMessage( m_pEditCtrl, WM_PASTE, 0, 0 );
}

// ----------------------------------------------------------------------------
// * Event handling
// ----------------------------------------------------------------------------
void WinMCTreeView::EditCompleted()
{
	DestroyEditCtrl();
}

void WinMCTreeView::InsertText( char *szText, long lSize )
{
	char *szNewText = (char *) malloc( sizeof(char)*(lSize+1) );
	assert( szNewText );	// otherwise out of memory
	if ( szNewText )
	{
		strncpy( szNewText, szText, lSize );
		szNewText[lSize] = 0;
		SendMessage( m_pEditCtrl, EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) szNewText );
		free( szNewText );
	}
}

int WinMCTreeView::GetEditText( char *szText, long lMaxSize, bool bCut )
{
	int iStartPos;
	int iEndPos;
	SendMessage( m_pEditCtrl, EM_GETSEL, (WPARAM) &iStartPos, (LPARAM) &iEndPos );
	int iSelectionLen = iEndPos - iStartPos;
	assert( iSelectionLen >= 0 );
	if ( iSelectionLen )
	{
		int iAllTextLen = GetWindowTextLength( m_pEditCtrl );
		char *szAllText = (char *) malloc( sizeof(char) * (iAllTextLen+1) );
		assert( szAllText );	// otherwise out of memory
		if ( szAllText )
		{
			if ( GetWindowText( m_pEditCtrl, szAllText, iAllTextLen+1 ) )
			{
				char *szSelectionText = (char *) malloc( sizeof(char) * (iSelectionLen+1) );
				assert( szSelectionText );	// otherwise out of memory
				if ( szSelectionText )
				{
					char *pCopyFrom = szAllText + iStartPos;
					memcpy( szSelectionText, pCopyFrom, iSelectionLen );
					szSelectionText[iSelectionLen] = 0;
					strncpy( szText, szSelectionText, lMaxSize );
					szText[lMaxSize-1] = 0;
					free( szSelectionText );

					if ( bCut )
					{	
						char *pPreserveFrom = pCopyFrom + iSelectionLen;
						int iPreserveLen = iAllTextLen - iEndPos;
						if ( iPreserveLen )
						{
							char *szPreserveText = (char *) malloc( sizeof(char) * (iPreserveLen+1) );
							assert( szPreserveText );	// otherwise out of memory
							if ( szPreserveText )
							{
								memcpy( szPreserveText, pPreserveFrom, iPreserveLen );
								memcpy( pCopyFrom, szPreserveText, iPreserveLen );
								pCopyFrom += iPreserveLen;
								free( szPreserveText );
							}
						}
						*pCopyFrom = 0;
						SetWindowText( m_pEditCtrl, szAllText );
						SendMessage( m_pEditCtrl, EM_SETSEL, (WPARAM) iStartPos, (LPARAM) iStartPos );
						SetEditCtrlModified( TRUE );
					}
				}
			}
			free( szAllText );
		}
	}

	return iSelectionLen;
}

int WinMCTreeView::GetSelectedText( char *szText, long lMaxSize )
{
	return GetEditText( szText, lMaxSize, FALSE );
}

int WinMCTreeView::CutSelectedText( char *szText, long lMaxSize )
{
	return GetEditText( szText, lMaxSize, TRUE );
}

void WinMCTreeView::Resize( int iWidth, int iHeight )
{
	
	SetWindowPos( m_pRootContainer, HWND_TOP, 0, 0, iWidth, iHeight, SWP_NOMOVE | SWP_SHOWWINDOW );
	SetWindowPos( m_pListView, HWND_TOP, 0, 0, iWidth, iHeight, SWP_NOMOVE | SWP_SHOWWINDOW );
	// ShowScrollBar( m_pListView, SB_HORZ, FALSE );
	UpdateVisibleRange( iHeight );

	SetSize( iWidth, iHeight );
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%% -------------------------------- %%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%    You are now entering TWinCustomHeader territory...  %%%%%%%%%%	
// %%%%%%%%%%%%%%%%%%%%% -------------------------------- %%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


/******************************************************************************
 * ++
 *
 * Section:			TWinCustomHeader
 *
 * --
 *****************************************************************************/
/*
bool TWinCustomHeader::OnParentNotify( HWND hWnd, int id, LPNMHDR lpinfo, LRESULT* pRetVal )
{
	bool bHandled = false;
	HWND hwndFrom = lpinfo->hwndFrom;
	UINT idFrom = lpinfo->idFrom;
	UINT actionCode = lpinfo->code;

	switch ( actionCode )
	{
		// Changing column widths by dragging the header divider
		case HDN_ITEMCHANGED:
			{
				m_pWinMCTreeView->AdjustColumnWidth();
				m_pWinMCTreeView->DestroyEditCtrl();

				bHandled = true;
			}
			break;
				
		// End of dragging the header divider. Store the column widths.
		case HDN_ENDTRACK:
			{
				m_pWinMCTreeView->ClampColumnWidth();
				bHandled = true;
			}
			break;

		case HDN_ITEMCLICK:
			{
				fpNativeView->GetFrameworkView()->TakeFocus();
				m_pWinMCTreeView->DispatchSortRootItems();
			}
			break;
	}

	return bHandled;
}

// When a user clicks on the header bar, we only show the push-button effect
// for the name column. This is because the user can sort the root items
// by clicking on the name column of the header bar. Clicking on the value
// column does nothing, so we short-circuit the windows message by *NOT* 
// passing it to the inherited framework code.
// 
// Passing the windows message to the framework will cause a WM_NOTIFY
// message with HDN_ITEMCLICK (see above), and is where we tell the
// TWinMCTreeView to perform the actual sorting.
void TWinCustomHeader::OnMouseDown( HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags )
{
	RECT rNameColRect;
	if ( Header_GetItemRect( GetHWND(), 0, &rNameColRect ) )
	{
		if ( (x >= rNameColRect.left) && (x <= (rNameColRect.right + HEADER_DIVIDER_REGION)) )
		{
			TWinCustomHeader_inherited::OnMouseDown( hWnd, fDoubleClick, x, y, keyFlags );
		}
	}
}

*/