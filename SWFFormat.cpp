#include "stdafx.h"
#include "SWFFormat.h"
#include "SWFError.h"
#include "SWFFormatDataSource.h"
#include "SWFFile.h"
#include <vector>

typedef struct WIN_CREATE_CTRL_PARAM {
	HINSTANCE appInst;
	HWND parent;
} WIN_CREATE_CTRL_PARAM;

#define NO_SWF_ERROR	(iSuccess == SWF_ERR_NONE)

/******************************************************************************
 * 
 * Utilities methods to track number of bytes read (among other things)
 *
 *****************************************************************************/

long ReadVariable( void *pData, int size, int number, SWFFile &swfFile, int &success )
{
	if ( success != SWF_ERR_NONE )
		return 0;

	long bytesToRead = size * number;
	success = swfFile.ReadBytes( pData, bytesToRead );

	return( (success == SWF_ERR_NONE) ? bytesToRead : 0 );
}

U8 ReadByte( SWFFile &swfFile, int &success )
{
	if ( success != SWF_ERR_NONE )
		return 0;

	U8 retValue = 0;
	success = swfFile.ReadBytes( &retValue, 1 );

	return retValue;
}

U16 ReadWord( SWFFile &swfFile, int &success )
{
	if ( success != SWF_ERR_NONE )
		return 0;

	U16 retValue = 0;
	U8 bytes[2];
	success = swfFile.ReadBytes( &bytes, 2 );
	if ( success == SWF_ERR_NONE )
		retValue = (U16) bytes[0] | ((U16) bytes[1] << 8);

	return retValue;
}

std::string ReadString(SWFFile &swfFile, int &success)	// assumes UTF8 format
{
	std::string newStr("");
	for (;;)
	{
		U8 ch = ReadByte( swfFile, success );
		if (ch == 0)
			break;
		newStr.push_back(ch);
	}
	return newStr;
}

S16 ReadSWord( SWFFile &swfFile, int &success )
{
	return( (S16) ReadWord(swfFile, success) );
}

U32 ReadDoubleWord( SWFFile &swfFile, int &success )
{
	if ( success != SWF_ERR_NONE )
		return 0;

	U32 retValue = 0;
	U8 bytes[4];
	success = swfFile.ReadBytes( &bytes, 4 );
	if ( success == SWF_ERR_NONE )
		retValue = (U32) bytes[0] | ((U32) bytes[1] << 8) | ((U32) bytes[2] << 16) | ((U32) bytes[3] << 24);

	return retValue;
}

U32 ReadBits( SWFFile &swfFile, U32 nBits, U8 &bitBuffer, S32 &bitPos )
{
	U32 v = 0;
	while ( true )
	{
		S32 s = nBits - bitPos;
		if ( s > 0 )
		{
			// Consume the entire buffer
			v |= bitBuffer << s;
			nBits -= bitPos;

			// Get the next Buffer
			swfFile.ReadBytes( &bitBuffer, 1 );
			bitPos = 8;
		}
		else
		{
			// Consume a portion of the buffer
			v |= bitBuffer >> -s;
			bitPos -= nBits;
			bitBuffer &= 0xff >> (8 - bitPos); // mask off the consumed bits

			return v;
		}
	}
}

S32 ReadSBits( SWFFile &swfFile, U32 nBits, U8 &bitBuffer, S32 &bitPos )
{
	S32 v = (S32) ReadBits( swfFile, nBits, bitBuffer, bitPos );
	
	// Is the number negative?
	if ( v & (1L << (nBits - 1)) )
		v |= -1L << nBits;	// Yes, extend the sign

	return v;
}

/******************************************************************************
 *
 * Primitive reading methods
 *
 *****************************************************************************/
int ReadSWFRect( SWFFile &swfFile, SWFRect &rect )
{
	int iSuccess = SWF_ERR_NONE;

	S32 bitPos = 0;
	U8 bitBuffer = 0;
	U32 iBits = ReadBits( swfFile, 5, bitBuffer, bitPos );
	rect.left	= ReadSBits( swfFile, iBits, bitBuffer, bitPos );
	rect.right	= ReadSBits( swfFile, iBits, bitBuffer, bitPos );
	rect.top	= ReadSBits( swfFile, iBits, bitBuffer, bitPos );
	rect.bottom	= ReadSBits( swfFile, iBits, bitBuffer, bitPos );

	return iSuccess;
}

int ReadSWFRGB( SWFFile &swfFile, RGB &rgb )
{
	int iSuccess = SWF_ERR_NONE;
	ReadVariable( &rgb, sizeof(RGB), 1, swfFile, iSuccess );
	return iSuccess;
}

int ReadSWFRGBA( SWFFile &swfFile, RGBA &rgba )
{
	int iSuccess = SWF_ERR_NONE;
	ReadVariable( &rgba, sizeof(RGBA), 1, swfFile, iSuccess );
	return iSuccess;
}

int ReadSWFMatrix( SWFFile &swfFile, SWFMatrix &matrix )
{
	int iSuccess = SWF_ERR_NONE;

	S32 bitPos = 0;
	U8 bitBuffer = 0;
	U32 hasScale = ReadBits( swfFile, 1, bitBuffer, bitPos );
	if ( hasScale )
	{
		U32 nBits = ReadBits( swfFile, 5, bitBuffer, bitPos );
		matrix.xScale = ReadSBits( swfFile, nBits, bitBuffer, bitPos );
		matrix.yScale = ReadSBits( swfFile, nBits, bitBuffer, bitPos );
		matrix.hasScale = true;
	}
	else
	{
		matrix.hasScale = false;
	}

	U32 hasRotate = ReadBits( swfFile, 1, bitBuffer, bitPos );
	if ( hasRotate )
	{
		U32 nBits = ReadBits( swfFile, 5, bitBuffer, bitPos );
		matrix.rotateSkew0 = ReadSBits( swfFile, nBits, bitBuffer, bitPos );
		matrix.rotateSkew1 = ReadSBits( swfFile, nBits, bitBuffer, bitPos );
		matrix.hasRotate = true;
	}
	else
	{
		matrix.hasRotate = false;
	}

	U32 nBits = ReadBits( swfFile, 5, bitBuffer, bitPos );
	matrix.xTranslate = ReadSBits( swfFile, nBits, bitBuffer, bitPos );
	matrix.yTranslate = ReadSBits( swfFile, nBits, bitBuffer, bitPos );

	return iSuccess;
}

// Note that color.bHasAlpha should be set before calling this...
int ReadSWFColorXForm( SWFFile &swfFile, SWFColorXForm &color )
{
	int iSuccess = SWF_ERR_NONE;

	S32 bitPos = 0;
	U8 bitBuffer = 0;

	color.bHasAdd = ReadBits( swfFile, 1, bitBuffer, bitPos ) != 0;
	color.bHasMult = ReadBits( swfFile, 1, bitBuffer, bitPos ) != 0;

	U32 numBits = ReadBits( swfFile, 4, bitBuffer, bitPos );
	if ( color.bHasMult )
	{
		color.multRed = ReadSBits( swfFile, numBits, bitBuffer, bitPos );
		color.multGreen = ReadSBits( swfFile, numBits, bitBuffer, bitPos );
		color.multBlue = ReadSBits( swfFile, numBits, bitBuffer, bitPos );
		if ( color.bHasAlpha )
			color.multAlpha = ReadSBits( swfFile, numBits, bitBuffer, bitPos );
	}

	if ( color.bHasAdd )
	{
		color.addRed = ReadSBits( swfFile, numBits, bitBuffer, bitPos );
		color.addGreen = ReadSBits( swfFile, numBits, bitBuffer, bitPos );
		color.addBlue = ReadSBits( swfFile, numBits, bitBuffer, bitPos );
		if ( color.bHasAlpha )
			color.addAlpha = ReadSBits( swfFile, numBits, bitBuffer, bitPos );
	}

	return iSuccess;
}

