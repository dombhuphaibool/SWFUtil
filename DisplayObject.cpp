#include "stdafx.h"
#include "DisplayObject.h"
#include "SWFFormat.h"

#define GROW_SIZE		8

DisplayObject::DisplayObject( const TString& name, const TString& value, const long bytelen ) :
	m_lSize(0),
	m_lNumItems(0),
	m_byteLen(bytelen),
	m_strName(name),
	m_strValue(value),
	m_ppChild(NULL)
{
	if (bytelen > 0)
	{
		m_strValue += " (";
		m_strValue += TString(bytelen);
		m_strValue += " bytes) ";
	}
}

DisplayObject::~DisplayObject()
{
	if ( m_ppChild )
	{
		for ( int i=0; i<m_lNumItems; i++ )
			delete m_ppChild[i];

		free( m_ppChild );
		m_lNumItems = 0;
	}
}

long DisplayObject::NumItems()
{
	return m_lNumItems;
}

TString DisplayObject::GetName()
{
	return m_strName;
}

TString DisplayObject::GetValue()
{
	return m_strValue;
}

void DisplayObject::SetName( const TString &name )
{
	m_strName = name;
}

void DisplayObject::SetValue( const TString &value )
{
	m_strValue = value;
}

void DisplayObject::AppendValue( const TString &appendValue )
{
	m_strValue += appendValue;
}

void DisplayObject::AddChild( DisplayObject *pChild )
{
	GrowChildrenArrayIfNecessary( m_lNumItems+1 );
	m_ppChild[m_lNumItems] = pChild;
	m_lNumItems++;
}

DisplayObject *DisplayObject::GetChild( long idx )
{
	DisplayObject *pChild = NULL;
	if ( idx < m_lNumItems )
		pChild = m_ppChild[idx];

	return pChild;
}

void DisplayObject::GrowChildrenArrayIfNecessary( long lSizeNeeded )
{
	if ( m_lSize == 0 )
	{
		long lNewSize = lSizeNeeded + GROW_SIZE;
		m_ppChild = (DisplayObject **) malloc( sizeof(DisplayObject*) * lNewSize );
		m_lSize = lNewSize;
	}
	else if ( lSizeNeeded > m_lSize )
	{
		long lNewSize = lSizeNeeded + GROW_SIZE;
		m_ppChild = (DisplayObject **) realloc( m_ppChild, sizeof(DisplayObject*) * lNewSize );
		m_lSize = lNewSize;
	}
}

/******************************************************************************
 *
 * In-Memory variable data structure conversion routines 
 * - Converts into DisplayObject (hierarchical display objects)
 *
 *****************************************************************************/
DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFTagHeader &tagHeader )
{
	DisplayObject *pDO = new DisplayObject( name, TString(EMPTY_SPACE), bytelen );
	pDO->AddChild( new DisplayObject(TString("Type"), TString(tagHeader.type)) );
	pDO->AddChild( new DisplayObject(TString("Length"), TString(tagHeader.length)) );

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen )
{
	DisplayObject *pDO = new DisplayObject( name, TString(EMPTY_SPACE), bytelen );
	if (bytelen > 0)
		pDO->AddChild( new DisplayObject(TString("Tag Length"), TString(bytelen)) );

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const int& depth, const int& index )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	pDO->AddChild( new DisplayObject(TString(SWFNAME_DEPTH), depth) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_INDEX), index) );
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const RGB &color )
{
	DisplayObject *pDO = new DisplayObject( name, TString(EMPTY_SPACE), bytelen );
	char buf[256];
	sprintf(buf, " rgb=0x%02x%02x%02x", color.red, color.green, color.blue);
	pDO->AppendValue(TString(buf));
	
	pDO->AddChild( new DisplayObject(TString(SWFNAME_RED), color.red) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_GREEN), color.green) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_BLUE), color.blue) );
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const RGBA &color )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE));
	char buf[256];
	sprintf(buf, " rgb=0x%02x%02x%02x alpha=%d", color.red, color.green, color.blue, color.alpha);
	pDO->AppendValue(TString(buf));

	pDO->AddChild( new DisplayObject(TString(SWFNAME_RED), color.red) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_GREEN), color.green) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_BLUE), color.blue) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_ALPHA), color.alpha) );
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const SWFRect &rect )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE));
	pDO->AddChild( new DisplayObject(TString(SWFNAME_LEFT), TString(rect.left)) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_RIGHT), TString(rect.right)) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_TOP), TString(rect.top)) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_BOTTOM), TString(rect.bottom)) );
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const SWFMatrix &matrix )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE));

	if ( matrix.hasScale )
	{
		pDO->AddChild( new DisplayObject(TString(SWFNAME_SCALEX), TString(matrix.xScale, TString::VALUE_IS_FIXED16)) );
		pDO->AddChild( new DisplayObject(TString(SWFNAME_SCALEY), TString(matrix.yScale, TString::VALUE_IS_FIXED16)) );
	}

	if ( matrix.hasRotate )
	{
		pDO->AddChild( new DisplayObject(TString(SWFNAME_ROTATE0), TString(matrix.rotateSkew0, TString::VALUE_IS_FIXED16)) );
		pDO->AddChild( new DisplayObject(TString(SWFNAME_ROTATE1), TString(matrix.rotateSkew1, TString::VALUE_IS_FIXED16)) );
	}

	pDO->AddChild( new DisplayObject(TString(SWFNAME_TRANSLATEX), TString(matrix.xTranslate)) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_TRANSLATEY), TString(matrix.yTranslate)) );

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const SWFColorXForm &color )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE));
	
	if ( color.bHasAdd )
	{
		pDO->AddChild( new DisplayObject(TString(SWFNAME_ADDRED), TString(color.addRed, TString::VALUE_IS_FIXED8)) );
		pDO->AddChild( new DisplayObject(TString(SWFNAME_ADDGREEN), TString(color.addGreen, TString::VALUE_IS_FIXED8)) );
		pDO->AddChild( new DisplayObject(TString(SWFNAME_ADDBLUE), TString(color.addBlue, TString::VALUE_IS_FIXED8)) );
		if ( color.bHasAlpha )
			pDO->AddChild( new DisplayObject(TString(SWFNAME_ADDALPHA), TString(color.addAlpha, TString::VALUE_IS_FIXED8)) );
	}

	if ( color.bHasMult )
	{
		pDO->AddChild( new DisplayObject(TString(SWFNAME_MULTRED), TString(color.multRed, TString::VALUE_IS_FIXED8)) );
		pDO->AddChild( new DisplayObject(TString(SWFNAME_MULTGREEN), TString(color.multGreen, TString::VALUE_IS_FIXED8)) );
		pDO->AddChild( new DisplayObject(TString(SWFNAME_MULTBLUE), TString(color.multBlue, TString::VALUE_IS_FIXED8)) );
		if ( color.bHasAlpha )
			pDO->AddChild( new DisplayObject(TString(SWFNAME_MULTALPHA), TString(color.multAlpha, TString::VALUE_IS_FIXED8)) );
	}

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const int numAssets, const SWFExportAssets* exports )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	for (int i = 0; i < numAssets; ++i)
	{
		pDO->AddChild( new DisplayObject(TString(SWFNAME_CHARACTERID), TString(exports[i].characterID)) );
		pDO->AddChild( new DisplayObject(TString(SWFNAME_NAME), TString(exports[i].name)) );
	}
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const SWFShapeType type, const TString &name, const SWFGradientRecord &gradientRec )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE));
	pDO->AddChild( new DisplayObject(TString(SWFNAME_RATIO), TString(gradientRec.ratio)) );
	if ( type == SWFShape3 )
		pDO->AddChild( CreateDisplayObject(TString(SWFNAME_COLOR), gradientRec.u.shape3Color) );
	else
		pDO->AddChild( CreateDisplayObject(TString(SWFNAME_COLOR), 0, gradientRec.u.color) );
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const SWFShapeType type, const TString &name, const SWFGradient &gradient )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE));
	pDO->AddChild( new DisplayObject(TString(SWFNAME_NUMCTRLPOINTS), TString(gradient.numControlPoints)) );
	char szTmp[8];
	SWFGradientRecord *pCurrRec = gradient.pRecord;
	SWFGradientRecord *pDelRec = NULL;
	int i = 1;
	while ( pCurrRec )
	{
		assert( pCurrRec != NULL );
		sprintf( szTmp, "[%i]", i++ );
		pDO->AddChild( CreateDisplayObject(type, TString(szTmp), *pCurrRec) );
		pDelRec = pCurrRec;
		pCurrRec = pCurrRec->next;
		delete pDelRec;
	}
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const SWFShapeType type, const TString &name, const SWFFillStyle &fillStyle )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE));
	switch( fillStyle.type )
	{
		case SWFSolidFill:
		{
			pDO->AddChild( new DisplayObject(TString(SWFNAME_FILLSTYLETYPE), 
											 TString(SWFNAME_SOLIDFILL)) );
			if ( type == SWFShape3 )
				pDO->AddChild( CreateDisplayObject(TString(SWFNAME_COLOR), fillStyle.shape3Color) );
			else
				pDO->AddChild( CreateDisplayObject(TString(SWFNAME_COLOR), 0, fillStyle.color) );									 
		}
		break;

		case SWFLinearGradientFill:	
		{
			pDO->AddChild( new DisplayObject(TString(SWFNAME_FILLSTYLETYPE), 
											 TString(SWFNAME_LINEARGRAD)) );		
			pDO->AddChild( CreateDisplayObject(TString(SWFNAME_GRADIENTMATRIX), fillStyle.gradientMatrix) );
			pDO->AddChild( CreateDisplayObject(type, TString(SWFNAME_GRADIENTFILL), fillStyle.gradientFill) );
		}
		break;
		
		case SWFRadialGradientFill: 
		{
			pDO->AddChild( new DisplayObject(TString(SWFNAME_FILLSTYLETYPE), 
											 TString(SWFNAME_RADIALGRAD)) );			
			pDO->AddChild( CreateDisplayObject(TString(SWFNAME_GRADIENTMATRIX), fillStyle.gradientMatrix) );
			pDO->AddChild( CreateDisplayObject(type, TString(SWFNAME_GRADIENTFILL), fillStyle.gradientFill) );
		}
		break;
		
		case SWFTiledBitmapFill:	
		{
			pDO->AddChild( new DisplayObject(TString(SWFNAME_FILLSTYLETYPE), 
											 TString(SWFNAME_TILEDBITMAP)) );			
			pDO->AddChild( new DisplayObject(TString(SWFNAME_BITMAPID), TString(fillStyle.bitmapID)) );
			pDO->AddChild( CreateDisplayObject(TString(SWFNAME_BITMAPMATRIX), fillStyle.bitmapMatrix) );
		}
		break;
		
		case SWFClippedBitmapFill:	
		{
			pDO->AddChild( new DisplayObject(TString(SWFNAME_FILLSTYLETYPE), 
											 TString(SWFNAME_CLIPPEDBITMAP)) );			
			pDO->AddChild( new DisplayObject(TString(SWFNAME_BITMAPID), TString(fillStyle.bitmapID)) );
			pDO->AddChild( CreateDisplayObject(TString(SWFNAME_BITMAPMATRIX), fillStyle.bitmapMatrix) );
		}
		break;
	}
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const SWFShapeType type, const TString &name, const SWFFillStyleArray &fillStyleArray )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE));
	pDO->AddChild( new DisplayObject(TString(SWFNAME_COUNT), TString(fillStyleArray.count)) );
	char szTmp[8];
	SWFFillStyle *pCurrStyle = fillStyleArray.pStyle;
	SWFFillStyle *pDelStyle = NULL;
	for ( int i=0; i<fillStyleArray.count; i++ )
	{
		assert( pCurrStyle != NULL );
		sprintf( szTmp, "[%i]", i );
		pDO->AddChild( CreateDisplayObject(type, TString(szTmp), *pCurrStyle) );
		pDelStyle = pCurrStyle;
		pCurrStyle = pCurrStyle->next;
		delete pDelStyle;
	}
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const SWFShapeType type, const TString &name, const SWFLineStyle &lineStyle )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE));
	pDO->AddChild( new DisplayObject(TString(SWFNAME_WIDTH), TString(lineStyle.width)) );
	if ( type == SWFShape3 )
		pDO->AddChild( CreateDisplayObject(TString(SWFNAME_COLOR), lineStyle.u.shape3Color) );
	else
		pDO->AddChild( CreateDisplayObject(TString(SWFNAME_COLOR), 0, lineStyle.u.color) );

	return pDO;
};

