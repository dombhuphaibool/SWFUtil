#pragma once

#include "TString.h"

class NativeMCTreeView
{
public:
	// ------------------------------------------------------------------------
	// * Initialization
	// ------------------------------------------------------------------------
	virtual int CreateControl( void *param ) = 0;
	virtual void DestroyControl() = 0;

	virtual void SetNameColumnWidth( int iWidth ) = 0;
	virtual void ClearControl() = 0;

	// ------------------------------------------------------------------------
	// * MC-TreeView Data Management 
	// ------------------------------------------------------------------------
	virtual int AddListItem( int iRow, const TString &szCol0, const TString &szCol1 ) = 0;
	virtual void SetItemText( int iRow, int iCol, const TString &szText ) = 0;

	virtual void UpdateRow( int iRow ) = 0;
	virtual void DeleteRow( int iRow ) = 0;

	// ------------------------------------------------------------------------
	// * Edits
	// ------------------------------------------------------------------------
	virtual void EditName( int iRow ) = 0;
	virtual void EditValue ( int iRow ) = 0;
	virtual void GetEditText( char *szBuffer, unsigned int iBufferSize ) = 0;
	virtual void BringEditCtrlToFront() = 0;

	// ------------------------------------------------------------------------
	// * Scrolling, etc.
	// ------------------------------------------------------------------------
	virtual void UpdateScrollPosition() = 0;

	// ------------------------------------------------------------------------
	// * Hacky whacky crap...
	// ------------------------------------------------------------------------
	virtual bool UseSpaceAsEmptyString() = 0;

	virtual void Resize( int iWidth, int iHeight ) = 0;
};

