#pragma once

#include <vector>
#include "TString.h"

struct SWFActionRecord;
struct SWFHeader;
struct SWFTagHeader;
struct SWFShapeTag;
struct RGB;
struct RGBA;
struct SWFRect;
struct SWFMatrix;
struct SWFColorXForm;
struct SWFExportAssets;

enum SWFShapeType;
struct SWFGradientRecord;
struct SWFGradient;
struct SWFFillStyle;
struct SWFFillStyleArray;
struct SWFLineStyle;
struct SWFLineStyleArray;
struct SWFShapeRecord;

struct SWFBitsLosslessTag;

struct SWFCodeTable;
struct SWFFont;
struct SWFFont2;
struct SWFFontInfo;
struct SWFFontInfo2;
struct SWFText;
struct SWFEditText;

struct SWFSoundStreamHead;
struct SWFMP3StreamBlock;

struct SWFSpriteTag;
struct SWFPlaceObjectTag;
struct SWFPlaceObject2Tag;
struct SWFRemoveObjectTag;
struct SWFRemoveObject2Tag;

struct SWFFileAnalysis;

class DisplayObject
{
private:
	DisplayObject( const TString& name, const TString& Value, const long bytelen = 0 );
public:
	~DisplayObject();

	long NumItems();
	TString GetName();
	TString GetValue();
	void SetName( const TString &name );
	void SetValue( const TString &value );
	void AppendValue( const TString &appendValue );
	void AddChild( DisplayObject *pChild );
	DisplayObject *GetChild( long idx );

	// General and Primitives
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const RGB &color );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const std::vector<SWFActionRecord>& actions, const long* spriteID );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFFont &font );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFFont2 &font );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFFontInfo &fontInfo, const long numGlyphs );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFFontInfo2 &fontInfo, const long numGlyphs );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFText &text, const SWFCodeTable *pCodeTableList );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFEditText &editText );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFShapeTag &shapeTag );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFSoundStreamHead &streamHead );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFMP3StreamBlock &streamBlock );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFBitsLosslessTag &bitmapTag, const bool hasAlpha );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFSpriteTag &spriteTag );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFPlaceObjectTag &place );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFPlaceObject2Tag &place );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFRemoveObjectTag &remove );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFRemoveObject2Tag &remove );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFTagHeader &tagHeader );
	static DisplayObject *CreateDisplayObject( const TString &name, const SWFFileAnalysis &swfAnalysis );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const SWFHeader &shapeTag );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const int& depth, const int& index );
	static DisplayObject *CreateDisplayObject( const TString &name, const long bytelen, const int numAssets, const SWFExportAssets* exports );

private:
	static void AddDisplayObjects(DisplayObject* pParent, const SWFActionRecord& action);
	static DisplayObject *CreateDisplayObject( const TString &name, const RGBA &color );
	static DisplayObject *CreateDisplayObject( const TString &name, const SWFRect &rect );
	static DisplayObject *CreateDisplayObject( const TString &name, const SWFMatrix &matrix );
	static DisplayObject *CreateDisplayObject( const TString &name, const SWFColorXForm &color );
	static DisplayObject *CreateDisplayObject( const SWFShapeType type, const TString &name, const SWFGradientRecord &gradientRec );
	static DisplayObject *CreateDisplayObject( const SWFShapeType type, const TString &name, const SWFGradient &gradient );
	static DisplayObject *CreateDisplayObject( const SWFShapeType type, const TString &name, const SWFFillStyle &fillStyle );
	static DisplayObject *CreateDisplayObject( const SWFShapeType type, const TString &name, const SWFFillStyleArray &fillStyleArray );
	static DisplayObject *CreateDisplayObject( const SWFShapeType type, const TString &name, const SWFLineStyle &lineStyle );
	static DisplayObject *CreateDisplayObject( const SWFShapeType type, const TString &name, const SWFLineStyleArray &lineStyleArray );
	static DisplayObject *CreateDisplayObject( const SWFShapeType type, const TString &name, SWFShapeRecord *pRecordList );

private:
	long m_lSize;
	long m_lNumItems;
	long m_byteLen;
	TString m_strName;
	TString m_strValue;
	DisplayObject **m_ppChild;

	void GrowChildrenArrayIfNecessary( long lSizeNeeded );
};

typedef struct DISP_OBJ_LIST {
	DisplayObject *pObject;
	struct DISP_OBJ_LIST *pNext;
} DispObjectList;

void AppendDispObjList( DispObjectList *pPrev, DispObjectList *pNext );