DisplayObject *DisplayObject::CreateDisplayObject( const SWFShapeType type, const TString &name, const SWFLineStyleArray &lineStyleArray )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE));
	pDO->AddChild( new DisplayObject(TString(SWFNAME_COUNT), TString(lineStyleArray.count)) );
	char szTmp[8];
	SWFLineStyle *pCurrStyle = lineStyleArray.pStyle;
	SWFLineStyle *pDelStyle = NULL;
	for ( int i=0; i<lineStyleArray.count; i++ )
	{
		assert( pCurrStyle != NULL );
		sprintf( szTmp, "[%i]", i );
		pDO->AddChild( CreateDisplayObject(type, TString(szTmp), *pCurrStyle) );
		pDelStyle = pCurrStyle;
		pCurrStyle = pCurrStyle->next;
		delete pDelStyle;
	}
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const SWFShapeType type, const TString &name, SWFShapeRecord *pRecordList )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE));

	SWFShapeRecord *pCurrRecord = pRecordList;
	SWFShapeRecord *pDelRecord = NULL;
	while ( pCurrRecord )
	{
		switch( pCurrRecord->type )
		{
			case SWFsrtEndShape:
			{
				pDO->AddChild( new DisplayObject(TString(SWFNAME_ENDSHAPEREC), TString(EMPTY_SPACE)) );
			}
			break;

			case SWFsrtStyleChange:
			{
				SWFStyleChangeRecord *pStyleChangeRec = (SWFStyleChangeRecord *) pCurrRecord->pRecord;
				DisplayObject *pStyleChange = new DisplayObject( TString(SWFNAME_STYLECHANGEREC), TString(EMPTY_SPACE) );
				if ( pStyleChangeRec->bMoveTo )
				{
					pStyleChange->AddChild( new DisplayObject(TString(SWFNAME_MOVEDELTAX), TString(pStyleChangeRec->moveDeltaX)) );
					pStyleChange->AddChild( new DisplayObject(TString(SWFNAME_MOVEDELTAY), TString(pStyleChangeRec->moveDeltaY)) );
				}

				if ( pStyleChangeRec->bFillStyle0Changed )
					pStyleChange->AddChild( new DisplayObject(TString(SWFNAME_FILLCHANGE0), TString(pStyleChangeRec->fillStyle0)) );
				if ( pStyleChangeRec->bFillStyle1Changed )
					pStyleChange->AddChild( new DisplayObject(TString(SWFNAME_FILLCHANGE1), TString(pStyleChangeRec->fillStyle1)) );
				if ( pStyleChangeRec->bLineStyleChanged )
					pStyleChange->AddChild( new DisplayObject(TString(SWFNAME_LINECHANGE), TString(pStyleChangeRec->lineStyle)) );
				
				if ( pStyleChangeRec->bNewStyles )
				{
					pStyleChange->AddChild( DisplayObject::CreateDisplayObject(type, TString(SWFNAME_FILLSTYLEARRAY), pStyleChangeRec->fillStyleArray) );
					pStyleChange->AddChild( DisplayObject::CreateDisplayObject(type, TString(SWFNAME_LINESTYLEARRAY), pStyleChangeRec->lineStyleArray) );
					pStyleChange->AddChild( new DisplayObject(TString(SWFNAME_NUMFILLBITS), TString(pStyleChangeRec->numFillBits)) );
					pStyleChange->AddChild( new DisplayObject(TString(SWFNAME_NUMLINEBITS), TString(pStyleChangeRec->numLineBits)) );
				}

				pDO->AddChild( pStyleChange );
			}
			break;

			case SWFsrtStraightEdge:
			{
				SWFStraightEdgeRecord *pEdgeRecord = (SWFStraightEdgeRecord *) pCurrRecord->pRecord;
				switch( pEdgeRecord->lineType )
				{
					case SWFGeneralLine:
					{
						DisplayObject *pLineObject = new DisplayObject( TString(SWFNAME_GENERALSTRAIGHTEDGE), TString(EMPTY_SPACE) );
						pLineObject->AddChild( new DisplayObject(TString(SWFNAME_DELTAX), TString(pEdgeRecord->deltaX)) );
						pLineObject->AddChild( new DisplayObject(TString(SWFNAME_DELTAY), TString(pEdgeRecord->deltaY)) );
						pDO->AddChild( pLineObject );
					}
					break;

					case SWFVerticalLine:
					{
						DisplayObject *pLineObject = new DisplayObject( TString(SWFNAME_VERTSTRAIGHTEDGE), TString(EMPTY_SPACE) );
						pLineObject->AddChild( new DisplayObject(TString(SWFNAME_DELTAY), TString(pEdgeRecord->vertDeltaY)) );
						pDO->AddChild( pLineObject );
					}
					break;

					case SWFHorizontalLine:
					{
						DisplayObject *pLineObject = new DisplayObject( TString(SWFNAME_HORZSTRAIGHTEDGE), TString(EMPTY_SPACE) );
						pLineObject->AddChild( new DisplayObject(TString(SWFNAME_DELTAX), TString(pEdgeRecord->horzDeltaX)) );
						pDO->AddChild( pLineObject );
					}
					break;
				}
			}
			break;

			case SWFsrtCurvedEdge:
			{
				SWFCurvedEdgeRecord *pEdgeRecord = (SWFCurvedEdgeRecord *) pCurrRecord->pRecord;
				DisplayObject *pLineObject = new DisplayObject( TString(SWFNAME_CURVEDEDGEREC), TString(EMPTY_SPACE) );
				pLineObject->AddChild( new DisplayObject(TString(SWFNAME_CONTROLDELTAX), TString(pEdgeRecord->controlDeltaX)) );
				pLineObject->AddChild( new DisplayObject(TString(SWFNAME_CONTROLDELTAY), TString(pEdgeRecord->controlDeltaY)) );
				pLineObject->AddChild( new DisplayObject(TString(SWFNAME_ANCHORDELTAX), TString(pEdgeRecord->anchorDeltaX)) );
				pLineObject->AddChild( new DisplayObject(TString(SWFNAME_ANCHORDELTAY), TString(pEdgeRecord->anchorDeltaY)) );
				pDO->AddChild( pLineObject );
			}
			break;
		}
		pDelRecord = pCurrRecord;
		pCurrRecord = pCurrRecord->next;
		if ( pDelRecord->pRecord )
			delete pDelRecord->pRecord;
		delete pDelRecord;
	}

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFShapeTag &shapeTag )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	pDO->AddChild( new DisplayObject(TString(SWFNAME_SHAPEID), TString(shapeTag.id)) );
	pDO->AddChild( CreateDisplayObject(TString(SWFNAME_SHAPEBOUNDS), shapeTag.bounds) );

	// Create ShapeWithStyle
	DisplayObject *pSWS = new DisplayObject( TString(SWFNAME_SHAPEWITHSTYLE), TString(EMPTY_SPACE) );
		pSWS->AddChild( CreateDisplayObject(shapeTag.type, TString(SWFNAME_FILLSTYLEARRAY), shapeTag.shapeWithStyle.fillStyleArray) );
		pSWS->AddChild( CreateDisplayObject(shapeTag.type, TString(SWFNAME_LINESTYLEARRAY), shapeTag.shapeWithStyle.lineStyleArray) );
		pSWS->AddChild( new DisplayObject(TString(SWFNAME_NUMFILLBITS), 
										  TString(shapeTag.shapeWithStyle.numFillBits)) );
		pSWS->AddChild( new DisplayObject(TString(SWFNAME_NUMLINEBITS), 
										  TString(shapeTag.shapeWithStyle.numLineBits)) );
		pSWS->AddChild( CreateDisplayObject(shapeTag.type, TString(SWFNAME_SHAPERECORDS), shapeTag.shapeWithStyle.pRecordList) );
	pDO->AddChild( pSWS );
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFSoundStreamHead &streamHead )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	TString playbackRate;
	switch (streamHead.playbackRate)
	{
		case SWFSndRate_5_5kHz:		playbackRate = SWFNAME_5_5KHZ;		break;
		case SWFSndRate_11kHz:		playbackRate = SWFNAME_11KHZ;		break;
		case SWFSndRate_22kHz:		playbackRate = SWFNAME_22KHZ;		break;
		case SWFSndRate_44kHz:		playbackRate = SWFNAME_44KHZ;		break;
		default:					playbackRate = SWFNAME_UNRECOGNIZED;break;
	}
	pDO->AddChild( new DisplayObject(TString(SWFNAME_PLAYBACKRATE), playbackRate) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_PLAYBACKSIZE), streamHead.playbackSize ? TString(SWFNAME_16BIT) : TString(SWFNAME_8BIT)) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_PLAYBACKTYPE), streamHead.playbackType ? TString(SWFNAME_STEREO) : TString(SWFNAME_MONO)) );

	TString compressionType;
	switch (streamHead.compressionType)
	{
		case SWFSndCompress_none:			compressionType = SWFNAME_UNCOMPRESSED;		break;
		case SWFSndCompress_ADPCM:			compressionType = SWFNAME_ADPCM;			break;
		case SWFSndCompress_MP3:			compressionType = SWFNAME_MP3;				break;
		case SWFSndCompress_littleEndian:	compressionType = SWFNAME_UNCOMPRESSED_LITTLEENDIAN;	break;
		case SWFSndCompress_Nellymoser:		compressionType = SWFNAME_NELLYMOSER;		break;
		default:							compressionType = SWFNAME_UNRECOGNIZED;		break;
	}
	pDO->AddChild( new DisplayObject(TString(SWFNAME_COMPRESSIONTYPE), compressionType) );

	TString streamRate;
	switch (streamHead.streamRate)
	{
		case SWFSndRate_5_5kHz:		streamRate = SWFNAME_5_5KHZ;		break;
		case SWFSndRate_11kHz:		streamRate = SWFNAME_11KHZ;			break;
		case SWFSndRate_22kHz:		streamRate = SWFNAME_22KHZ;			break;
		case SWFSndRate_44kHz:		streamRate = SWFNAME_44KHZ;			break;
		default:					streamRate = SWFNAME_UNRECOGNIZED;	break;
	}
	pDO->AddChild( new DisplayObject(TString(SWFNAME_STREAMRATE), streamRate) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_STREAMSIZE), streamHead.streamSize ? TString(SWFNAME_16BIT) : TString(SWFNAME_8BIT)) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_STREAMTYPE), streamHead.streamType ? TString(SWFNAME_STEREO) : TString(SWFNAME_MONO)) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_SAMPLECOUNT), TString(streamHead.streamSampleCount)) );
	if (streamHead.compressionType == SWFSndCompress_MP3)
	{
		pDO->AddChild( new DisplayObject(TString(SWFNAME_LATENCYSEEK), TString(streamHead.latencySeek)) );
	}

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFMP3StreamBlock &streamBlock )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	pDO->AddChild( new DisplayObject(TString(SWFNAME_SAMPLECOUNT), TString(streamBlock.sampleCount)) );
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFBitsLosslessTag &bitmapTag, const bool hasAlpha )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	pDO->AddChild( new DisplayObject(TString(SWFNAME_CHARACTERID), TString(bitmapTag.id)) );
	TString strFormat;
	switch ( bitmapTag.format )
	{
		case 3:
			strFormat = "8-bit colormapped RGB";
			break;

		case 4:
			strFormat = "16-bit RGB";
			break;

		case 5:
			strFormat = "32-bit RGB";
			break;

		default:
			strFormat = "unrecognized RGB";
	}
	if (hasAlpha)
		strFormat += "A";
	
	pDO->AddChild( new DisplayObject(TString(SWFNAME_BITMAPFORMAT), strFormat) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_WIDTH), TString(bitmapTag.width)) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_HEIGHT), TString(bitmapTag.height)) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_COLORTABLESIZE), TString(bitmapTag.tableSize)) );
	TString strDataSize(bitmapTag.dataSize);
	strDataSize += " bytes";
	pDO->AddChild( new DisplayObject(TString(SWFNAME_DATASIZE), strDataSize) );

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFFont &font )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	pDO->AddChild( new DisplayObject(TString(SWFNAME_FONTID), TString(font.fontID)) );

	DisplayObject *pOT = new DisplayObject( TString(SWFNAME_OFFSETTABLE), TString(EMPTY_SPACE) );
	DisplayObject *pST = new DisplayObject( TString(SWFNAME_SHAPETABLE), TString(EMPTY_SPACE) );
	DisplayObject *pNewShapeDO = NULL;
	char szTmp[8];
	for ( int i=0; i<font.numGlyphs; i++ )
	{
		sprintf( szTmp, "[%i]", i );
		pOT->AddChild( new DisplayObject(TString(szTmp), TString(font.pOffsetTable[i])) );
		pNewShapeDO = new DisplayObject( TString(szTmp), TString(EMPTY_SPACE) );
			pNewShapeDO->AddChild( new DisplayObject(TString(SWFNAME_NUMFILLBITS), TString(font.pShapeTable[i].numFillBits)) );
			pNewShapeDO->AddChild( new DisplayObject(TString(SWFNAME_NUMLINEBITS), TString(font.pShapeTable[i].numLineBits)) );
			pNewShapeDO->AddChild( DisplayObject::CreateDisplayObject(SWFShape1, TString(SWFNAME_SHAPERECORDS), font.pShapeTable[i].pRecordList) );
		pST->AddChild( pNewShapeDO );
	}
	pDO->AddChild( pOT );
	pDO->AddChild( pST );

	if ( font.pOffsetTable )
		delete[] font.pOffsetTable;
	if ( font.pShapeTable )
		delete[] font.pShapeTable;

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFFont2 &font )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);

	pDO->AddChild( new DisplayObject(TString(SWFNAME_FONTID), TString(font.fontID)) );

	/* Skip the flags for now... It's not that interesting...
	// flags
	DisplayObject *pFDO = new DisplayObject(TString(SWFNAME_FLAGS), TString(EMPTY_SPACE) );
	pFDO->AddChild( new DisplayObject(TString(SWFNAME_HASLAYOUT), TString(font.bHasLayout)) );
	pFDO->AddChild( new DisplayObject(TString(SWFNAME_SHIFTJIS), TString(font.bShiftJIS)) );
	pFDO->AddChild( new DisplayObject(TString(SWFNAME_ANSI), TString(font.bANSI)) );
	pFDO->AddChild( new DisplayObject(TString(SWFNAME_WIDEOFFSETS), TString(font.bWideOffsets)) );
	pFDO->AddChild( new DisplayObject(TString(SWFNAME_WIDECODES), TString(font.bWideCodes)) );
	pFDO->AddChild( new DisplayObject(TString(SWFNAME_ITALIC), TString(font.bItalic)) );
	pFDO->AddChild( new DisplayObject(TString(SWFNAME_BOLD), TString(font.bBold)) );
	pDO->AddChild( pFDO );
	*/

	pDO->AddChild( new DisplayObject(TString(SWFNAME_FONTNAME), TString((char *) font.szFontName)) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_NUMGLYPHS), TString(font.numGlyphs)) );

	// Skip the font offset table... it's not interesting...
	// DisplayObject *pFOT = new DisplayObject( TString(SWFNAME_FONTOFFSETTBL), TString(EMPTY_SPACE) );

	DisplayObject *pST = new DisplayObject( TString(SWFNAME_SHAPETABLE), TString(EMPTY_SPACE) );
	DisplayObject *pCT = new DisplayObject( TString(SWFNAME_CODETABLE), TString(EMPTY_SPACE) );

	DisplayObject *pNewShapeDO = NULL;
	char szTmp[8];
	char szCharCode[8];
	for ( int i=0; i<font.numGlyphs; i++ )
	{
		sprintf( szTmp, "[%i]", i );
		pNewShapeDO = new DisplayObject( TString(szTmp), TString(EMPTY_SPACE) );
			pNewShapeDO->AddChild( new DisplayObject(TString(SWFNAME_NUMFILLBITS), TString(font.pShapeTable[i].numFillBits)) );
			pNewShapeDO->AddChild( new DisplayObject(TString(SWFNAME_NUMLINEBITS), TString(font.pShapeTable[i].numLineBits)) );
			pNewShapeDO->AddChild( DisplayObject::CreateDisplayObject(SWFShape1, TString(SWFNAME_SHAPERECORDS), font.pShapeTable[i].pRecordList) );
		pST->AddChild( pNewShapeDO );
		sprintf( szCharCode, "%c", font.pCodeTable[i] );
		pCT->AddChild( new DisplayObject(TString(szTmp), TString(szCharCode)) );
	}
	pDO->AddChild( pST );
	pDO->AddChild( pCT );

	if ( font.bHasLayout )
	{
		pDO->AddChild( new DisplayObject(TString(SWFNAME_ASCENT), TString(font.ascent)) );
		pDO->AddChild( new DisplayObject(TString(SWFNAME_DESCENT), TString(font.descent)) );
		pDO->AddChild( new DisplayObject(TString(SWFNAME_LEADING), TString(font.leading)) );

		// Skip the advance and bounds table for now... They're not interesting...

		// Skip the kerning table too...
	}

	if ( font.szFontName )
		delete[] font.szFontName;
	if ( font.pFontOffsetTable )
		delete[] font.pFontOffsetTable;
	if ( font.pShapeTable )
		delete[] font.pShapeTable;
	if ( font.pCodeTable )
		delete[] font.pCodeTable;
	if ( font.pAdvanceTable )
		delete[] font.pAdvanceTable;
	if ( font.pBoundsTable )
		delete[] font.pBoundsTable;
	if ( font.pKerningTable )
		delete[] font.pKerningTable;

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFFontInfo &fontInfo, const long numGlyphs )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);

	pDO->AddChild( new DisplayObject(TString(SWFNAME_FONTID), TString(fontInfo.header.fontID)) );
	/* Skip the flags for now... It's not that interesting... */
	pDO->AddChild( new DisplayObject(TString(SWFNAME_FONTNAME), TString((char *) fontInfo.header.szFontName)) );

	DisplayObject *pCT = new DisplayObject( TString(SWFNAME_CODETABLE), TString(EMPTY_SPACE) );
	char szTmp[8];
	char szCharCode[8];
	for ( int i=0; i<numGlyphs; i++ )
	{
		sprintf( szTmp, "[%i]", i );
		sprintf( szCharCode, "%c", fontInfo.pCodeTable[i] );
		pCT->AddChild( new DisplayObject(TString(szTmp), TString(szCharCode)) );
	}
	pDO->AddChild( pCT );

	if ( fontInfo.header.szFontName )
		delete[] fontInfo.header.szFontName;
	if ( fontInfo.pCodeTable )
		delete fontInfo.pCodeTable;

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFFontInfo2 &fontInfo, const long numGlyphs )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);

	pDO->AddChild( new DisplayObject(TString(SWFNAME_FONTID), TString(fontInfo.header.fontID)) );
	/* Skip the flags for now... It's not that interesting... */
	pDO->AddChild( new DisplayObject(TString(SWFNAME_FONTNAME), TString((char *) fontInfo.header.szFontName)) );

	DisplayObject *pCT = new DisplayObject( TString(SWFNAME_CODETABLE), TString(EMPTY_SPACE) );
	char szTmp[8];
	char szCharCode[8];
	for ( int i=0; i<numGlyphs; i++ )
	{
		sprintf( szTmp, "[%i]", i );
		sprintf( szCharCode, "%c", fontInfo.pCodeTable[i] );
		pCT->AddChild( new DisplayObject(TString(szTmp), TString(szCharCode)) );
	}
	pDO->AddChild( pCT );

	if ( fontInfo.header.szFontName )
		delete[] fontInfo.header.szFontName;
	if ( fontInfo.pCodeTable )
		delete fontInfo.pCodeTable;

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFText &text, const SWFCodeTable *pCodeTableList )
{
	char *szTextDisplay = NULL;
	U16 currFontID = 0;
	const SWFCodeTable *pCurrCodeTable = NULL;

	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	pDO->AddChild( new DisplayObject(TString(SWFNAME_CHARACTERID), TString(text.characterID)) );
	pDO->AddChild( DisplayObject::CreateDisplayObject(TString(SWFNAME_BOUNDS), text.bound) );
	pDO->AddChild( DisplayObject::CreateDisplayObject(TString(SWFNAME_MATRIX), text.matrix) );

	DisplayObject* pDO_misc = DisplayObject::CreateDisplayObject(TString("Misc"), 0);
	char buffer[256];
	sprintf(buffer, "NumGlyphBits=%d, AdvanceGlyphBits=%d", (int)text.numGlyphBits, (int)text.advanceGlyphBits);
	pDO_misc->AppendValue( buffer );
	pDO->AddChild(pDO_misc);

	TString textStr = "";
	DisplayObject *pTR = new DisplayObject( TString(SWFNAME_TEXTRECORDS), TString(EMPTY_SPACE) );
	SWFTextRecordList *pCurrRecList = text.pRecordList;
	SWFTextRecordList *pDelRecList = NULL;
	while ( pCurrRecList )
	{
		switch( pCurrRecList->type )
		{
			case SWFGlyph:
			{
				SWFGlyphRecord *pGRec = (SWFGlyphRecord *) pCurrRecList->pRecord;
				assert( pGRec );
				if ( pGRec )
				{
					DisplayObject *pG = new DisplayObject( TString(SWFNAME_GLYPH), TString(EMPTY_SPACE) );
					pG->AddChild( new DisplayObject(TString(SWFNAME_COUNT), TString(pGRec->count)) );

					// Find the code table associated with the curr font ID if we can
					pCurrCodeTable = FindFontCodeTable( currFontID, pCodeTableList );
					if ( pCurrCodeTable )
						szTextDisplay = new char[pGRec->count+1];

					// For now, only show the count without showing all the arrays...
					DisplayObject *pGEntries = new DisplayObject( TString(SWFNAME_GLYPHENTRIES), TString(EMPTY_SPACE) );
					SWFGlyphEntry *pCurrGlyphEntry = pGRec->pEntries;
					SWFGlyphEntry *pDelGlyphEntry = NULL;
					char szIndex[16];
					// char szAdvance[16];
					for ( int i=1; i<=pGRec->count; i++ )
					{
						assert( pCurrGlyphEntry );

						// Let's not show Advance, it's not very interesting for now...
						sprintf( szIndex, "[%i] Index", i );
						// sprintf( szAdvance, "[%i] Advance", i );
						pGEntries->AddChild( new DisplayObject(TString(szIndex), TString(pCurrGlyphEntry->index)) );
						// pGEntries->AddChild( new DisplayObject(TString(szAdvance), TString(pCurrGlyphEntry->advance)) );

						// Add to the text display string
						if ( szTextDisplay )
							szTextDisplay[i-1] = (char) pCurrCodeTable->pTable[pCurrGlyphEntry->index];

						pDelGlyphEntry = pCurrGlyphEntry;
						pCurrGlyphEntry = pCurrGlyphEntry->next;
						delete pDelGlyphEntry;
					}
					pG->AddChild( pGEntries );

					pTR->AddChild( pG );

					// clean-up the text display string and reset the current code table
					if ( szTextDisplay )
					{
						szTextDisplay[pGRec->count] = 0;
						// pG->SetValue( strTextDisplay ); - Commented out because we now display the text at the root tag (see next line)
						textStr += szTextDisplay;
						delete[] szTextDisplay;
						szTextDisplay = NULL;
					}
					pCurrCodeTable = NULL;
				}
			}
			break;

			case SWFTextStyleChange:
			{
				SWFTextStyleChangeRecord *pTSCRec = (SWFTextStyleChangeRecord *) pCurrRecList->pRecord;
				assert( pTSCRec );
				if ( pTSCRec )
				{
					DisplayObject *pTSC = new DisplayObject( TString(SWFNAME_TEXTSTYLECHANGE), TString(EMPTY_SPACE) );
					if ( pTSCRec->bHasFont )
					{
						pTSC->AddChild( new DisplayObject(TString(SWFNAME_FONTID), TString(pTSCRec->fontID)) );
						currFontID = pTSCRec->fontID;
					}
					if ( pTSCRec->bHasColor )
						pTSC->AddChild( DisplayObject::CreateDisplayObject(TString(SWFNAME_COLOR), 0, pTSCRec->u.textColor) );
					if ( pTSCRec->bHasXOffset )
						pTSC->AddChild( new DisplayObject(TString(SWFNAME_XOFFSET), TString(pTSCRec->xOffset)) );
					if ( pTSCRec->bHasYOffset )
						pTSC->AddChild( new DisplayObject(TString(SWFNAME_YOFFSET), TString(pTSCRec->yOffset)) );
					if ( pTSCRec->bHasFont )
						pTSC->AddChild( new DisplayObject(TString(SWFNAME_TEXTHEIGHT), TString(pTSCRec->textHeight)) );	
					pTR->AddChild( pTSC );
				}
			}
			break;
		}

		pDelRecList = pCurrRecList;
		pCurrRecList = pCurrRecList->next;
		delete pDelRecList;
	}

	pDO->AppendValue(TString(" text=\""));
	pDO->AppendValue(textStr);
	pDO->AppendValue(TString("\""));

	pDO->AddChild( pTR );

	return pDO;
}

