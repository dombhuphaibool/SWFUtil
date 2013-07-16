#include "stdafx.h"
#include "SWFFormatDataSource.h"
#include "DisplayObject.h"

SWFFormatDataSource::SWFFormatDataSource()
{

}

SWFFormatDataSource::~SWFFormatDataSource()
{

}

void *SWFFormatDataSource::FindItemByName( const TString &szName, 
										   const TString &szInternalName,
										   long lHandlerIdx		/* = -1 */,
										   long lVariableIdx	/* = -1 */) 
{
	return NULL;
}

int SWFFormatDataSource::GetNumChildren( const void *pItem )
{
	int iNumChildren = 0;
	DisplayObject *pObj = (DisplayObject *) pItem;
	if ( pObj )
		iNumChildren = pObj->NumItems();

	return iNumChildren;
}

void *SWFFormatDataSource::GetChild( const void *pItem, int childIdx )
{
	void *pChild = NULL;
	DisplayObject *pObj = (DisplayObject *) pItem;
	if ( pObj )
		pChild = pObj->GetChild( childIdx );

	return pChild;
}

bool SWFFormatDataSource::HasChildren( const void *pItem )
{
	return( GetNumChildren(pItem) > 0 );
}

TString SWFFormatDataSource::GetName( const void *pItem, bool bReevaluate /* = false */ )
{
	if ( !pItem )
		return TString::Null();

	return ((DisplayObject *) pItem)->GetName();
}

TString SWFFormatDataSource::GetValue( const void *pItem, bool bReevaluate /* = false */ )
{
	if ( !pItem )
		return TString::Null();

	return ((DisplayObject *) pItem)->GetValue();
}

int SWFFormatDataSource::SetValue( const void *pItem, const TString &szValue ) 
{
	if ( !pItem )
		return 0;

	((DisplayObject *) pItem)->SetValue( szValue );
	return 1;
}

short SWFFormatDataSource::HasStringRepValue( const void *pItem )
{
	return( GetValue(pItem).IsEmpty() );
}

// Unused and unsupported
long SWFFormatDataSource::IsDirty( const void *pItem )
{
	return 0;
}

// Unused and unsupported
bool SWFFormatDataSource::ToggleNonLeafDisplayValue( const void *pItem )
{
	return false;
}