int ReadSWFGradient( SWFFile &swfFile, SWFShapeType type, SWFGradient &gradient )
{
	int iSuccess = SWF_ERR_NONE;

	gradient.numControlPoints = ReadByte( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
	{
		SWFGradientRecord *pNewRec = NULL;
		SWFGradientRecord *pCurrRec = gradient.pRecord;
		for ( int i=0; (i<gradient.numControlPoints) && NO_SWF_ERROR; i++ )
		{
			pNewRec = new SWFGradientRecord;
			pNewRec->next = NULL;

			pNewRec->ratio = ReadByte( swfFile, iSuccess );
			if ( NO_SWF_ERROR )
			{
				if ( type == SWFShape3 )
					iSuccess = ReadSWFRGBA( swfFile, pNewRec->u.shape3Color );
				else
					iSuccess = ReadSWFRGB( swfFile, pNewRec->u.color );
			}

			if ( NO_SWF_ERROR )
			{
				if ( pCurrRec == NULL )
					gradient.pRecord = pNewRec;
				else
					pCurrRec->next = pNewRec;
				pCurrRec = pNewRec;
			}
			else
			{
				delete pNewRec;
			}
		}
	}

	return iSuccess;
}

/******************************************************************************
 *
 * SWF Shape reading methods
 *
 *****************************************************************************/
int ReadSWFShapeTag( SWFFile &swfFile, SWFShapeTag &shapeTag )
{
	int iSuccess = SWF_ERR_NONE;

	// ReadVariable( &shapeTag.id, U16_SIZE, 1, swfFile, iSuccess );
	shapeTag.id = ReadWord( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
		iSuccess = ReadSWFRect( swfFile, shapeTag.bounds );

	if ( NO_SWF_ERROR )
		iSuccess = ReadSWFShapeWithStyle( swfFile, shapeTag.type, shapeTag.shapeWithStyle );

	return iSuccess;
}

int ReadSWFShapeRecords( SWFFile &swfFile, SWFShapeType type, SWFShapeWithStyle &shapeWithStyle )
{
	int iSuccess = SWF_ERR_NONE;

	SWFShapeRecord *pNewShapeRec = NULL;
	SWFShapeRecord *pCurrShapeRec = NULL;
	U32 currNumFillBits = shapeWithStyle.numFillBits;
	U32 currNumLineBits = shapeWithStyle.numLineBits;

	bool bDone = false;
	U8 bitBuffer = 0;
	S32 bitPos = 0;
	while ( !bDone && NO_SWF_ERROR )
	{
		pNewShapeRec = new SWFShapeRecord;
		pNewShapeRec->next = NULL;

		U32 isEdge = ReadBits( swfFile, 1, bitBuffer, bitPos );
		if ( isEdge ) // it's an edge record, straight or curved?
		{
			U32 straightEdge = ReadBits( swfFile, 1, bitBuffer, bitPos );
			U32 numBits = ReadBits( swfFile, 4, bitBuffer, bitPos );
			if ( straightEdge )
			{
				SWFStraightEdgeRecord *pNewEdgeRecord = new SWFStraightEdgeRecord;

				U32 generalLine = ReadBits( swfFile, 1, bitBuffer, bitPos );
				if ( generalLine )
				{
					pNewEdgeRecord->lineType = SWFGeneralLine;
					pNewEdgeRecord->deltaX = ReadSBits( swfFile, numBits+2, bitBuffer, bitPos );
					pNewEdgeRecord->deltaY = ReadSBits( swfFile, numBits+2, bitBuffer, bitPos );
				}
				else // it's a vertical or horizontal line
				{
					pNewEdgeRecord->lineType = ReadBits(swfFile, 1, bitBuffer, bitPos) ? SWFVerticalLine : SWFHorizontalLine;
					if ( pNewEdgeRecord->lineType == SWFVerticalLine )
						pNewEdgeRecord->vertDeltaY = ReadSBits( swfFile, numBits+2, bitBuffer, bitPos );
					else // SWFHorizontalLine
						pNewEdgeRecord->horzDeltaX = ReadSBits( swfFile, numBits+2, bitBuffer, bitPos );
				}

				pNewShapeRec->type = SWFsrtStraightEdge;
				pNewShapeRec->pRecord = pNewEdgeRecord;
			}
			else // it's a curved edge
			{
				SWFCurvedEdgeRecord *pNewEdgeRecord = new SWFCurvedEdgeRecord;
				pNewEdgeRecord->controlDeltaX = ReadSBits( swfFile, numBits+2, bitBuffer, bitPos );
				pNewEdgeRecord->controlDeltaY = ReadSBits( swfFile, numBits+2, bitBuffer, bitPos );
				pNewEdgeRecord->anchorDeltaX = ReadSBits( swfFile, numBits+2, bitBuffer, bitPos );
				pNewEdgeRecord->anchorDeltaY = ReadSBits( swfFile, numBits+2, bitBuffer, bitPos );

				pNewShapeRec->type = SWFsrtCurvedEdge;
				pNewShapeRec->pRecord = pNewEdgeRecord;
			}
		}
		else // it's a non-edge record
		{
			U8 flags = (U8) ReadBits( swfFile, 5, bitBuffer, bitPos );
			if ( flags == 0 )	// it's an end-of-shape record
			{
				pNewShapeRec->type = SWFsrtEndShape;
				pNewShapeRec->pRecord = NULL;
				bDone = true;
			}
			else // it's a style change record
			{
				pNewShapeRec->type = SWFsrtStyleChange;
				SWFStyleChangeRecord *pNewStyleChangeRecord = new SWFStyleChangeRecord;
				pNewStyleChangeRecord->bNewStyles		  = ((flags & 0x10) != 0);
				pNewStyleChangeRecord->bLineStyleChanged  = ((flags & 0x08) != 0);
				pNewStyleChangeRecord->bFillStyle1Changed = ((flags & 0x04) != 0);
				pNewStyleChangeRecord->bFillStyle0Changed = ((flags & 0x02) != 0);
				pNewStyleChangeRecord->bMoveTo			  = ((flags & 0x01) != 0);

				if ( pNewStyleChangeRecord->bMoveTo )
				{
					U32 moveToBits = ReadBits( swfFile, 5, bitBuffer, bitPos );
					pNewStyleChangeRecord->moveDeltaX = ReadSBits( swfFile, moveToBits, bitBuffer, bitPos );
					pNewStyleChangeRecord->moveDeltaY = ReadSBits( swfFile, moveToBits, bitBuffer, bitPos );
				}

				if ( pNewStyleChangeRecord->bFillStyle0Changed )
					pNewStyleChangeRecord->fillStyle0 = ReadBits( swfFile, currNumFillBits, bitBuffer, bitPos );

				if ( pNewStyleChangeRecord->bFillStyle1Changed )
					pNewStyleChangeRecord->fillStyle1 = ReadBits( swfFile, currNumFillBits, bitBuffer, bitPos );

				if ( pNewStyleChangeRecord->bLineStyleChanged )
					pNewStyleChangeRecord->lineStyle = ReadBits( swfFile, currNumLineBits, bitBuffer, bitPos );

				if ( pNewStyleChangeRecord->bNewStyles )
				{
					if ( NO_SWF_ERROR )
						iSuccess = ReadSWFFillStyleArray( swfFile, type, pNewStyleChangeRecord->fillStyleArray );
					if ( NO_SWF_ERROR )
						iSuccess = ReadSWFLineStyleArray( swfFile, type, pNewStyleChangeRecord->lineStyleArray );

					if ( NO_SWF_ERROR )
					{
						bitBuffer = 0;
						bitPos = 0;
						pNewStyleChangeRecord->numFillBits = (short) ReadBits( swfFile, 4, bitBuffer, bitPos );
						pNewStyleChangeRecord->numLineBits = (short) ReadBits( swfFile, 4, bitBuffer, bitPos );
						currNumFillBits = pNewStyleChangeRecord->numFillBits;
						currNumLineBits = pNewStyleChangeRecord->numLineBits;
					}
				}

				pNewShapeRec->pRecord = pNewStyleChangeRecord;
			}
		}

		if ( pCurrShapeRec == NULL )
			shapeWithStyle.pRecordList = pNewShapeRec;
		else
			pCurrShapeRec->next = pNewShapeRec;
		pCurrShapeRec = pNewShapeRec;
	}

	return iSuccess;
}

int ReadSWFShapeWithStyle( SWFFile &swfFile, SWFShapeType type, SWFShapeWithStyle &shapeWithStyle )
{
	int iSuccess = SWF_ERR_NONE;

	iSuccess = ReadSWFFillStyleArray( swfFile, type, shapeWithStyle.fillStyleArray );
	if ( NO_SWF_ERROR )
		iSuccess = ReadSWFLineStyleArray( swfFile, type, shapeWithStyle.lineStyleArray );

	if ( NO_SWF_ERROR )
	{
		U8 bitBuffer = 0;
		S32 bitPos = 0;
		shapeWithStyle.numFillBits = (short) ReadBits( swfFile, 4, bitBuffer, bitPos );
		shapeWithStyle.numLineBits = (short) ReadBits( swfFile, 4, bitBuffer, bitPos );

		iSuccess = ReadSWFShapeRecords( swfFile, type, shapeWithStyle );
	}

	return iSuccess;
}

int ReadSWFShape( SWFFile &swfFile, SWFShapeWithStyle &shapeWithStyle )
{
	int iSuccess = SWF_ERR_NONE;

	shapeWithStyle.fillStyleArray.count = 0;
	shapeWithStyle.fillStyleArray.pStyle = NULL;
	shapeWithStyle.lineStyleArray.count = 0;
	shapeWithStyle.lineStyleArray.pStyle = NULL;

	U8 bitBuffer = 0;
	S32 bitPos = 0;
	shapeWithStyle.numFillBits = (short) ReadBits( swfFile, 4, bitBuffer, bitPos );
	shapeWithStyle.numLineBits = (short) ReadBits( swfFile, 4, bitBuffer, bitPos );

	// Use SWF Shape1 as default shape... no alpha for text...
	iSuccess = ReadSWFShapeRecords( swfFile, SWFShape1, shapeWithStyle );

	return iSuccess;
}

int ReadSWFFillStyleArray( SWFFile &swfFile, SWFShapeType type, SWFFillStyleArray &fillStyleArray )
{
	int iSuccess = SWF_ERR_NONE;

	U16 fillStyleCount = ReadByte( swfFile, iSuccess );
	if ( NO_SWF_ERROR && (fillStyleCount == 0xFF) )
		// ReadVariable( &fillStyleCount, U16_SIZE, 1, swfFile, iSuccess );
		fillStyleCount = ReadWord( swfFile, iSuccess );
	fillStyleArray.count = fillStyleCount;

	SWFFillStyle *pNewFillStyle = NULL;
	SWFFillStyle *pCurrFillStyle = NULL;
	for ( int i=1; (i<=fillStyleCount) && NO_SWF_ERROR; i++ )
	{
		pNewFillStyle = new SWFFillStyle;
		pNewFillStyle->next = NULL;

		U8 fillStyleType = ReadByte( swfFile, iSuccess );
		assert(fillStyleType == 0x00 ||
				fillStyleType == 0x10 ||
				fillStyleType == 0x12 ||
				fillStyleType == 0x40 ||
				fillStyleType == 0x41 ||
				fillStyleType == 0x42 ||
				fillStyleType == 0x43);
		if ( fillStyleType & FILL_GRADIENT_LINEAR )  // 0x10
		{
			pNewFillStyle->type = (fillStyleType == FILL_GRADIENT_LINEAR) ? SWFLinearGradientFill : SWFRadialGradientFill;
			iSuccess = ReadSWFMatrix( swfFile, pNewFillStyle->gradientMatrix );
			if ( NO_SWF_ERROR )
			{
				pNewFillStyle->gradientFill.pRecord = NULL;
				iSuccess = ReadSWFGradient( swfFile, type, pNewFillStyle->gradientFill );
			}
		}
		else if ( fillStyleType & FILL_BITMAP_TILED ) // 0x40
		{
			pNewFillStyle->type = (fillStyleType == FILL_BITMAP_TILED) ? SWFTiledBitmapFill : SWFClippedBitmapFill;
			// ReadVariable( &pNewFillStyle->bitmapID , U16_SIZE, 1, swfFile, iSuccess );
			pNewFillStyle->bitmapID = ReadWord( swfFile, iSuccess );
			if ( NO_SWF_ERROR )
				iSuccess = ReadSWFMatrix( swfFile, pNewFillStyle->bitmapMatrix );
		}
		else	// It's a solid fill
		{
			pNewFillStyle->type = SWFSolidFill;
			if ( type == SWFShape3 )
				iSuccess = ReadSWFRGBA( swfFile, pNewFillStyle->shape3Color ); 
			else
				iSuccess = ReadSWFRGB( swfFile, pNewFillStyle->color );
		}

		if ( pCurrFillStyle )
			pCurrFillStyle->next = pNewFillStyle;
		else
			fillStyleArray.pStyle = pNewFillStyle;
		pCurrFillStyle = pNewFillStyle;
	}

	return iSuccess;
}


int ReadSWFLineStyleArray( SWFFile &swfFile, SWFShapeType type, SWFLineStyleArray &lineStyleArray )
{
	int iSuccess = SWF_ERR_NONE;

	U16 lineStyleCount = ReadByte( swfFile, iSuccess );
	if ( NO_SWF_ERROR && (lineStyleCount == 0xFF) )
		// ReadVariable( &lineStyleCount, U16_SIZE, 1, swfFile, iSuccess );
		lineStyleCount = ReadWord( swfFile, iSuccess );
	lineStyleArray.count = lineStyleCount;

	SWFLineStyle *pNewLineStyle = NULL;
	SWFLineStyle *pCurrLineStyle = NULL;
	for ( int i=1; (i<=lineStyleCount) && NO_SWF_ERROR; i++ )
	{
		pNewLineStyle = new SWFLineStyle;
		pNewLineStyle->next = NULL;

		// ReadVariable( &(pNewLineStyle->width), U16_SIZE, 1, swfFile, iSuccess );
		pNewLineStyle->width = ReadWord( swfFile, iSuccess );
		if ( NO_SWF_ERROR )
		{
			if ( type == SWFShape3 )
				iSuccess = ReadSWFRGBA( swfFile, pNewLineStyle->u.shape3Color );
			else
				iSuccess = ReadSWFRGB( swfFile, pNewLineStyle->u.color );
		}

		if ( pCurrLineStyle == NULL )
			lineStyleArray.pStyle = pNewLineStyle;
		else
			pCurrLineStyle->next = pNewLineStyle;
		pCurrLineStyle = pNewLineStyle;
	}

	return iSuccess;
}

/******************************************************************************
 *
 * SWF Bitmaps reading methods
 *
 *****************************************************************************/
int ReadSWFBitsLosslessTag( SWFFile &swfFile, SWFBitsLosslessTag &bitmapTag, long tagLength )
{
	int iSuccess = SWF_ERR_NONE;

	bitmapTag.id = ReadWord( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
		bitmapTag.format = ReadByte( swfFile, iSuccess );

	if ( NO_SWF_ERROR )
		bitmapTag.width = ReadWord( swfFile, iSuccess );

	if ( NO_SWF_ERROR )
		bitmapTag.height = ReadWord( swfFile, iSuccess );

	if ( NO_SWF_ERROR )
		tagLength -= 7;

	bitmapTag.tableSize = 0;
	if ( NO_SWF_ERROR && (bitmapTag.format == 3) )
	{
		bitmapTag.tableSize = ReadByte( swfFile, iSuccess );
		tagLength--;
	}

	if ( NO_SWF_ERROR )
	{
		bitmapTag.dataSize = tagLength;

		// Skip the data section...
		swfFile.Seek(tagLength);
	}

	return iSuccess;
}

int ReadSWFBitsLossless2Tag( SWFFile &swfFile, SWFBitsLosslessTag &bitmapTag, long tagLength )
{
	return ReadSWFBitsLosslessTag( swfFile, bitmapTag, tagLength );
}

/******************************************************************************
 *
 * SWF Font Code Table managing routines
 *
 *****************************************************************************/
SWFCodeTable *CreateFontCodeTable( U16 fontID, U16 size, U16 *pCodeTable )
{
	SWFCodeTable *pNewCodeTable = NULL;

	if ( (size > 0) && (pCodeTable) )
	{
		pNewCodeTable = new SWFCodeTable;
		pNewCodeTable->fontID = fontID;
		pNewCodeTable->size = size;
		pNewCodeTable->pTable = new U16[size];
		memcpy( pNewCodeTable->pTable, pCodeTable, U16_SIZE * size );
		pNewCodeTable->next = NULL;
	}

	return pNewCodeTable;
}

int AppendFontCodeTable( SWFCodeTable **hCodeTableList, SWFCodeTable *pNewCodeTable )
{
	int iSuccess = SWF_ERR_NONE;
	assert( hCodeTableList );

	if ( hCodeTableList )
	{
		if ( *hCodeTableList )
		{
			// Find the last code table in the list & append it
			SWFCodeTable *pCurrCodeTable = *hCodeTableList;
			while ( pCurrCodeTable->next )
				pCurrCodeTable = pCurrCodeTable->next;
			pCurrCodeTable->next = pNewCodeTable;
		}
		else
		{
			*hCodeTableList = pNewCodeTable;
		}
	}
	else
	{
		iSuccess = SWF_ERR_UNDEFINED;
	}

	return iSuccess;
}

const SWFCodeTable *FindFontCodeTable( U16 fontID, const SWFCodeTable *pCodeTableList )
{
	const SWFCodeTable *pTableFound = NULL;
	const SWFCodeTable *pCurrTable = pCodeTableList;
	while ( pCurrTable && !pTableFound )
	{
		if ( pCurrTable->fontID == fontID )
			pTableFound = pCurrTable;
		pCurrTable = pCurrTable->next;
	}
	
	return pTableFound;
}

int ReleaseFontCodeTableList( SWFCodeTable *pCodeTableList )
{
	SWFCodeTable *pCurrCodeTable = pCodeTableList;
	SWFCodeTable *pDelCodeTable;

	while ( pCurrCodeTable )
	{
		if ( pCurrCodeTable->pTable )
			delete[] pCurrCodeTable->pTable;

		pDelCodeTable = pCurrCodeTable;
		pCurrCodeTable = pCurrCodeTable->next;
		delete pDelCodeTable;
	}

	return SWF_ERR_NONE;
}

/******************************************************************************
 *
 * SWF Text
 *
 *****************************************************************************/
int ReadSWFFontTag( SWFFile &swfFile, SWFFont &font, long *glyphCountTable )
{
	int iSuccess = SWF_ERR_NONE;

	/* Should have been done by the caller...
	font.pOffsetTable = NULL;
	font.pShapeTable = NULL;
	*/

	font.fontID = ReadWord( swfFile, iSuccess );
	int iStartPos = swfFile.GetPosition();
	int iOffset = 0;

	if ( NO_SWF_ERROR )
		iOffset = ReadWord( swfFile, iSuccess );

	if ( NO_SWF_ERROR )
	{
		font.numGlyphs = iOffset / 2;
		assert( font.numGlyphs > 0 );
		glyphCountTable[font.fontID] = font.numGlyphs;
		font.pOffsetTable = new short[font.numGlyphs];
		font.pOffsetTable[0] = iOffset;
		font.pShapeTable = new SWFShapeWithStyle[font.numGlyphs];

		for ( int i=1; (i<font.numGlyphs) && NO_SWF_ERROR; i++ )
			font.pOffsetTable[i] = ReadWord( swfFile, iSuccess );
	}

	if ( NO_SWF_ERROR )
	{
		for ( int i=0; (i<font.numGlyphs) && NO_SWF_ERROR; i++ )
		{
			iSuccess = swfFile.SetPosition( font.pOffsetTable[i] + iStartPos );
			if ( NO_SWF_ERROR )
				iSuccess = ReadSWFShape( swfFile, font.pShapeTable[i] );
		}
	}

	// go to the end of this font tag...
	swfFile.SeekToTagLimit();

	return iSuccess;
}

int ReadSWFSetTabIndex( SWFFile &swfFile, int& depth, int& index )
{
	int iSuccess = SWF_ERR_NONE;
	depth = ReadWord( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
	{
		index = ReadWord( swfFile, iSuccess );
	}
	return iSuccess;
}

int ReadSWFExportAssets( SWFFile &swfFile, int &count, SWFExportAssets **exports )
{
	int iSuccess = SWF_ERR_NONE;
	count = ReadWord( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
	{
		*exports = new SWFExportAssets[count];
		for (int i = 0; i < count; ++i)
		{
			(*exports)[i].characterID = ReadWord( swfFile, iSuccess );
			if ( !(NO_SWF_ERROR) )
				break;

			char *szFileBuffer = swfFile.GetBufferAtCurrentPos();
			U32 valueLen = strlen(szFileBuffer) + 1;
			strncpy( (*exports)[i].name, szFileBuffer, sizeof((*exports)[i].name)-1 );
			swfFile.Seek( valueLen );
		}
	}
	return iSuccess;
}

int ReadSWFFont2Tag( SWFFile &swfFile, SWFFont2 &font, SWFCodeTable **hCodeTableList )
{
	int iSuccess = SWF_ERR_NONE;

	/* Should have been done by the caller...
	font.szFontName = NULL;
	font.pFontOffsetTable = NULL;
	font.pShapeTable = NULL;
	font.pCodeTable = NULL;
	font.pAdvanceTable = NULL;
	font.pBoundsTable = NULL;
	font.pKerningTable = NULL;
	*/

	font.fontID = ReadWord( swfFile, iSuccess );
	// First 8 bits are flags, next 8 bits is the lang code if <= SWF5, or zero if >= SWF6
	U8 flags = 0;
	if ( NO_SWF_ERROR )
		flags = ReadByte( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
		font.languageCode = ReadByte( swfFile, iSuccess );

	// decode the flags...
	if ( NO_SWF_ERROR )
	{
		font.bHasLayout = (flags & DF2_HASLAYOUT) != 0;
		font.bShiftJIS = (flags & DF2_SHIFJIS) != 0;
		font.bANSI = (flags & DF2_ANSI) != 0;
		font.bWideOffsets = (flags & DF2_WIDEOFFSETS) != 0;
		font.bWideCodes = (flags & DF2_WIDECODES) != 0;
		font.bItalic = (flags & DF2_ITALIC) != 0;
		font.bBold = (flags & DF2_BOLD) != 0;
	}

	U8 nameLen = 0;
	if ( NO_SWF_ERROR )
		nameLen = ReadByte( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
	{
		font.szFontName = new U8[nameLen+1];
		ReadVariable( font.szFontName, U8_SIZE, nameLen, swfFile, iSuccess );
		font.szFontName[nameLen] = 0;
	}
	if ( NO_SWF_ERROR )
		font.numGlyphs = ReadWord( swfFile, iSuccess );

	int iStartPos = swfFile.GetPosition();
	if ( NO_SWF_ERROR && (font.numGlyphs > 0) )
	{
		// Get Font Offset Table		
		font.pFontOffsetTable = new U32[font.numGlyphs];
		for ( int i=0; (i<font.numGlyphs) && NO_SWF_ERROR; i++ )
			if ( font.bWideOffsets )
				font.pFontOffsetTable[i] = ReadDoubleWord( swfFile, iSuccess );
			else
				font.pFontOffsetTable[i] = ReadWord( swfFile, iSuccess );
	}

	// Get Code Offset Table
	if ( NO_SWF_ERROR )
	{
		if ( font.bWideOffsets )
			font.codeTableOffset = ReadDoubleWord( swfFile, iSuccess );
		else
			font.codeTableOffset = ReadWord( swfFile, iSuccess );
	}

	// Get the Glyphs
	if ( NO_SWF_ERROR && (font.numGlyphs > 0) )
	{
		font.pShapeTable = new SWFShapeWithStyle[font.numGlyphs];
		font.pShapeTable->pRecordList = NULL;
		for ( int i=0; (i<font.numGlyphs) && NO_SWF_ERROR; i++ )
		{
			iSuccess = swfFile.SetPosition( font.pFontOffsetTable[i] + iStartPos );
			if ( NO_SWF_ERROR )
				iSuccess = ReadSWFShape( swfFile, font.pShapeTable[i] );
		}
	}

	// Get the Code Table
	if ( NO_SWF_ERROR )
		iSuccess = swfFile.SetPosition( font.codeTableOffset + iStartPos );
	if ( NO_SWF_ERROR && (font.numGlyphs > 0) )
	{
		font.pCodeTable = new U16[font.numGlyphs];
		for ( int i=0; (i<font.numGlyphs) && NO_SWF_ERROR; i++ )
		{
			if ( font.bWideCodes )
				font.pCodeTable[i] = ReadWord( swfFile, iSuccess );
			else
				font.pCodeTable[i] = ReadByte( swfFile, iSuccess );
		}

		// Store the code table if we get a handle to the font code table list...
		if ( NO_SWF_ERROR && hCodeTableList )
		{
			SWFCodeTable *pNewCodeTable = CreateFontCodeTable( font.fontID, font.numGlyphs, font.pCodeTable );
			iSuccess = AppendFontCodeTable( hCodeTableList, pNewCodeTable );
		}
	}

	// Get Layout fields
	if ( NO_SWF_ERROR && font.bHasLayout )
	{
		// Get the ascent, descent, and leading values
		font.ascent = ReadSWord( swfFile, iSuccess );
		if ( NO_SWF_ERROR )
			font.descent = ReadSWord( swfFile, iSuccess );
		if ( NO_SWF_ERROR )
			font.leading = ReadSWord( swfFile, iSuccess );

		// Get the advance and bounds table
		if ( NO_SWF_ERROR )
		{
			font.pAdvanceTable = new S16[font.numGlyphs];
			font.pBoundsTable = new SWFRect[font.numGlyphs];
			int i;
			for ( i=0; (i<font.numGlyphs) && NO_SWF_ERROR; i++ )
				font.pAdvanceTable[i] = ReadSWord( swfFile, iSuccess );
			for ( i=0; (i<font.numGlyphs) && NO_SWF_ERROR; i++ )
				iSuccess = ReadSWFRect( swfFile, font.pBoundsTable[i] );											
		}

		// Get the kerning table size
		if ( NO_SWF_ERROR )
			font.kerningCount = ReadWord( swfFile, iSuccess );

		// Get the kerning pairs
		if ( NO_SWF_ERROR )
		{
			SWFKerningRecord *pNewKerningRec = NULL;
			SWFKerningRecord *pCurrKerningRec = NULL;
			font.pKerningTable = new SWFKerningRecord[font.kerningCount];
			for ( int i=0; (i<font.kerningCount) && NO_SWF_ERROR; i++ )
			{
				if ( font.bWideCodes )
				{
					font.pKerningTable[i].code1 = ReadWord( swfFile, iSuccess );
					if ( NO_SWF_ERROR )
						font.pKerningTable[i].code2 = ReadWord( swfFile, iSuccess );
				}
				else
				{
					font.pKerningTable[i].code1 = ReadByte( swfFile, iSuccess );
					if ( NO_SWF_ERROR )
						font.pKerningTable[i].code2 = ReadByte( swfFile, iSuccess );
				}

				if ( NO_SWF_ERROR )
					font.pKerningTable[i].adjustment = ReadSWord( swfFile, iSuccess );
			}
		}
	} // end of font.bHasLayout

	return iSuccess;
}

int ReadSWFTextTag( SWFFile &swfFile, SWFText &text, bool bHasAlpha )
{
	int iSuccess = SWF_ERR_NONE;
	
	text.characterID = ReadWord( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
		iSuccess = ReadSWFRect( swfFile, text.bound );
	if ( NO_SWF_ERROR )
		iSuccess = ReadSWFMatrix( swfFile, text.matrix );
	if ( NO_SWF_ERROR )
		text.numGlyphBits = ReadByte( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
		text.advanceGlyphBits = ReadByte( swfFile, iSuccess );

	// Read the text records...
	if ( NO_SWF_ERROR ) 
	{
		text.pRecordList = NULL;
		SWFTextRecordList *pNewGlyphRecList = NULL;
		SWFTextRecordList *pCurrGlyphRecList = text.pRecordList;

		SWFTextStyleChangeRecord *pNewSCRec = NULL;
		SWFGlyphRecord *pNewGlyphRec = NULL;

		U8 flags = 0;
		while( ((flags = ReadByte(swfFile, iSuccess)) != 0) && NO_SWF_ERROR )
		{
			pNewGlyphRecList = new SWFTextRecordList;
			pNewGlyphRecList->next = NULL;
			pNewGlyphRecList->pRecord = NULL;

			if ( flags >> 7 ) // It's a text style change record
			{
				pNewGlyphRecList->type = SWFTextStyleChange;

				pNewSCRec = new SWFTextStyleChangeRecord;
				pNewSCRec->bHasFont = (flags & TSC_HASFONT) != 0;
				pNewSCRec->bHasColor = (flags & TSC_HASCOLOR) != 0;
				pNewSCRec->bHasYOffset = (flags & TSC_HASYOFFSET) != 0;
				pNewSCRec->bHasXOffset = (flags & TSC_HASXOFFSET) != 0;

				if ( pNewSCRec->bHasFont )
					// ReadVariable( &pNewSCRec->fontID, U16_SIZE, 1, swfFile, iSuccess );
					pNewSCRec->fontID = ReadWord( swfFile, iSuccess );

				if ( NO_SWF_ERROR && pNewSCRec->bHasColor )
				{
					if ( bHasAlpha )
						iSuccess = ReadSWFRGBA( swfFile, pNewSCRec->u.text2Color );
					else
						iSuccess = ReadSWFRGB( swfFile, pNewSCRec->u.textColor );
				}

				if ( NO_SWF_ERROR && pNewSCRec->bHasXOffset )
					// ReadVariable( &pNewSCRec->xOffset, S16_SIZE, 1, swfFile, iSuccess );
					pNewSCRec->xOffset = ReadSWord( swfFile, iSuccess );

				if ( NO_SWF_ERROR && pNewSCRec->bHasYOffset )
					// ReadVariable( &pNewSCRec->yOffset, S16_SIZE, 1, swfFile, iSuccess );
					pNewSCRec->yOffset = ReadSWord( swfFile, iSuccess );

				if ( NO_SWF_ERROR && pNewSCRec->bHasFont )
					// ReadVariable( &pNewSCRec->textHeight, U16_SIZE, 1, swfFile, iSuccess );
					pNewSCRec->textHeight = ReadWord( swfFile, iSuccess );

				if ( NO_SWF_ERROR )
					pNewGlyphRecList->pRecord = pNewSCRec;
				else
					delete pNewSCRec;
			}
			else // It's a glyph record
			{
				pNewGlyphRecList->type = SWFGlyph;

				pNewGlyphRec = new SWFGlyphRecord;
				pNewGlyphRec->count = flags;
				pNewGlyphRec->pEntries = NULL;
				SWFGlyphEntry *pNewGlyphEntry = NULL;
				SWFGlyphEntry *pCurGlyphEntry = pNewGlyphRec->pEntries;

				S32 bitPos = 0;
				U8 bitBuffer = 0;
				for ( int i=0; (i<pNewGlyphRec->count) && NO_SWF_ERROR; i++ )
				{
					pNewGlyphEntry = new SWFGlyphEntry;
					pNewGlyphEntry->index = ReadBits( swfFile, text.numGlyphBits, bitBuffer, bitPos );
					pNewGlyphEntry->advance = ReadBits( swfFile, text.advanceGlyphBits, bitBuffer, bitPos );
					pNewGlyphEntry->next = NULL;

					if ( !pCurGlyphEntry )
						pNewGlyphRec->pEntries = pNewGlyphEntry;
					else
						pCurGlyphEntry->next = pNewGlyphEntry;
					pCurGlyphEntry = pNewGlyphEntry;
				}

				if ( NO_SWF_ERROR )
					pNewGlyphRecList->pRecord = pNewGlyphRec;
				else
					delete pNewGlyphRec; // no need to clean up glyph entries because it can't error out in there yet...
			}

			if ( !pCurrGlyphRecList )
				text.pRecordList = pNewGlyphRecList;
			else
				pCurrGlyphRecList->next = pNewGlyphRecList;
			pCurrGlyphRecList = pNewGlyphRecList;
		}

	}

	return iSuccess;
}

int ReadSWFEditTextTag( SWFFile &swfFile, SWFEditText &text )
{
	int iSuccess = SWF_ERR_NONE;

	text.characterID = ReadWord( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
		iSuccess = ReadSWFRect( swfFile, text.bound );

	// Read the flags
	U8 flags = 0;
	if ( NO_SWF_ERROR )
		flags = ReadByte( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
	{
		text.hasText		= (flags & ET_HASTEXT) != 0;
		text.wordWrap		= (flags & ET_WORDWRAP) != 0;
		text.multiLine		= (flags & ET_MULTILINE) != 0;
		text.password		= (flags & ET_PASSWORD) != 0;
		text.readOnly		= (flags & ET_READONLY) != 0;
		text.hasTextColor	= (flags & ET_HASTEXTCOLOR) != 0;
		text.hasMaxLength	= (flags & ET_HASMAXLENGTH) != 0;
		text.hasFont		= (flags & ET_HASFONT) != 0;
		flags = ReadByte( swfFile, iSuccess );
	}
	if ( NO_SWF_ERROR )
	{
		text.autoSize		= (flags & ET_AUTOSIZE) != 0;
		text.hasLayout		= (flags & ET_HASLAYOUT) != 0;
		text.noSelect		= (flags & ET_NOSELECT) != 0;
		text.border			= (flags & ET_BORDER) != 0;
		text.html			= (flags & ET_HTML) != 0;
		text.useOutline		= (flags & ET_USEOUTLINE) != 0;
	}

	if ( NO_SWF_ERROR && text.hasFont )
	{
		text.fontID = ReadWord( swfFile, iSuccess );
		if ( NO_SWF_ERROR )
			text.fontHeight = ReadWord( swfFile, iSuccess );
	}

	if ( NO_SWF_ERROR && text.hasTextColor )
		iSuccess = ReadSWFRGBA( swfFile, text.textColor );

	if ( NO_SWF_ERROR && text.hasMaxLength )
		text.maxLength = ReadWord( swfFile, iSuccess );

	if ( NO_SWF_ERROR && text.hasLayout )
	{
		text.align = ReadByte( swfFile, iSuccess );
		if ( NO_SWF_ERROR )
			text.leftMargin = ReadWord( swfFile, iSuccess );
		if ( NO_SWF_ERROR )
			text.rightMargin = ReadWord( swfFile, iSuccess );
		if ( NO_SWF_ERROR )
			text.indent = ReadWord( swfFile, iSuccess );
		if ( NO_SWF_ERROR )
			text.leading = ReadWord( swfFile, iSuccess );
	}

	if ( NO_SWF_ERROR )
	{
		char *szVariableBuffer = swfFile.GetBufferAtCurrentPos();
		U32 varLen = strlen(szVariableBuffer) + 1;
		text.szVariableName = new char[varLen];
		strcpy( text.szVariableName, szVariableBuffer );
		swfFile.Seek( varLen );
	}

	if ( NO_SWF_ERROR && text.hasText )
	{
		char *szTextBuffer = swfFile.GetBufferAtCurrentPos();
		U32 textLen = strlen(szTextBuffer) + 1;
		text.szInitialText = new char[textLen];
		strcpy( text.szInitialText, szTextBuffer );
		swfFile.Seek( textLen );
	}

	return iSuccess;
}

int ReadSWFFontInfoHeader( SWFFile &swfFile, SWFFontInfoHeader &fontInfoHeader )
{
	int iSuccess = SWF_ERR_NONE;

	if ( NO_SWF_ERROR )
		fontInfoHeader.fontID = ReadWord( swfFile, iSuccess );

	U8 nameLen = 0;
	if ( NO_SWF_ERROR )
		nameLen = ReadByte( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
	{
		fontInfoHeader.szFontName = new U8[nameLen+1];
		ReadVariable( fontInfoHeader.szFontName, U8_SIZE, nameLen, swfFile, iSuccess );
		fontInfoHeader.szFontName[nameLen] = 0;
	}

	U8 flags = 0;
	if ( NO_SWF_ERROR )
		flags = ReadByte( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
	{
		fontInfoHeader.bShiftJIS = (flags & FIH_SHIFJIS) != 0;
		fontInfoHeader.bANSI = (flags & FIH_ANSI) != 0;
		fontInfoHeader.bItalic = (flags & FIH_ITALIC) != 0;
		fontInfoHeader.bBold = (flags & FIH_BOLD) != 0;
		fontInfoHeader.bWideCodes = (flags & FIH_WIDECODES) != 0;
	}

	return iSuccess;
}

int ReadSWFFontInfoTag( SWFFile &swfFile, SWFFontInfo &fontInfo, long *glyphCountTable, SWFCodeTable **hCodeTableList )
{
	int iSuccess = glyphCountTable ? SWF_ERR_NONE : SWF_ERR_UNDEFINED;
	
	if ( NO_SWF_ERROR )
		iSuccess = ReadSWFFontInfoHeader( swfFile, fontInfo.header );

	// Read in the code table
	if ( NO_SWF_ERROR )
	{
		int numGlyphs = glyphCountTable[fontInfo.header.fontID];
		fontInfo.pCodeTable = new U16[numGlyphs];
		for ( int i=0; (i<numGlyphs) && NO_SWF_ERROR; i++ )
		{
			if ( fontInfo.header.bWideCodes )
				fontInfo.pCodeTable[i] = ReadWord( swfFile, iSuccess );
			else
				fontInfo.pCodeTable[i] = ReadByte( swfFile, iSuccess );
		}

		// Store the code table if we get a handle to the font code table list...
		if ( NO_SWF_ERROR && hCodeTableList )
		{
			SWFCodeTable *pNewCodeTable = CreateFontCodeTable( fontInfo.header.fontID, numGlyphs, fontInfo.pCodeTable );
			iSuccess = AppendFontCodeTable( hCodeTableList, pNewCodeTable );
		}
	}

	return iSuccess;
}

int ReadSWFFontInfo2Tag( SWFFile &swfFile, SWFFontInfo2 &fontInfo, long *glyphCountTable, SWFCodeTable **hCodeTableList )
{
	int iSuccess = glyphCountTable ? SWF_ERR_NONE : SWF_ERR_UNDEFINED;

	if ( NO_SWF_ERROR )
		iSuccess = ReadSWFFontInfoHeader( swfFile, fontInfo.header );

	if ( NO_SWF_ERROR )
		fontInfo.languageCode = ReadByte( swfFile, iSuccess );

	// Read in the code table
	if ( NO_SWF_ERROR )
	{
		int numGlyphs = glyphCountTable[fontInfo.header.fontID];
		fontInfo.pCodeTable = new U16[numGlyphs];
		for ( int i=0; (i<numGlyphs) && NO_SWF_ERROR; i++ )
		{
			fontInfo.pCodeTable[i] = ReadWord( swfFile, iSuccess );
		}

		// Store the code table if we get a handle to the font code table list...
		if ( NO_SWF_ERROR && hCodeTableList )
		{
			SWFCodeTable *pNewCodeTable = CreateFontCodeTable( fontInfo.header.fontID, numGlyphs, fontInfo.pCodeTable );
			iSuccess = AppendFontCodeTable( hCodeTableList, pNewCodeTable );
		}
	}

	return iSuccess;
}

/******************************************************************************
 *
 * SWF Sound
 *
 *****************************************************************************/
int ReadSWFSoundStreamHeader( SWFFile &swfFile, SWFSoundStreamHead &head )
{
	int iSuccess = SWF_ERR_NONE;

	S32 bitPos = 0;
	U8 bitBuffer = 0;
	U32 iBits = ReadBits( swfFile, 4, bitBuffer, bitPos );
	head.playbackRate = static_cast<SWFSoundRate>(ReadBits( swfFile, 2, bitBuffer, bitPos ));
	head.playbackSize = static_cast<SWFSoundSize>(ReadBits( swfFile, 1, bitBuffer, bitPos ));
	head.playbackType = static_cast<SWFSoundType>(ReadBits( swfFile, 1, bitBuffer, bitPos ));
	head.compressionType = static_cast<SWFSoundCompression>(ReadBits( swfFile, 4, bitBuffer, bitPos ));
	head.streamRate = static_cast<SWFSoundRate>(ReadBits( swfFile, 2, bitBuffer, bitPos ));
	head.streamSize = static_cast<SWFSoundSize>(ReadBits( swfFile, 1, bitBuffer, bitPos ));
	head.streamType = static_cast<SWFSoundType>(ReadBits( swfFile, 1, bitBuffer, bitPos ));

	/*
	U8 data = ReadByte( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
	{
		data &= 0x0F;	// First four bits are reserved, but just in case
		head.playbackType = (SWFSoundType) data & 0x01;
		data = data >> 1;
		head.playbackSize = (SWFSoundSize) data & 0x01;
		data = data >> 1;
		head.playbackRate = (SWFSoundRate) data;

		data = ReadByte( swfFile, iSuccess );
	}

	if ( NO_SWF_ERROR )
	{
		head.streamType = (SWFSoundType) data & 0x01;
		data = data >> 1;
		head.streamSize = (SWFSoundSize) data & 0x01;
		data = data >> 1;
		head.streamRate = (SWFSoundRate) data & 0x03;
		data = data >> 2;
		head.compressionType = (SWFSoundCompression) data & 0x0F;
	}
	*/

	if ( NO_SWF_ERROR )
		head.streamSampleCount = ReadWord( swfFile, iSuccess );

	if ( NO_SWF_ERROR )
	{
		if (head.compressionType == SWFSndCompress_MP3)
		{
			head.latencySeek = ReadSWord( swfFile, iSuccess );
		}
		else
		{
			head.latencySeek = 0;
		}
	}

	return iSuccess;

}

int ReadSWFMP3StreamBlock( SWFFile &swfFile, SWFMP3StreamBlock &block )
{
	int iSuccess = SWF_ERR_NONE;
	block.sampleCount = ReadWord( swfFile, iSuccess);
	return iSuccess;
}

/******************************************************************************
 *
 * SWF Display List
 *
 *****************************************************************************/
int ReadSWFSpriteTag( SWFFile &swfFile, SWFSpriteTag &sprite )
{
	int iSuccess = SWF_ERR_NONE;

	sprite.id = ReadWord( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
		sprite.frameCount = ReadWord( swfFile, iSuccess );

	return iSuccess;
}

int ReadSWFPlaceObjectTag( SWFFile &swfFile, SWFPlaceObjectTag &place )
{
	int iSuccess = SWF_ERR_NONE;

	place.characterID = ReadWord( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
		place.depth = ReadWord( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
		iSuccess = ReadSWFMatrix( swfFile, place.matrix );
	if ( NO_SWF_ERROR && !swfFile.TagLimitReached() )
	{
		place.bHasColorXform = true;
		iSuccess = ReadSWFColorXForm( swfFile, place.colorXform );
	}

	return iSuccess;
}

int ReadSWFPlaceObject2Tag( SWFFile &swfFile, SWFPlaceObject2Tag &place )
{
	int iSuccess = SWF_ERR_NONE;

	U8 flags = ReadByte( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
	{
		place.bHasClipActions	= (flags & PO2_CLIPACTIONS) != 0;
		place.bHasClipDepth		= (flags & PO2_CLIPDEPTH) != 0;
		place.bHasName			= (flags & PO2_NAME) != 0;
		place.bHasRatio			= (flags & PO2_RATIO) != 0;
		place.bHasColorXform	= (flags & PO2_COLORXFORM) != 0;
		place.bHasMatrix		= (flags & PO2_MATRIX) != 0;
		place.bHasCharacter		= (flags & PO2_CHARACTER) != 0;
		place.bMove				= (flags & PO2_MOVE) != 0;

		place.depth = ReadWord( swfFile, iSuccess );
	}

	if ( NO_SWF_ERROR && place.bHasCharacter )
		place.characterID = ReadWord( swfFile, iSuccess );

	if ( NO_SWF_ERROR && place.bHasMatrix )
		iSuccess = ReadSWFMatrix( swfFile, place.matrix );

	if ( NO_SWF_ERROR && place.bHasColorXform )
	{
		place.colorXform.bHasAlpha = true;
		iSuccess = ReadSWFColorXForm( swfFile, place.colorXform );
	}

	if ( NO_SWF_ERROR && place.bHasRatio )
		place.ratio = ReadWord( swfFile, iSuccess );

	if ( NO_SWF_ERROR && place.bHasName )
	{
		char szTmpBuffer[512];
		int i = 0;
		U8 c = 0;
		while ( NO_SWF_ERROR && (c=ReadByte(swfFile, iSuccess)) && (i<511) )
		{
			szTmpBuffer[i++] = c;
		}
		szTmpBuffer[i] = 0;

		place.szName = new char[strlen(szTmpBuffer)+1];
		strcpy( place.szName, szTmpBuffer );
	}

	if ( NO_SWF_ERROR && place.bHasClipDepth )
		place.clipDepth = ReadWord( swfFile, iSuccess );

	if ( NO_SWF_ERROR && place.bHasClipActions )
	{
		// skip the clip actions for now...
		swfFile.SeekToTagLimit();
	}

	return iSuccess;
}

int ReadSWFRemoveObjectTag( SWFFile &swfFile, SWFRemoveObjectTag &remove )
{
	int iSuccess = SWF_ERR_NONE;
	remove.characterID = ReadWord( swfFile, iSuccess );
	if ( NO_SWF_ERROR )
		remove.depth = ReadWord( swfFile, iSuccess );
	return iSuccess;
}

int ReadSWFRemoveObject2Tag( SWFFile &swfFile, SWFRemoveObject2Tag &remove )
{
	int iSuccess = SWF_ERR_NONE;
	remove.depth = ReadWord( swfFile, iSuccess );
	return iSuccess;
}

/******************************************************************************
 *
 * SWF ActionScript
 *
 *****************************************************************************/
static int ReadSWFActionRecord(SWFFile& swfFile, SWFActionRecord& newRec, long& length, const SWFActionRecord* pCurCP)
{
	length = 0;

	int iSuccess = SWF_ERR_NONE;

	bool handled = false;
	newRec.code = ReadByte(swfFile, iSuccess);
	if ( NO_SWF_ERROR && newRec.code )
	{
		if ( newRec.code & 0x80 )
			length = ReadWord( swfFile, iSuccess );

		long endOfAction = swfFile.GetPosition() + length;

		switch (newRec.code)
		{
			case SWFACTION_DEFFUNC:
			case SWFACTION_DEFFUNC2:
			{
				newRec.description = ReadString(swfFile, iSuccess);
				if (newRec.description.empty())
				{
					newRec.description = "<Anon>";
				}

				newRec.description += "(";
				U16 numParms = ReadWord(swfFile, iSuccess);
				U8 numRegs = 0;
				U16 flags = 0;
				if (newRec.code == SWFACTION_DEFFUNC2)
				{
					numRegs = ReadByte(swfFile, iSuccess);
					flags = ReadWord(swfFile, iSuccess);
				}	
				for (U16 i = 0; i < numParms; ++i)
				{
					if (i > 0)
						newRec.description += ",";
					if (newRec.code == SWFACTION_DEFFUNC2)
					{
						U8 theReg = ReadByte(swfFile, iSuccess);
					}
					newRec.description += ReadString(swfFile, iSuccess);
				}
				newRec.description += ")";

				U16 codeSize = ReadWord(swfFile, iSuccess);

				newRec.description += " (";
				newRec.description += TString(codeSize);
				newRec.description += " bytes)";

// just skip over the opcodes that define this function.
// someday, put in the treeview under this entry...
				if (codeSize > 0)
				{
					endOfAction += codeSize;	// yes, to make it work out
					
					long subStartOfCode = swfFile.GetPosition();
					SWFActionRecord curSubCP;
					for (;;)
					{
						SWFActionRecord newSubRec;
						long subLength = 0;
						iSuccess = ReadSWFActionRecord(swfFile, newSubRec, subLength, pCurCP);
						if (!(NO_SWF_ERROR))
							break;

						newRec.actions.push_back(newSubRec);
						
						if (newSubRec.code == 0)
						{
							assert(0);	// shouldn't ever see it here
							break;
						}

						long curSubPos = swfFile.GetPosition();
						if (curSubPos >= subStartOfCode + codeSize)
						{
							assert(curSubPos == subStartOfCode + codeSize);
							break;
						}

						if (newSubRec.code == SWFACTION_CONSTPOOL)
						{
							curSubCP = newSubRec;
							pCurCP = &curSubCP;
						}

					};
				}
				
				handled = true;
			}
			break;
			case SWFACTION_STOREREG:
			{
				U8 reg = ReadByte(swfFile, iSuccess);
				newRec.description = TString(reg);
				newRec.description += " <register>";
				handled = true;
			}
			break;
			case SWFACTION_CONSTPOOL:
			{
				U16 count = ReadWord( swfFile, iSuccess );
				for (U16 i = 0; i < count; ++i)
				{
					std::string newStr = ReadString(swfFile, iSuccess);
					newRec.theStrings.push_back(newStr);
				}
				handled = true;
			}
			break;

			case SWFACTION_GETURL2:
			{
				U8 flags = ReadByte( swfFile, iSuccess );

				const char* sendVarsMethod = "";
				if (flags & 0x01)
					sendVarsMethod = "POST";
				else if (flags & 0x02)
					sendVarsMethod = "GET";
				else
					sendVarsMethod = "NONE";

				const char* loadTarget = "";
				if (flags & 0x40)
					loadTarget = "SPRITE";
				else
					loadTarget = "WINDOW";

				const char* loadVars = "";
				if (flags & 0x80)
					loadVars = "YES";
				else
					loadVars = "NO";

				char szTmpBuffer[1024];
				sprintf(szTmpBuffer, "SendVars=%s Target=%s LoadVars=%s",sendVarsMethod,loadTarget,loadVars);
				newRec.description = szTmpBuffer;

				handled = true;
			}
			break;
		
			case SWFACTION_PUSH:
			{
				char szTmpBuffer[1024];
				long remaining = endOfAction - swfFile.GetPosition();
				std::string tmpDesc;
				while (remaining > 0)
				{
					U8 dataType = ReadByte( swfFile, iSuccess );
					remaining -= 1;
					switch ( dataType )
					{
						case 0:		// String
						{
							tmpDesc = ReadString(swfFile, iSuccess);
							{
								std::string tmp2 = "\"";
								tmp2 += tmpDesc;
								tmp2 += "\" <string>";
								newRec.theStrings.push_back(tmp2);
							}
							remaining -= tmpDesc.size() + 1;	// must include the null terminator
						}
						break;

						case 1:		// Float
							U32 tmp;
							tmp = ReadDoubleWord( swfFile, iSuccess );
							sprintf(szTmpBuffer, "%f <float>",*(float*)&tmp);
							tmpDesc = szTmpBuffer;
							newRec.theStrings.push_back(tmpDesc);
							remaining -= 4;
							break;
						case 2:		// Null
							tmpDesc = "null <null>";
							newRec.theStrings.push_back(tmpDesc);
							//remaining -= 0;
							break;
						case 3:		// Undefined
							tmpDesc = "undefined <undefined>";
							newRec.theStrings.push_back(tmpDesc);
							//remaining -= 0;
							break;
						case 4:		// Register
						{
							U8 reg = ReadByte( swfFile, iSuccess );
							sprintf(szTmpBuffer, "%d <register>",reg);
							tmpDesc = szTmpBuffer;
							newRec.theStrings.push_back(tmpDesc);
							remaining -= 1;
							break;
						}
						case 5:		// Boolean
						{
							U8 b = ReadByte( swfFile, iSuccess );
							tmpDesc = b ? "true <boolean>" : "false <boolean>";
							newRec.theStrings.push_back(tmpDesc);
							remaining -= 1;
							break;
						}
						case 6:		// Double
						{
							__int64 d0 = ReadDoubleWord( swfFile, iSuccess );
							__int64 d1 = ReadDoubleWord( swfFile, iSuccess );
							double d = (double) ((d1 << 32) | d0);
							sprintf(szTmpBuffer, "%f <double>",d);
							tmpDesc = szTmpBuffer;
							newRec.theStrings.push_back(tmpDesc);
							remaining -= 8;
							break;
						}
						case 7:		// Integer
						{
							U32 i = ReadDoubleWord( swfFile, iSuccess );
							sprintf(szTmpBuffer, "%d <integer>",i);
							tmpDesc = szTmpBuffer;
							newRec.theStrings.push_back(tmpDesc);
							remaining -= 4;
							break;
						}
						case 8:		// Constant Byte
						{
							U8 idx = ReadByte( swfFile, iSuccess );
							if (pCurCP && idx < pCurCP->theStrings.size())
							{
								sprintf(szTmpBuffer, "\"%s\" <constant8 %d>",pCurCP->theStrings[idx].c_str(),idx);
								tmpDesc = szTmpBuffer;
								newRec.theStrings.push_back(tmpDesc);
							}
							else
							{
								assert(0);
								tmpDesc = "BAD constant pool string (byte)";
								newRec.theStrings.push_back(tmpDesc);
							}
							remaining -= 1;
							break;
						}
						case 9:		// Constant Word
						{
							U16 idx = ReadWord( swfFile, iSuccess );
							if (pCurCP && idx < pCurCP->theStrings.size())
							{
								sprintf(szTmpBuffer, "\"%s\" <constant16 %d>",pCurCP->theStrings[idx].c_str(),idx);
								tmpDesc = szTmpBuffer;
								newRec.theStrings.push_back(tmpDesc);
							}
							else
							{
								assert(0);
								tmpDesc = "BAD constant pool string (word)";
								newRec.theStrings.push_back(tmpDesc);
							}
							remaining -= 2;
							break;
						}
						default:
							break;
					}	// switch pushtype
				} // while
				handled = true;
			}
			break;
		} //switch code

		// Our protection against bad files usually Constant Bytes are odd
		// The SWF spec doesn't describe enough about it :(
		long curPos = swfFile.GetPosition();
		if ( curPos != endOfAction )
		{
			assert(!handled);
			swfFile.SetPosition( endOfAction );
		}
	}

	return iSuccess;
}

/******************************************************************************
 *
 * SWF ActionScript
 *
 *****************************************************************************/
int ReadSWFActionTag(SWFFile& swfFile, std::vector<SWFActionRecord>& actions, long* spriteID, long& constPoolTotal)
{
	constPoolTotal = 0;
	
	int curCpIdx = -1;
	int iSuccess = SWF_ERR_NONE;

	if (spriteID)
	{
		*spriteID = ReadWord( swfFile, iSuccess );
		if (!(NO_SWF_ERROR))
		{
			return iSuccess;
		}
	}

	actions.clear();
	for (;;)
	{
		const SWFActionRecord* pCurCP = NULL;
		if (curCpIdx >= 0 && curCpIdx < (long)actions[curCpIdx].theStrings.size())
		{
			pCurCP = &actions[curCpIdx];
		}

		SWFActionRecord newRec;
		long length = 0;
		iSuccess = ReadSWFActionRecord(swfFile, newRec, length, pCurCP);
		if (!(NO_SWF_ERROR))
			break;

		actions.push_back(newRec);
		
		if (newRec.code == 0)
			break;
		
		if (newRec.code == SWFACTION_CONSTPOOL)
		{
			constPoolTotal += length;
			curCpIdx = actions.size()-1;
		}
	};

	assert( swfFile.TagLimitReached() );

	return iSuccess;
}

/******************************************************************************
 *
 * SWF Header & Tags
 *
 *****************************************************************************/
int ReadSWFHeader( SWFFile &swfFile, SWFHeader &fileHeader )
{
	int iSuccess = swfFile.LoadHeader( fileHeader );

	// Get the Frame RECT
	if ( NO_SWF_ERROR )
		iSuccess = ReadSWFRect( swfFile, fileHeader.rFrameSize );

	if ( NO_SWF_ERROR )
	{
		// Continue reading the frame rate and frame count...
		fileHeader.iFrameRateFraction = ReadByte( swfFile, iSuccess );
		fileHeader.iFrameRate = ReadByte( swfFile, iSuccess );
	}

	// Get the frame count
	U16 usFrameCount = ReadWord( swfFile, iSuccess );
	// ReadVariable( &usFrameCount, U16_SIZE, 1, swfFile, iSuccess );
	if ( NO_SWF_ERROR )
		fileHeader.iFrameCount = usFrameCount;

	return iSuccess;
}

int ReadSWFTagHeader( SWFFile &swfFile, SWFTagHeader &tagHeader, short *pBytesRead /* = NULL */ )
{
	int iSuccess = SWF_ERR_NONE;

	long tagOffset = swfFile.GetPosition();

	U16 usTagCodeAndLen;
	ReadVariable( &usTagCodeAndLen, U16_SIZE, 1, swfFile, iSuccess );
	if ( pBytesRead )
		(*pBytesRead) += 2;

	if ( NO_SWF_ERROR )
	{
		int tagType = usTagCodeAndLen >> 6;
		int tagLength = usTagCodeAndLen & 0x003F;

		if ( tagLength == 0x3F )	// Long tag type
		{
			U32 ulTagLength;
			ReadVariable( &ulTagLength, U32_SIZE, 1, swfFile, iSuccess );
			if ( pBytesRead )
				(*pBytesRead) += 4;

			if ( NO_SWF_ERROR )
				tagLength = ulTagLength;
		}

		tagHeader.type = tagType;
		tagHeader.offset = tagOffset;
		tagHeader.length = tagLength;	// Not including the tag header
	}

	return iSuccess;
}

int SkipSWFTag( SWFFile &swfFile, long length )
{
	int iSuccess = swfFile.AdvanceTagLimit( length );
	if ( NO_SWF_ERROR )
		iSuccess = swfFile.Seek( length );
	return iSuccess;
}

/******************************************************************************
 *
 * SWF Format Container
 *
 *****************************************************************************/
SWFFormatContainer::SWFFormatContainer()
{
	m_pMCTreeView = NULL;
	m_pTreeData = NULL;
	m_pDataSource = NULL;
	m_pDispObjList = NULL;
}

int SWFFormatContainer::Initialize( HINSTANCE hInstance, HWND hwndParent )
{
	int iSuccess = 0;

	WIN_CREATE_CTRL_PARAM platformParam;
	platformParam.appInst = hInstance;
	platformParam.parent = hwndParent;

	m_pMCTreeView = NULL;
	m_pTreeData = new TreeData();
	m_pDataSource = new SWFFormatDataSource();

	if ( m_pTreeData && m_pDataSource )
	{
		m_pMCTreeView = new MCTreeView( &platformParam, MCTreeView::kStandard, FALSE, m_pTreeData, m_pDataSource );
		iSuccess = 1;
	}
	
	// If we failed to initialize, it's up to the user to clean up...

	return iSuccess;
}

void SWFFormatContainer::Resize( int iWidth, int iHeight )
{
	m_pMCTreeView->Resize( iWidth, iHeight );
}

void SWFFormatContainer::AddRootDispObject( DisplayObject *pDispObj )
{
	m_pMCTreeView->AppendItem( pDispObj );
}

void SWFFormatContainer::AddDispObjectList( DispObjectList *pDispObjList )
{
	m_pDispObjList = pDispObjList;

	DispObjectList *pCurrObjList = m_pDispObjList;
	while ( pCurrObjList != NULL )
	{
		AddRootDispObject( pCurrObjList->pObject );
		pCurrObjList = pCurrObjList->pNext;
	}
}

SWFFormatContainer::~SWFFormatContainer()
{
	if ( m_pMCTreeView )
		delete m_pMCTreeView;

	if ( m_pTreeData )
		delete m_pTreeData;

	if ( m_pDataSource )
		delete m_pDataSource;

	DispObjectList *pToBeDeleted;
	DispObjectList *pCurrObjList = m_pDispObjList;
	while ( pCurrObjList )
	{
		pToBeDeleted = pCurrObjList;
		pCurrObjList = pCurrObjList->pNext;
		delete pToBeDeleted;
	}		
}