static void AppendFlags( TString &strFlag, const char *szNewFlag )
{
	if ( !strFlag.IsEmpty() )
		strFlag += ", ";
	strFlag += szNewFlag;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFEditText &editText )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);

	pDO->AddChild( new DisplayObject(TString(SWFNAME_CHARACTERID), TString(editText.characterID)) );
	pDO->AddChild( DisplayObject::CreateDisplayObject(TString(SWFNAME_BOUNDS), editText.bound) );

	if ( editText.hasFont )
	{
		pDO->AddChild( new DisplayObject(TString(SWFNAME_FONTID), TString(editText.fontID)) );
		pDO->AddChild( new DisplayObject(TString(SWFNAME_FONTHEIGHT), TString(editText.fontHeight)) );
	}

	if ( editText.hasTextColor )
		pDO->AddChild( DisplayObject::CreateDisplayObject(TString(SWFNAME_TEXTCOLOR), editText.textColor) );
	if ( editText.hasMaxLength )
		pDO->AddChild( new DisplayObject(TString(SWFNAME_MAXLENGTH), TString(editText.maxLength)) );

	if ( editText.hasLayout )
	{
		DisplayObject *pLO = new DisplayObject( TString(SWFNAME_LAYOUT), TString(EMPTY_SPACE) );
		pLO->AddChild( new DisplayObject(TString(SWFNAME_LEFTMARGIN), TString(editText.leftMargin)) );
		pLO->AddChild( new DisplayObject(TString(SWFNAME_RIGHTMARGIN), TString(editText.rightMargin)) );
		pLO->AddChild( new DisplayObject(TString(SWFNAME_INDENT), TString(editText.indent)) );
		pLO->AddChild( new DisplayObject(TString(SWFNAME_LEADING), TString(editText.leading)) );
	}

	TString flags;
	if ( editText.wordWrap )
		AppendFlags( flags, SWFNAME_WORDWRAP );
	if ( editText.multiLine )
		AppendFlags( flags, SWFNAME_MULTILINE );
	if ( editText.password )
		AppendFlags( flags, SWFNAME_PASSWORD );
	if ( editText.readOnly )
		AppendFlags( flags, SWFNAME_READONLY );
	if ( editText.autoSize )
		AppendFlags( flags, SWFNAME_AUTOSIZE );
	if ( editText.noSelect )
		AppendFlags( flags, SWFNAME_NOSELECT );
	if ( editText.border )
		AppendFlags( flags, SWFNAME_BORDER );
	if ( editText.html )
		AppendFlags( flags, SWFNAME_HTML );
	if ( editText.useOutline )
		AppendFlags( flags, SWFNAME_USEOUTLINE );
	if ( !flags.IsEmpty() )
		pDO->AddChild( new DisplayObject(TString(SWFNAME_FLAGS), flags) );

	pDO->AddChild( new DisplayObject(TString(SWFNAME_VARNAME), TString(editText.szVariableName)) );
	if ( editText.hasText )
		pDO->AddChild( new DisplayObject(TString(SWFNAME_INITIALTEXT), TString(editText.szInitialText)) );

	if ( editText.szVariableName )
		delete[] editText.szVariableName;
	if ( editText.szInitialText )
		delete[] editText.szInitialText;

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFSpriteTag &spriteTag )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	pDO->AppendValue(TString(" id="));
	pDO->AppendValue(TString(spriteTag.id));

	pDO->AddChild( new DisplayObject(TString(SWFNAME_ID), TString(spriteTag.id)) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_FRAMECOUNT), TString(spriteTag.frameCount)) );
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFPlaceObjectTag &place )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	pDO->AppendValue(TString(" id="));
	pDO->AppendValue(TString(place.characterID));
	pDO->AppendValue(TString(" depth="));
	pDO->AppendValue(TString(place.depth));

	pDO->AddChild( new DisplayObject(TString(SWFNAME_CHARACTERID), TString(place.characterID)) );
	pDO->AddChild( new DisplayObject(TString(SWFNAME_DEPTH), TString(place.depth)) );
	pDO->AddChild( DisplayObject::CreateDisplayObject(TString(SWFNAME_MATRIX), place.matrix) );
	if ( place.bHasColorXform )
		pDO->AddChild( DisplayObject::CreateDisplayObject(TString(SWFNAME_COLORXFORM), place.colorXform) );

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFPlaceObject2Tag &place )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	pDO->AppendValue(TString(" id="));
	pDO->AppendValue(TString(place.characterID));
	pDO->AppendValue(TString(" depth="));
	pDO->AppendValue(TString(place.depth));
	if ( place.bHasName && place.szName )
	{
		pDO->AppendValue(TString(" name=\""));
		pDO->AppendValue(TString(place.szName));
		pDO->AppendValue(TString("\""));
	}
	
	pDO->AddChild( new DisplayObject(TString(SWFNAME_DEPTH), TString(place.depth)) );
	if ( place.bHasCharacter )
		pDO->AddChild( new DisplayObject(TString(SWFNAME_CHARACTERID), TString(place.characterID)) );
	if ( place.bHasMatrix )
		pDO->AddChild( DisplayObject::CreateDisplayObject(TString(SWFNAME_MATRIX), place.matrix) );
	if ( place.bHasColorXform )
		pDO->AddChild( DisplayObject::CreateDisplayObject(TString(SWFNAME_COLORXFORM), place.colorXform) );
	if ( place.bHasRatio )
		pDO->AddChild( new DisplayObject(TString(SWFNAME_RATIO), TString(place.ratio)) );
	if ( place.bHasName && place.szName )
	{
		pDO->AddChild( new DisplayObject(TString(SWFNAME_NAME), TString(place.szName)) );
		delete[] place.szName;
	}
	if ( place.bHasClipDepth )
		pDO->AddChild( new DisplayObject(TString(SWFNAME_CLIPDEPTH), TString(place.clipDepth)) );

	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFRemoveObjectTag &remove )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	pDO->AppendValue(TString(" id="));
	pDO->AppendValue(TString(remove.characterID));
	pDO->AppendValue(TString(" depth="));
	pDO->AppendValue(TString(remove.depth));

	//pDO->AddChild( new DisplayObject(TString(SWFNAME_CHARACTERID), TString(remove.characterID)) );
	//pDO->AddChild( new DisplayObject(TString(SWFNAME_DEPTH), TString(remove.depth)) );
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFRemoveObject2Tag &remove )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	pDO->AppendValue(TString(" depth="));
	pDO->AppendValue(TString(remove.depth));

	//pDO->AddChild( new DisplayObject(TString(SWFNAME_DEPTH), TString(remove.depth)) );
	return pDO;
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const std::vector<SWFActionRecord>& actions, const long* spriteID )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);

	if (spriteID)
	{
		pDO->AddChild( new DisplayObject(TString(SWFNAME_CHARACTERID), TString(*spriteID)) );
	}

	for (std::vector<SWFActionRecord>::const_iterator pCurrRec = actions.begin(); pCurrRec != actions.end(); ++pCurrRec)
	{
		DisplayObject::AddDisplayObjects(pDO, *pCurrRec);
	}

	return pDO;
}

