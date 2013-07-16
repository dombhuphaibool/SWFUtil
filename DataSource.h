#pragma once

class TString;

class DataSource
{

public:
	typedef enum DirtyType
	{
		NOT_DIRTY		= 0x0000,
		NAME_DIRTY		= 0x0001,
		VALUE_DIRTY		= 0x0002,
		STRUCT_DIRTY	= 0x0004
	} DirtyType;

	// szName is the display name. If szInternalName is not specified, then we use szName
	// also as the internal name :)
	virtual void *FindItemByName( const TString &szName, 
								  const TString &szInternalName,
								  long lHandlerIdx = -1,
								  long lVariableIdx = -1 ) = 0;
	virtual int GetNumChildren( const void *pItem ) = 0;
	virtual void *GetChild( const void *pItem, int childIdx ) = 0;
	virtual bool HasChildren( const void *pItem ) = 0;
	virtual TString GetName( const void *pItem, bool bReevaluate = false ) = 0;
	virtual TString GetValue( const void *pItem, bool bReevaluate = false ) = 0;
	virtual int SetValue( const void *pItem, const TString &szValue ) = 0;
	virtual short HasStringRepValue( const void *pItem ) = 0;
	virtual long IsDirty( const void *pItem ) = 0;
	virtual bool ToggleNonLeafDisplayValue( const void *pItem ) = 0;
};
