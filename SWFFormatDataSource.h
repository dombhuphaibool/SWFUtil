#pragma once

#include "DataSource.h"
#include "TString.h"

class SWFFormatDataSource : public DataSource
{
public:
	SWFFormatDataSource();
	~SWFFormatDataSource();

	// Overridden methods
	virtual void *FindItemByName( const TString &szName, 
								  const TString &szInternalName,
								  long lHandlerIdx = -1,
								  long lVariableIdx = -1 );
	virtual int GetNumChildren( const void *pItem );
	virtual void *GetChild( const void *pItem, int childIdx );
	virtual bool HasChildren( const void *pItem );
	virtual TString GetName( const void *pItem, bool bReevaluate = false );
	virtual TString GetValue( const void *pItem, bool bReevaluate = false );
	virtual int SetValue( const void *pItem, const TString &szValue );
	virtual short HasStringRepValue( const void *pItem );
	virtual long IsDirty( const void *pItem );
	virtual bool ToggleNonLeafDisplayValue( const void *pItem );

	// Temp stuff
	void RemoveAllChildren( void *pParent ) {}
	TString GetLingoName( void *pData ) { return TString::Null(); }
	void RemoveItem( void *pData ) {}
	bool IsDebugRow( void *pData ) { return false; }
	void *GetParent( void *pData ) { return NULL; }
	void SetParent( void *pData, void *pParent ) {}
};