void DisplayObject::AddDisplayObjects(DisplayObject* pParent, const SWFActionRecord& action)
{
	DisplayObject* pDO = NULL;
	
	bool bHandled = false;
	TString actionCode;
	switch ( action.code )
	{
		// SWF 3
		case 0:						actionCode = "End";	break;
		case SWFACTION_GOTOFRAME:	actionCode = SWFACTIONNAME_GOTOFRAME;	break;
		case SWFACTION_GETURL:		actionCode = SWFACTIONNAME_GETURL;		break;
		case SWFACTION_NEXTFRAME:	actionCode = SWFACTIONNAME_NEXTFRAME;	break;
		case SWFACTION_PREVFRAME:	actionCode = SWFACTIONNAME_PREVFRAME;	break;
		case SWFACTION_PLAY:		actionCode = SWFACTIONNAME_PLAY;		break;
		case SWFACTION_STOP:		actionCode = SWFACTIONNAME_STOP;		break;
		case SWFACTION_TOGGLEQUAL:	actionCode = SWFACTIONNAME_TOGGLEQUAL;	break;
		case SWFACTION_STOPSOUNDS:	actionCode = SWFACTIONNAME_STOPSOUNDS;	break;
		case SWFACTION_WAITFRAME:	actionCode = SWFACTIONNAME_WAITFRAME;	break;
		case SWFACTION_SETTARGET:	actionCode = SWFACTIONNAME_SETTARGET;	break;
		case SWFACTION_GOTOLABEL:	actionCode = SWFACTIONNAME_GOTOLABEL;	break;
		case SWFACTION_POP:			actionCode = SWFACTIONNAME_POP;			break;
		case SWFACTION_ADD:			actionCode = SWFACTIONNAME_ADD;			break;
		case SWFACTION_SUBTRACT:	actionCode = SWFACTIONNAME_SUBTRACT;	break;
		case SWFACTION_MULTIPLY:	actionCode = SWFACTIONNAME_MULTIPLY;	break;
		case SWFACTION_DIVIDE:		actionCode = SWFACTIONNAME_DIVIDE;		break;
		case SWFACTION_EQUALS:		actionCode = SWFACTIONNAME_EQUALS;		break;
		case SWFACTION_LESS:		actionCode = SWFACTIONNAME_LESS;		break;
		case SWFACTION_AND:			actionCode = SWFACTIONNAME_AND;			break;
		case SWFACTION_OR:			actionCode = SWFACTIONNAME_OR;			break;
		case SWFACTION_NOT:			actionCode = SWFACTIONNAME_NOT;			break;
		// SWF 4 (Strings)
		case SWFACTION_STREQUALS:	actionCode = SWFACTIONNAME_STREQUALS;	break;
		case SWFACTION_STRLEN:		actionCode = SWFACTIONNAME_STRLEN;		break;
		case SWFACTION_STRADD:		actionCode = SWFACTIONNAME_STRADD;		break;
		case SWFACTION_STREXTRACT:	actionCode = SWFACTIONNAME_STREXTRACT;	break;
		case SWFACTION_STRLESS:		actionCode = SWFACTIONNAME_STRLESS;		break;
		case SWFACTION_MBSTRLEN:	actionCode = SWFACTIONNAME_MBSTRLEN;	break;
		case SWFACTION_MBSTREXTRACT:actionCode = SWFACTIONNAME_MBSTREXTRACT;break;
		// SWF 4 (Conversion)
		case SWFACTION_TOINT:		actionCode = SWFACTIONNAME_TOINT;		break;
		case SWFACTION_CHARTOASCII:	actionCode = SWFACTIONNAME_CHARTOASCII;	break;
		case SWFACTION_ASCIITOCHAR:	actionCode = SWFACTIONNAME_ASCIITOCHAR;	break;
		case SWFACTION_MBCHARTOASCII:	actionCode = SWFACTIONNAME_MBCHARTOASCII;	break;
		case SWFACTION_MBASCIITOCHAR:	actionCode = SWFACTIONNAME_MBASCIITOCHAR;	break;
		// SWF 4 (Control Flow)
		case SWFACTION_JUMP:		actionCode = SWFACTIONNAME_JUMP;		break;
		case SWFACTION_IF:			actionCode = SWFACTIONNAME_IF;			break;
		case SWFACTION_CALL:		actionCode = SWFACTIONNAME_CALL;		break;
		// SWF 4 (Variables)
		case SWFACTION_GETVAR:		actionCode = SWFACTIONNAME_GETVAR;		break;
		case SWFACTION_SETVAR:		actionCode = SWFACTIONNAME_SETVAR;		break;
		// SWF 4 (Movie Control)
		case SWFACTION_GETURL2:		
		{
			pDO = new DisplayObject(SWFACTIONNAME_GETURL2, action.description.c_str());
			pParent->AddChild(pDO);
			bHandled = true;
			break;
		}
		case SWFACTION_GOTOFRAME2:	actionCode = SWFACTIONNAME_GOTOFRAME2;	break;
		case SWFACTION_SETTARGET2:	actionCode = SWFACTIONNAME_SETTARGET2;	break;
		case SWFACTION_GETPROP:		actionCode = SWFACTIONNAME_GETPROP;		break;
		case SWFACTION_SETPROP:		actionCode = SWFACTIONNAME_SETPROP;		break;
		case SWFACTION_CLONESPRITE:	actionCode = SWFACTIONNAME_CLONESPRITE;	break;
		case SWFACTION_REMOVESPRITE:actionCode = SWFACTIONNAME_REMOVESPRITE;break;
		case SWFACTION_STARTDRAG:	actionCode = SWFACTIONNAME_STARTDRAG;	break;
		case SWFACTION_ENDDRAG:		actionCode = SWFACTIONNAME_ENDDRAG;		break;
		case SWFACTION_WAITFRAME2:	actionCode = SWFACTIONNAME_WAITFRAME2;	break;
		// SWF 4 (Utilities)
		case SWFACTION_TRACE:		actionCode = SWFACTIONNAME_TRACE;		break;
		case SWFACTION_GETTIME:		actionCode = SWFACTIONNAME_GETTIME;		break;
		case SWFACTION_RAND:		actionCode = SWFACTIONNAME_RAND;		break;

		// SWF 5 (Script Object)
		case SWFACTION_CALLFUNC:	actionCode = SWFACTIONNAME_CALLFUNC;	break;
		case SWFACTION_CALLMETH:	actionCode = SWFACTIONNAME_CALLMETH;	break;
		case SWFACTION_PUSH:	
		{	
			char szTmpBuffer[64];
			if ( action.theStrings.empty() == false )
			{
				for (U32 i = 0; i < action.theStrings.size(); ++i)
				{
					pParent->AddChild(new DisplayObject("Push", action.theStrings[i].c_str()));
				}
				bHandled = true;
			}
			else
			{
				sprintf(szTmpBuffer, "Push (BROKEN)");
				pDO = DisplayObject::CreateDisplayObject(szTmpBuffer, 0);
				pParent->AddChild(pDO);
				bHandled = true;
			}
			break;
		}
		case SWFACTION_CONSTPOOL:	
		{	
			char szTmpBuffer[64];
			if ( action.theStrings.empty() == false )
			{
				sprintf(szTmpBuffer, "Constant Pool (%d strings)", action.theStrings.size());
				pDO = DisplayObject::CreateDisplayObject(szTmpBuffer, 0);
				for (U32 i = 0; i < action.theStrings.size(); ++i)
				{
					pDO->AddChild(DisplayObject::CreateDisplayObject(action.theStrings[i].c_str(), 0));
				}
				pParent->AddChild(pDO);
				bHandled = true;
			}
			else
			{
				sprintf(szTmpBuffer, "Constant Pool (BROKEN)");
				pDO = DisplayObject::CreateDisplayObject(szTmpBuffer, 0);
				pParent->AddChild(pDO);
				bHandled = true;
			}
			break;
		}
		case SWFACTION_DEFFUNC:
		case SWFACTION_DEFFUNC2:
		{
			const char* nm = action.code == SWFACTION_DEFFUNC ? SWFACTIONNAME_DEFFUNC : SWFACTIONNAME_DEFFUNC2;
			if (action.description.empty())
				pDO = DisplayObject::CreateDisplayObject(nm, 0);
			else
				pDO = new DisplayObject(nm, action.description.c_str());
			if (action.actions.empty() == false)
			{
				for (std::vector<SWFActionRecord>::const_iterator pCurrRec = action.actions.begin(); pCurrRec != action.actions.end(); ++pCurrRec)
				{
					DisplayObject::AddDisplayObjects(pDO, *pCurrRec);
				}
			}
			pParent->AddChild(pDO);
			bHandled = true;
			break;
		}
		case SWFACTION_DEFLOCAL:	actionCode = SWFACTIONNAME_DEFLOCAL;	break;
		case SWFACTION_DEFLOCAL2:	actionCode = SWFACTIONNAME_DEFLOCAL2;	break;
		case SWFACTION_DELETE:		actionCode = SWFACTIONNAME_DELETE;		break;
		case SWFACTION_DELETE2:		actionCode = SWFACTIONNAME_DELETE2;		break;
		case SWFACTION_ENUM:		actionCode = SWFACTIONNAME_ENUM;		break;
		case SWFACTION_EQUALS2:		actionCode = SWFACTIONNAME_EQUALS2;		break;
		case SWFACTION_GETMEMBER:	actionCode = SWFACTIONNAME_GETMEMBER;	break;
		case SWFACTION_INITARRAY:	actionCode = SWFACTIONNAME_INITARRAY;	break;
		case SWFACTION_INITOBJECT:	actionCode = SWFACTIONNAME_INITOBJECT;	break;
		case SWFACTION_NEWMETH:		actionCode = SWFACTIONNAME_NEWMETH;		break;
		case SWFACTION_NEWOBJECT:	actionCode = SWFACTIONNAME_NEWOBJECT;	break;
		case SWFACTION_SETMEMBER:	actionCode = SWFACTIONNAME_SETMEMBER;	break;
		case SWFACTION_TARGETPATH:	actionCode = SWFACTIONNAME_TARGETPATH;	break;
		case SWFACTION_WITH:		actionCode = SWFACTIONNAME_WITH;		break;
		// SWF 5 (Type)
		case SWFACTION_TONUMBER:	actionCode = SWFACTIONNAME_TONUMBER;	break;
		case SWFACTION_TOSTRING:	actionCode = SWFACTIONNAME_TOSTRING;	break;
		case SWFACTION_TYPEOF:		actionCode = SWFACTIONNAME_TYPEOF;		break;
		// SWF 5 (Math)
		case SWFACTION_ADD2:		actionCode = SWFACTIONNAME_ADD2;		break;
		case SWFACTION_LESS2:		actionCode = SWFACTIONNAME_LESS2;		break;
		case SWFACTION_MODULO:		actionCode = SWFACTIONNAME_MODULO;		break;
		// SWF 5 (Stack Op)
		case SWFACTION_BITAND:		actionCode = SWFACTIONNAME_BITAND;		break;
		case SWFACTION_BITLSHIFT:	actionCode = SWFACTIONNAME_BITLSHIFT;	break;
		case SWFACTION_BITOR:		actionCode = SWFACTIONNAME_BITOR;		break;
		case SWFACTION_BITRSHIFT:	actionCode = SWFACTIONNAME_BITRSHIFT;	break;
		case SWFACTION_BITURSHIFT:	actionCode = SWFACTIONNAME_BITURSHIFT;	break;
		case SWFACTION_BITXOR:		actionCode = SWFACTIONNAME_BITXOR;		break;
		case SWFACTION_DECREMENT:	actionCode = SWFACTIONNAME_DECREMENT;	break;
		case SWFACTION_INCREMENT:	actionCode = SWFACTIONNAME_INCREMENT;	break;
		case SWFACTION_PUSHDUP:		actionCode = SWFACTIONNAME_PUSHDUP;		break;
		case SWFACTION_RETURN:		actionCode = SWFACTIONNAME_RETURN;		break;
		case SWFACTION_STACKSWAP:	actionCode = SWFACTIONNAME_STACKSWAP;	break;
		case SWFACTION_STOREREG:	actionCode = SWFACTIONNAME_STOREREG;	break;

		// SWF 6
		case SWFACTION_INSTANCEOF:	actionCode = SWFACTIONNAME_INSTANCEOF;	break;
		case SWFACTION_ENUM2:		actionCode = SWFACTIONNAME_ENUM2;		break;
		case SWFACTION_STRICTEQUALS:actionCode = SWFACTIONNAME_STRICTEQUALS;break;
		case SWFACTION_GREATER:		actionCode = SWFACTIONNAME_GREATER;		break;
		case SWFACTION_STRGREATER:	actionCode = SWFACTIONNAME_STRGREATER;	break;
		default:					
		{
			char szTmpBuffer[64];
			sprintf(szTmpBuffer, "Unrecognized Action Code (0x%02x)", action.code);
			pDO = DisplayObject::CreateDisplayObject(szTmpBuffer, 0);
			pParent->AddChild(pDO);
			bHandled = true;
			break;
		}
	}

	if (!bHandled)
	{
		if (action.description.empty())
			pDO = DisplayObject::CreateDisplayObject(actionCode, 0);
		else
			pDO = new DisplayObject(actionCode, action.description.c_str());
		pParent->AddChild(pDO);
	}
}

DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const long bytelen, const SWFHeader &fileHeader )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE), bytelen);
	TString strSig = ( fileHeader.bCompressed ) ? "FWC (Compressed SWF file)" : "FWS";
	pDO->AddChild( new DisplayObject(TString("Signature"), strSig) );
	pDO->AddChild( new DisplayObject(TString("Version"), TString(fileHeader.iVersion)) );
	pDO->AddChild( new DisplayObject(TString("FileLength"), TString(fileHeader.iFileLen)) );
	pDO->AddChild( DisplayObject::CreateDisplayObject(TString("FrameSize"), fileHeader.rFrameSize) );
	pDO->AddChild( new DisplayObject(TString("FrameRate"), TString(fileHeader.iFrameRate)) );
	pDO->AddChild( new DisplayObject(TString("FrameCount"), TString(fileHeader.iFrameCount)) );
	return pDO;
}


DisplayObject *DisplayObject::CreateDisplayObject( const TString &name, const SWFFileAnalysis &swfAnalysis )
{
	DisplayObject *pDO = new DisplayObject(name, TString(EMPTY_SPACE));

	char szShapeOut[64];
	char szBitmapOut[64];
	char szTextOut[64];
	char szControlOut[64];
	char szActionOut[64];
	char szInitActionOut[64];
	char szConstPoolOut[64];
	char szSoundOut[64];
	char szButtonOut[64];
	char szDisplayOut[64];
	char szOtherOut[64];

	sprintf( szShapeOut,	"%i bytes  (%.2f percent)", swfAnalysis.shape,	(float)(swfAnalysis.shape*100.0)	/(float)(swfAnalysis.fileSize) );
	sprintf( szBitmapOut,	"%i bytes  (%.2f percent)", swfAnalysis.bitmap,	(float)(swfAnalysis.bitmap*100.0)	/(float)(swfAnalysis.fileSize) );
	sprintf( szTextOut,		"%i bytes  (%.2f percent)", swfAnalysis.text,	(float)(swfAnalysis.text*100.0)	/(float)(swfAnalysis.fileSize) );
	sprintf( szControlOut,	"%i bytes  (%.2f percent)", swfAnalysis.control,(float)(swfAnalysis.control*100.0)/(float)(swfAnalysis.fileSize) );
	sprintf( szActionOut,	"%i bytes  (%.2f percent)", swfAnalysis.action,	(float)(swfAnalysis.action*100.0)	/(float)(swfAnalysis.fileSize) );
	sprintf( szInitActionOut,	"%i bytes  (%.2f percent)", swfAnalysis.initaction,	(float)(swfAnalysis.initaction*100.0)	/(float)(swfAnalysis.fileSize) );
	sprintf( szConstPoolOut,"%i bytes  (also included in DoAction/DoInitAction))", swfAnalysis.constPool);
	sprintf( szSoundOut,	"%i bytes  (%.2f percent)", swfAnalysis.sound,	(float)(swfAnalysis.sound*100.0)	/(float)(swfAnalysis.fileSize) );
	sprintf( szButtonOut,	"%i bytes  (%.2f percent)", swfAnalysis.button,	(float)(swfAnalysis.button*100.0)	/(float)(swfAnalysis.fileSize) );
	sprintf( szDisplayOut,	"%i bytes  (%.2f percent)", swfAnalysis.display,(float)(swfAnalysis.display*100.0)/(float)(swfAnalysis.fileSize) );
	sprintf( szOtherOut,	"%i bytes  (%.2f percent)", swfAnalysis.other,	(float)(swfAnalysis.other*100.0)	/(float)(swfAnalysis.fileSize) );

	char szScratch[64];

	// shapes
	DisplayObject *pItemDO = new DisplayObject( TString("Shape"), TString(szShapeOut));
	sprintf( szScratch,	"%i instances", swfAnalysis.shapeCount);
	pItemDO->AddChild( new DisplayObject(TString("Count"), TString(szScratch)));
	pDO->AddChild( pItemDO );

	//bitmaps
	pItemDO = new DisplayObject( TString("Bitmap"), TString(szBitmapOut));
	sprintf( szScratch,	"%i instances", swfAnalysis.bitmapCount);
	pItemDO->AddChild( new DisplayObject(TString("Count"), TString(szScratch)));
	sprintf( szScratch,	"%i instances", swfAnalysis.definebitsCount);
	pItemDO->AddChild( new DisplayObject(TString("DefineBits"), TString(szScratch)));
	sprintf( szScratch,	"%i instances", swfAnalysis.jpegtablesCount);
	pItemDO->AddChild( new DisplayObject(TString("JPEGTables"), TString(szScratch)));
	sprintf( szScratch,	"%i instances, %i bytes", swfAnalysis.jpeg2Count, swfAnalysis.jpeg2);
	pItemDO->AddChild( new DisplayObject(TString("JPEG2"), TString(szScratch)));
	sprintf( szScratch,	"%i instances", swfAnalysis.jpeg3Count);
	pItemDO->AddChild( new DisplayObject(TString("JPEG3"), TString(szScratch)));
	sprintf( szScratch,	"%i instances", swfAnalysis.losslessCount);
	pItemDO->AddChild( new DisplayObject(TString("Lossless"), TString(szScratch)));
	sprintf( szScratch,	"%i instances, %i bytes", swfAnalysis.lossless2Count, swfAnalysis.lossless2);
	pItemDO->AddChild( new DisplayObject(TString("Lossless2"), TString(szScratch)));
	pDO->AddChild( pItemDO );

	//text
	pItemDO = new DisplayObject( TString("Text"), TString(szTextOut));
	sprintf( szScratch,	"%i instances", swfAnalysis.textCount);
	pItemDO->AddChild( new DisplayObject(TString("Count"), TString(szScratch)));
	sprintf( szScratch,	"%i instances", swfAnalysis.definefontCount);
	pItemDO->AddChild( new DisplayObject(TString("DefineFont"), TString(szScratch)));
	sprintf( szScratch,	"%i instances, %i bytes", swfAnalysis.definefont2Count, swfAnalysis.definefont2);
	pItemDO->AddChild( new DisplayObject(TString("DefineFont2"), TString(szScratch)));
	sprintf( szScratch,	"%i instances", swfAnalysis.definefontinfoCount);
	pItemDO->AddChild( new DisplayObject(TString("DefineFontInfo"), TString(szScratch)));
	sprintf( szScratch,	"%i instances", swfAnalysis.definefontinfo2Count);
	pItemDO->AddChild( new DisplayObject(TString("DefineFontInfo2"), TString(szScratch)));
	sprintf( szScratch,	"%i instances, %i bytes", swfAnalysis.definetextCount, swfAnalysis.definetext);
	pItemDO->AddChild( new DisplayObject(TString("DefineText"), TString(szScratch)));
	sprintf( szScratch,	"%i instances", swfAnalysis.definetext2Count);
	pItemDO->AddChild( new DisplayObject(TString("DefineText2"), TString(szScratch)));
	sprintf( szScratch,	"%i instances", swfAnalysis.edittextCount);
	pItemDO->AddChild( new DisplayObject(TString("EditText"), TString(szScratch)));
	pDO->AddChild( pItemDO );

	pDO->AddChild( new DisplayObject(TString("Control"),		TString(szControlOut)) );
	pDO->AddChild( new DisplayObject(TString("DoAction"),		TString(szActionOut)) );
	pDO->AddChild( new DisplayObject(TString("DoInitAction"),	TString(szInitActionOut)) );
	pDO->AddChild( new DisplayObject(TString("  ConstPools"),	TString(szConstPoolOut)) );
	pDO->AddChild( new DisplayObject(TString("Sound"),			TString(szSoundOut)) );
	pDO->AddChild( new DisplayObject(TString("Button"),			TString(szButtonOut)) );

	//display items
	pItemDO = new DisplayObject( TString("Display"), TString(szDisplayOut));
	sprintf( szScratch,	"%i instances", swfAnalysis.displayCount);
	pItemDO->AddChild( new DisplayObject(TString("Count"), TString(szScratch)));
	sprintf( szScratch,	"%i instances, %i bytes", swfAnalysis.settabCount, swfAnalysis.settab);
	pItemDO->AddChild( new DisplayObject(TString("SetTab"), TString(szScratch)));
	sprintf( szScratch,	"%i instances, %i bytes", swfAnalysis.definespriteCount, swfAnalysis.definesprite);
	pItemDO->AddChild( new DisplayObject(TString("DefineSprite"), TString(szScratch)));
	pDO->AddChild( pItemDO );

	pDO->AddChild( new DisplayObject(TString("Other"),			TString(szOtherOut)) );

	return pDO;
}

void AppendDispObjList( DispObjectList *pPrev, DispObjectList *pNext )
{
	assert( pPrev );
	DispObjectList *pCurr = pPrev;
	if ( pCurr )
	{
		while( pCurr->pNext )
			pCurr = pCurr->pNext;
		pCurr->pNext = pNext;
	}
}
