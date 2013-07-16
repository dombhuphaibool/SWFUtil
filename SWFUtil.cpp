// PrinterShell.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "winspool.h"
#include "commdlg.h"
#include "shellapi.h"
#include "SWFUtil.h"
#include "SWFError.h"
#include "SWFFormat.h"
#include "WinMCTreeView.h"		// for InitializeMCTreeWindowClass()
#include "DisplayObject.h"
#include "SWFFile.h"

#define MAX_LOADSTRING 100

// supress 'type cast' warnings of type
// C4311 pointer truncation from <a> to <b>
// C4312 conversion from <a> to <b> of greater size
#pragma warning( disable : 4311 4312 )

#define WC_SWFANALYZER		"SWFAnalyzerWindowClass"

// Global Variables:
HINSTANCE hInst;								// current instance
HWND hMainAppWindow;							// the main application window
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass( HINSTANCE hInstance );
void				RegisterSWFAnalyzerWndClass( HINSTANCE hInstance );
BOOL				InitInstance( HINSTANCE, int );
LRESULT CALLBACK	WndProc( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK	About( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK	SWFAnalyzerWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
HWND				CreateSWFAnalyzerWindow( HINSTANCE hInstance, HWND hwndParent, const char *szTitle, DispObjectList *pObjList );
void				DisplayResult( const char* szFileName );

int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPTSTR    lpCmdLine,
                    int       nCmdShow )
{
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString( hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING );
	LoadString( hInstance, IDC_PRINTERSHELL, szWindowClass, MAX_LOADSTRING );

	if ( !hPrevInstance )
	{
		InitCommonControls();
		MyRegisterClass( hInstance );
		InitializeMCTreeWindowClass( hInstance );
		RegisterSWFAnalyzerWndClass( hInstance );
	}

	// Perform application initialization:
	if ( !InitInstance(hInstance, nCmdShow) ) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_PRINTERSHELL);

	// Process command-line arguments
	if ( lpCmdLine )
	{
		// Convert command line arguments to lower case
		int cmdLineLen = strlen( lpCmdLine );
		for (int i=0; i<cmdLineLen; ++i)
		{
			lpCmdLine[i] = tolower(lpCmdLine[i]);
		}

		// Look for swf file extensions
		char seps[] = " \t";
		char *token = strtok( lpCmdLine, seps );
		while (token != NULL)
		{
			char *ext = strstr(token, ".swf");
			if (ext)
			{
				DisplayResult( token );
			}
			token = strtok( NULL, seps );
		}
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}


int DisplayOpenFileDialog( HWND hWnd, int iBufferLen, char *pBuffer )
{
	OPENFILENAME ofn;
	memset( &ofn, 0, sizeof(OPENFILENAME) );

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	memset( pBuffer, 0, iBufferLen );
	ofn.lpstrFile = pBuffer;
	ofn.nMaxFile = iBufferLen;

	return( GetOpenFileName(&ofn) );
}

int AnalyzeHeader( SWFFile &swfFile, DispObjectList *pObjList )
{
	SWFHeader fileHeader;
	int err = ReadSWFHeader( swfFile, fileHeader );
	if ( err == SWF_ERR_NONE )
	{
		int iVersion = fileHeader.iVersion;
		int iFileLenInK = fileHeader.iFileLen / 1024;
		if ( (fileHeader.iFileLen % 1024) > 0 )
			iFileLenInK++;

		pObjList->pObject = DisplayObject::CreateDisplayObject( TString("Header"), 0, fileHeader );
	}
	else
	{
		ReportError( err );
	}

	return err;
}

int AnalyzeTags( SWFFile &swfFile, DispObjectList *pObjList, DispObjectList **hAnalyzeObjList, DisplayObject *pDispObj )
{
	SWFTagHeader tagHeader;
	tagHeader.type = SWF_TAG_UNRECOGNIZED;
	short tagHeaderSize = 0;
	int err = SWF_ERR_NONE;
	DispObjectList *pCurrObjList = pObjList;
	DispObjectList *pNewObjList;
	swfFile.InitializeTagLimit();

	// Initialize our custom SWF File analysis tag
	SWFFileAnalysis swfAnalysis;
	memset( &swfAnalysis, 0, sizeof(SWFFileAnalysis) );
	swfAnalysis.fileSize = swfFile.GetSize();

	// Initialize font code table (used for cross referencing font & text tags)
	SWFCodeTable *pCodeTableList = NULL;

	// Glyph count table used for connecting DefineFont tags with DefineFontInfo tags
	long glyphCountTable[512];
	memset( glyphCountTable, 0, sizeof(long) * 512 );

	// Initialize sound stream compression type
	SWFSoundCompression currSndStream = SWFSndCompress_none;

	bool bDone = false;
	while ( (err == SWF_ERR_NONE) && !bDone )
	{
		tagHeaderSize = 0;
		swfFile.DiscardTagLimit();
		err = ReadSWFTagHeader( swfFile, tagHeader, &tagHeaderSize );
		swfFile.InitializeTagLimit();

		if ( err == SWF_ERR_NONE )
		{
			pNewObjList = new DispObjectList;
			pNewObjList->pNext = NULL;

			switch( tagHeader.type )
			{
				case SWFTAG_SetTabIndex:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					
					int depth = 0;
					int index = 0;
					if ( err == SWF_ERR_NONE )
						err = ReadSWFSetTabIndex( swfFile, depth, index );
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_SetTabIndex), tagHeader.length, depth, index ); 

					swfAnalysis.display += tagHeader.length + tagHeaderSize;
					swfAnalysis.settab += tagHeader.length + tagHeaderSize;
					swfAnalysis.displayCount++;
					swfAnalysis.settabCount++;
				}
				break;

				case SWFTAG_PlaceObject:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFPlaceObjectTag place;
					if ( err == SWF_ERR_NONE )
						ReadSWFPlaceObjectTag( swfFile, place );
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_PlaceObject), tagHeader.length, place ); 

					swfAnalysis.control += tagHeader.length + tagHeaderSize;
					swfAnalysis.controlCount++;
				}
				break;

				case SWFTAG_PlaceObject2:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFPlaceObject2Tag place;
					if ( err == SWF_ERR_NONE )
						ReadSWFPlaceObject2Tag( swfFile, place );
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_PlaceObject2), tagHeader.length, place ); 

					swfAnalysis.control += tagHeader.length + tagHeaderSize;
					swfAnalysis.controlCount++;
				}
				break;

				case SWFTAG_RemoveObject:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFRemoveObjectTag remove;
					if ( err == SWF_ERR_NONE )
						ReadSWFRemoveObjectTag( swfFile, remove );
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_RemoveObject), tagHeader.length, remove );						

					swfAnalysis.control += tagHeader.length + tagHeaderSize;
					swfAnalysis.controlCount++;
				}
				break;
				
				case SWFTAG_RemoveObject2:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFRemoveObject2Tag remove;
					if ( err == SWF_ERR_NONE )
						ReadSWFRemoveObject2Tag( swfFile, remove );
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_RemoveObject2), tagHeader.length, remove );

					swfAnalysis.control += tagHeader.length + tagHeaderSize;
					swfAnalysis.controlCount++;
				}
				break;
				
				case SWFTAG_ShowFrame:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_ShowFrame), tagHeader.length );
					swfAnalysis.control += tagHeader.length + tagHeaderSize;
					swfAnalysis.controlCount++;
				}
				break;
				
				case SWFTAG_SetBackgroundColor:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					RGB bgColor;
					if ( err == SWF_ERR_NONE )
						err = ReadSWFRGB( swfFile, bgColor );
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_SetBackgroundColor), tagHeader.length, bgColor );
					swfAnalysis.control += tagHeader.length + tagHeaderSize;
					swfAnalysis.controlCount++;
				}
				break;
				
				case SWFTAG_FrameLabel:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_FrameLabel), tagHeader.length ); 
					
					if (pNewObjList->pObject)
					{
						char *szFileBuffer = swfFile.GetBufferAtCurrentPos();
						pNewObjList->pObject->AppendValue( szFileBuffer );
						U32 valueLen = strlen(szFileBuffer) + 1;
						swfFile.Seek( valueLen );
					}

					swfAnalysis.control += tagHeader.length + tagHeaderSize;
					swfAnalysis.controlCount++;
				}
				break;
				
				case SWFTAG_Protect:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_Protect), tagHeader.length ); 
					swfAnalysis.control += tagHeader.length + tagHeaderSize;
					swfAnalysis.controlCount++;
				}
				break;

				case SWFTAG_End:
				{
					// assert only valid if pObjList is not NULL...
					assert( !pObjList || swfFile.IsEOF() ); 
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_End), tagHeader.length ); 
					bDone = true;

					if ( pObjList && !swfFile.IsEOF() )
						ReportError( SWF_ERR_READ_UNFINISHED, tagHeader.type, tagHeader.offset );

				}
				break;

				case SWFTAG_ExportAssets:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					int count;
					SWFExportAssets* exports;
					count = 0;
					exports = NULL;
					err = ReadSWFExportAssets( swfFile, count, &exports );
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_ExportAssets), tagHeader.length, count, exports ); 
					if (exports)
						delete [] exports;
					swfAnalysis.control += tagHeader.length + tagHeaderSize;
					swfAnalysis.controlCount++;
				}
				break;

				case SWFTAG_ImportAssets:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_ImportAssets), tagHeader.length ); 
					swfAnalysis.control += tagHeader.length + tagHeaderSize;
					swfAnalysis.controlCount++;
				}
				break;

				case SWFTAG_EnableDebugger:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_EnableDebugger), tagHeader.length ); 
					swfAnalysis.control += tagHeader.length + tagHeaderSize;
					swfAnalysis.controlCount++;
				}
				break;

				case SWFTAG_EnableDebugger2:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_EnableDebugger2), tagHeader.length ); 
					swfAnalysis.control += tagHeader.length + tagHeaderSize;
					swfAnalysis.controlCount++;
				}
				break;

				case SWFTAG_DefineShape:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFShapeTag shapeTag;
					if ( err == SWF_ERR_NONE )
					{
						shapeTag.type = SWFShape1;
						shapeTag.shapeWithStyle.pRecordList = NULL;
						err = ReadSWFShapeTag( swfFile, shapeTag );
					}
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineShape), tagHeader.length, shapeTag ); 

					swfAnalysis.shape += tagHeader.length + tagHeaderSize;					
					swfAnalysis.shapeCount++;
				}
				break;

				case SWFTAG_DefineShape2:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFShapeTag shapeTag;
					if ( err == SWF_ERR_NONE )
					{
						shapeTag.type = SWFShape2;
						shapeTag.shapeWithStyle.pRecordList = NULL;
						err = ReadSWFShapeTag( swfFile, shapeTag );
					}
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineShape2), tagHeader.length, shapeTag );

					swfAnalysis.shape += tagHeader.length + tagHeaderSize;					
					swfAnalysis.shapeCount++;
				}
				break;

				case SWFTAG_DefineShape3:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFShapeTag shapeTag;
					if ( err == SWF_ERR_NONE )
					{
						shapeTag.type = SWFShape3;
						shapeTag.shapeWithStyle.pRecordList = NULL;
						err = ReadSWFShapeTag( swfFile, shapeTag );
					}
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineShape3), tagHeader.length, shapeTag ); 

					swfAnalysis.shape += tagHeader.length + tagHeaderSize;					
					swfAnalysis.shapeCount++;
				}
				break;

				case SWFTAG_DefineBits:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineBits), tagHeader.length ); 
					swfAnalysis.bitmap += tagHeader.length + tagHeaderSize;					
					swfAnalysis.bitmapCount++;
					swfAnalysis.definebitsCount++;
				}
				break;

				case SWFTAG_JPEGTables:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_JPEGTables), tagHeader.length ); 
					swfAnalysis.bitmap += tagHeader.length + tagHeaderSize;					
					swfAnalysis.bitmapCount++;
					swfAnalysis.jpegtablesCount++;
				}
				break;

				case SWFTAG_DefineBitsJPEG2:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineBitsJPEG2), tagHeader.length ); 
					swfAnalysis.bitmap += tagHeader.length + tagHeaderSize;					
					swfAnalysis.bitmapCount++;
					swfAnalysis.jpeg2 += tagHeader.length + tagHeaderSize;					
					swfAnalysis.jpeg2Count++;
				}
				break;

				case SWFTAG_DefineBitsJPEG3:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineBitsJPEG3), tagHeader.length ); 
					swfAnalysis.bitmap += tagHeader.length + tagHeaderSize;					
					swfAnalysis.bitmapCount++;
					swfAnalysis.jpeg3Count++;
				}
				break;

				case SWFTAG_DefineBitsLossless:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFBitsLosslessTag bitmapTag;
					if ( err == SWF_ERR_NONE )
						err = ReadSWFBitsLosslessTag( swfFile, bitmapTag, tagHeader.length );

					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineBitsLossless), tagHeader.length, bitmapTag, false ); 

					swfAnalysis.bitmap += tagHeader.length + tagHeaderSize;					
					swfAnalysis.bitmapCount++;
					swfAnalysis.losslessCount++;
				}
				break;

				case SWFTAG_DefineBitsLossless2:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFBitsLosslessTag bitmapTag;
					if ( err == SWF_ERR_NONE )
						err = ReadSWFBitsLossless2Tag( swfFile, bitmapTag, tagHeader.length );

					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineBitsLossless2), tagHeader.length, bitmapTag, true ); 

					swfAnalysis.bitmap += tagHeader.length + tagHeaderSize;					
					swfAnalysis.bitmapCount++;
					swfAnalysis.lossless2 += tagHeader.length + tagHeaderSize;					
					swfAnalysis.lossless2Count++;
				}
				break;

				case SWFTAG_DefineFont:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFFont fontTag;
					if ( err == SWF_ERR_NONE )
					{
						fontTag.pOffsetTable = NULL;
						fontTag.pShapeTable = NULL;
						err = ReadSWFFontTag( swfFile, fontTag, glyphCountTable );
					}
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineFont), tagHeader.length, fontTag ); 

					swfAnalysis.text += tagHeader.length + tagHeaderSize;					
					swfAnalysis.textCount++;
					swfAnalysis.definefontCount++;
				}
				break;

				case SWFTAG_DefineFont2:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFFont2 fontTag;
					if ( err == SWF_ERR_NONE )
					{
						fontTag.szFontName = NULL;
						fontTag.pFontOffsetTable = NULL;
						fontTag.pShapeTable = NULL;
						fontTag.pCodeTable = NULL;
						fontTag.pAdvanceTable = NULL;
						fontTag.pBoundsTable = NULL;
						fontTag.pKerningTable = NULL;
						err = ReadSWFFont2Tag( swfFile, fontTag, &pCodeTableList );
					}
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineFont2), tagHeader.length, fontTag ); 

					swfAnalysis.text += tagHeader.length + tagHeaderSize;					
					swfAnalysis.definefont2 += tagHeader.length + tagHeaderSize;					
					swfAnalysis.textCount++;
					swfAnalysis.definefont2Count++;
				}
				break;

				case SWFTAG_DefineFontInfo:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFFontInfo fontInfo;
					if ( err == SWF_ERR_NONE )
					{
						fontInfo.header.szFontName = NULL;
						fontInfo.pCodeTable = NULL;
						err = ReadSWFFontInfoTag( swfFile, fontInfo, glyphCountTable, &pCodeTableList );
					}
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineFontInfo), tagHeader.length, fontInfo, glyphCountTable[fontInfo.header.fontID] ); 

					swfAnalysis.text += tagHeader.length + tagHeaderSize;					
					swfAnalysis.textCount++;
					swfAnalysis.definefontinfoCount++;
				}
				break;

				case SWFTAG_DefineFontInfo2:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFFontInfo2 fontInfo;
					if ( err == SWF_ERR_NONE )
					{
						fontInfo.header.szFontName = NULL;
						fontInfo.pCodeTable = NULL;
						err = ReadSWFFontInfo2Tag( swfFile, fontInfo, glyphCountTable, &pCodeTableList );
					}

					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineFontInfo2), tagHeader.length, fontInfo, glyphCountTable[fontInfo.header.fontID] ); 

					swfAnalysis.text += tagHeader.length + tagHeaderSize;					
					swfAnalysis.textCount++;
					swfAnalysis.definefontinfo2Count++;
				}
				break;

				case SWFTAG_DefineText:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFText textTag;
					if ( err == SWF_ERR_NONE )
					{
						textTag.pRecordList = NULL;
						err = ReadSWFTextTag( swfFile, textTag, false );
					}
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineText), tagHeader.length, textTag, pCodeTableList ); 

					swfAnalysis.text += tagHeader.length + tagHeaderSize;					
					swfAnalysis.definetext += tagHeader.length + tagHeaderSize;					
					swfAnalysis.textCount++;
					swfAnalysis.definetextCount++;
				}
				break;

				case SWFTAG_DefineText2:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFText textTag;
					if ( err == SWF_ERR_NONE )
					{
						textTag.pRecordList = NULL;
						err = ReadSWFTextTag( swfFile, textTag, true );
					}
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineText2), tagHeader.length, textTag, pCodeTableList ); 

					swfAnalysis.text += tagHeader.length + tagHeaderSize;					
					swfAnalysis.textCount++;
					swfAnalysis.definetext2Count++;
				}
				break;

				case SWFTAG_DefineEditText:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFEditText editText;
					if ( err == SWF_ERR_NONE )
					{
						editText.szInitialText = NULL;
						editText.szVariableName = NULL;
						err = ReadSWFEditTextTag( swfFile, editText );
					}
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineEditText), tagHeader.length, editText ); 

					swfAnalysis.text += tagHeader.length + tagHeaderSize;					
					swfAnalysis.textCount++;
					swfAnalysis.edittextCount++;
				}
				break;

				case SWFTAG_DoAction:
				{
					long constPoolTotal = 0;
					std::vector<SWFActionRecord> actions;
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					if ( err == SWF_ERR_NONE )
						ReadSWFActionTag(swfFile, actions, NULL, constPoolTotal );
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DoAction), tagHeader.length, actions, NULL ); 
					
					swfAnalysis.constPool += constPoolTotal;
					swfAnalysis.action += tagHeader.length + tagHeaderSize;
					swfAnalysis.actionCount++;
				}
				break;

				case SWFTAG_DoInitAction:
				{
					long constPoolTotal = 0;
					long spriteID = 0;
					std::vector<SWFActionRecord> actions;
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					if ( err == SWF_ERR_NONE )
						ReadSWFActionTag( swfFile, actions, &spriteID, constPoolTotal );
					if ( err == SWF_ERR_NONE )
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DoInitAction), tagHeader.length, actions, &spriteID ); 

					swfAnalysis.constPool += constPoolTotal;
					swfAnalysis.initaction += tagHeader.length + tagHeaderSize;
					swfAnalysis.initactionCount++;
				}
				break;

				case SWFTAG_DefineSound:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineSound), tagHeader.length ); 
					swfAnalysis.sound += tagHeader.length + tagHeaderSize;
					swfAnalysis.soundCount++;
				}
				break;

				case SWFTAG_StartSound:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_StartSound), tagHeader.length ); 
					swfAnalysis.sound += tagHeader.length + tagHeaderSize;
					swfAnalysis.soundCount++;
				}
				break;

				case SWFTAG_SoundStreamHead:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFSoundStreamHead soundStreamHead;
					soundStreamHead.type = SWFSoundStreamHead1;
					if ( err == SWF_ERR_NONE )
					{
						err = ReadSWFSoundStreamHeader( swfFile, soundStreamHead );
					}
					if ( err == SWF_ERR_NONE )
					{
						currSndStream = soundStreamHead.compressionType;
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_SoundStreamHead), tagHeader.length, soundStreamHead ); 
					}

					swfAnalysis.sound += tagHeader.length + tagHeaderSize;
					swfAnalysis.soundCount++;
				}
				break;

				case SWFTAG_SoundStreamHead2:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );
					SWFSoundStreamHead soundStreamHead;
					soundStreamHead.type = SWFSoundStreamHead2;
					if ( err == SWF_ERR_NONE )
					{
						err = ReadSWFSoundStreamHeader( swfFile, soundStreamHead );
					}
					if ( err == SWF_ERR_NONE )
					{
						currSndStream = soundStreamHead.compressionType;
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_SoundStreamHead2), tagHeader.length, soundStreamHead ); 
					}

					swfAnalysis.sound += tagHeader.length + tagHeaderSize;
					swfAnalysis.soundCount++;
				}
				break;

				case SWFTAG_SoundStreamBlock:
				{
					if (currSndStream == SWFSndCompress_MP3)
					{
						err = swfFile.AdvanceTagLimit( tagHeader.length );
						SWFMP3StreamBlock mp3StreamBlock;
						if ( err == SWF_ERR_NONE )
						{
							err = ReadSWFMP3StreamBlock( swfFile, mp3StreamBlock );
						}
						if ( err == SWF_ERR_NONE )
							pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_SoundStreamBlock), tagHeader.length, mp3StreamBlock ); 

						// Skip the MP3 data
						swfFile.SeekToTagLimit();
					}
					else
					{
						err = SkipSWFTag( swfFile, tagHeader.length );
						pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_SoundStreamBlock), tagHeader.length ); 
					}

					swfAnalysis.sound += tagHeader.length + tagHeaderSize;
					swfAnalysis.soundCount++;
				}
				break;

				case SWFTAG_DefineButton:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineButton), tagHeader.length ); 
					swfAnalysis.button += tagHeader.length + tagHeaderSize;
					swfAnalysis.buttonCount++;
				}
				break;

				case SWFTAG_DefineButton2:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineButton2), tagHeader.length ); 
					swfAnalysis.button += tagHeader.length + tagHeaderSize;
					swfAnalysis.buttonCount++;
				}
				break;

				case SWFTAG_DefineButtonCxform:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineButtonCxform), tagHeader.length ); 
					swfAnalysis.button += tagHeader.length + tagHeaderSize;
					swfAnalysis.buttonCount++;
				}
				break;

				case SWFTAG_DefineButtonSound:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineButtonSound), tagHeader.length ); 
					swfAnalysis.button += tagHeader.length + tagHeaderSize;
					swfAnalysis.buttonCount++;
				}
				break;

				case SWFTAG_DefineSprite:
				{
					err = swfFile.AdvanceTagLimit( tagHeader.length );

					DisplayObject *pSpriteObj = NULL;
					SWFSpriteTag spriteTag;
					if ( err == SWF_ERR_NONE )
						err = ReadSWFSpriteTag( swfFile, spriteTag );
					if ( err == SWF_ERR_NONE )
						pSpriteObj = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_DefineSprite), tagHeader.length, spriteTag ); 

					if ( err == SWF_ERR_NONE )
					{
						AnalyzeTags( swfFile, NULL, NULL, pSpriteObj );
						pNewObjList->pObject = pSpriteObj;
					}

					swfAnalysis.display += tagHeader.length + tagHeaderSize;
					swfAnalysis.definesprite += tagHeader.length + tagHeaderSize;
					swfAnalysis.displayCount++;
					swfAnalysis.definespriteCount++;
				}
				break;

				default:
				{
					err = SkipSWFTag( swfFile, tagHeader.length );
	
					pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString(SWFTAGNAME_Unrecognized), tagHeader.length, tagHeader );
					swfAnalysis.other += tagHeader.length + tagHeaderSize;
					swfAnalysis.otherCount++;
				}
			}

			if ( pObjList ) // Add to the Object List
			{
				pCurrObjList->pNext = pNewObjList;
				pCurrObjList = pNewObjList;
			}
			else if ( pDispObj ) // Called by DefineSprite, add as child
			{
				pDispObj->AddChild( pNewObjList->pObject );
				delete pNewObjList;
			}
		}

		// If we violated the tag limit, skip the tag, report 
		// the error and keep reading the SWF file...
		if ( err == SWF_ERR_TAGLIMIT_VIOLATED )
		{
			ReportError( err, tagHeader.type, tagHeader.offset );
			err = SWF_ERR_NONE;
		}

		// Check if the tag limit was reached. If not,
		// notify the user and advance to the next tag.
		if ( !swfFile.TagLimitReached() )
		{
			ReportError( SWF_ERR_TAGLIMITNOTREACHED, tagHeader.type, tagHeader.offset );
			swfFile.SeekToTagLimit();
		}
	}

	// Append a SWF analysis tag to the end
	if ( hAnalyzeObjList && (err == SWF_ERR_NONE) )
	{
		DispObjectList *pEmptyTag = new DispObjectList;
		pEmptyTag->pObject = DisplayObject::CreateDisplayObject( TString("<=====================>"), 0 );
		pEmptyTag->pNext = NULL;

		pNewObjList = new DispObjectList;
		pNewObjList->pNext = pEmptyTag;
		pNewObjList->pObject = DisplayObject::CreateDisplayObject( TString("SWF Spy Analysis"), swfAnalysis );

		*hAnalyzeObjList = pNewObjList;
	}

	// Clean up the font code table list
	if ( pCodeTableList )
		ReleaseFontCodeTableList( pCodeTableList );

	if ( err != SWF_ERR_NONE )
		ReportError( err, tagHeader.type, tagHeader.offset );

	return err;
}

HWND CreateSWFAnalyzerWindow( HINSTANCE hInstance, HWND hwndParent, const char *szTitle, DispObjectList *pObjList )
{
	HWND hWnd = CreateWindow( WC_SWFANALYZER, 
							 szTitle, 
							 WS_OVERLAPPEDWINDOW,
							 CW_USEDEFAULT, 0, 
							 400, 300, 
							 hwndParent, 
							 NULL, 
							 hInstance, 
							 NULL );

	SWFFormatContainer *pContainer = NULL;
	if ( hWnd )
	{
		pContainer = new SWFFormatContainer();
		assert( pContainer );

		int iSuccess = pContainer->Initialize( hInstance, hWnd );
		if ( pContainer && iSuccess )
		{
		   	SetWindowLong( hWnd, GWL_USERDATA, (LONG) pContainer );
			ShowWindow( hWnd, true );
			UpdateWindow( hWnd );

			pContainer->AddDispObjectList( pObjList );
		}
		
		if ( pContainer && !iSuccess )
		{
			delete pContainer;
			pContainer = NULL;
		}

		if ( !pContainer )
		{
			DestroyWindow( hWnd );
			hWnd = NULL;
		}
	}

	return hWnd;
}

void DisplayResult( const char* szFileName )
{
	SWFFile swfFile;
	int iSuccess = swfFile.Initialize( szFileName );
	if ( iSuccess == SWF_ERR_NONE )
	{
		DispObjectList *pObjList = new DispObjectList;
		pObjList->pObject = NULL;
		pObjList->pNext = NULL;
		DispObjectList *pAnalyzeObjList = NULL;

		AnalyzeHeader( swfFile, pObjList );
		AnalyzeTags( swfFile, pObjList, &pAnalyzeObjList, NULL );

		assert( pAnalyzeObjList );
		if ( pAnalyzeObjList )
			AppendDispObjList( pAnalyzeObjList, pObjList );

		HWND hwndSFWAnalyzer = CreateSWFAnalyzerWindow( hInst, hMainAppWindow , szFileName, pAnalyzeObjList );
	}
	else 
	{
		ReportError( iSuccess );
	}
}

void AnalyzeSWF( HWND hWnd )
{
	char szFileName[512];
	if ( DisplayOpenFileDialog(hWnd, 512, szFileName) )
	{
		DisplayResult( szFileName );
	}
}

void ConvertDocument( HWND hWnd )
{
	/*
	PRINTER_INFO_1 printerInfo;
	memset( &printerInfo, 0, sizeof(PRINTER_INFO_1) );
	if ( EnumPrinters(PRINTER_ENUM_NAME, "Flash Printer", 1, &printerInfo, sizeof(PRINTER_INFO_1), 
	*/

	char szFileName[512];
	if ( DisplayOpenFileDialog(hWnd, 512, szFileName) )
	{
		// EnumPrinter
		// SetPrinter
		// OpenPrinter
		HANDLE hPrinter;
		if ( OpenPrinter("Flash Printer", &hPrinter, NULL) )
		{
			ShellExecute( NULL, "print", szFileName, NULL, "C:\\", SW_MINIMIZE );
			ClosePrinter( hPrinter );
		}
	}
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_PRINTERSHELL);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_PRINTERSHELL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

void RegisterSWFAnalyzerWndClass( HINSTANCE hInstance )
{
	WNDCLASSEX wc;

	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= (WNDPROC) SWFAnalyzerWndProc;
	wc.cbClsExtra		= NULL;
	wc.cbWndExtra		= NULL;
	wc.hInstance		= hInstance;
	wc.hIcon			= NULL;
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= WC_SWFANALYZER;
	wc.hIconSm			= NULL;

	RegisterClassEx( &wc );
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 200, 150, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   hMainAppWindow = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_FILE_ANALYZESWF:
			AnalyzeSWF( hWnd );
			break;
		case ID_FILE_CONVERTDOC:
			ConvertDocument( hWnd );
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK SWFAnalyzerWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		case WM_SIZE:
		{
			int iWidth = LOWORD( lParam );
			int iHeight = HIWORD( lParam );
			SWFFormatContainer *pContainer = (SWFFormatContainer *) GetWindowLong( hWnd, GWL_USERDATA );
			if ( pContainer )
				pContainer->Resize( iWidth, iHeight );
		}
		break;

		case WM_DESTROY:
		{
			SWFFormatContainer *pContainer = (SWFFormatContainer *) GetWindowLong( hWnd, GWL_USERDATA );
			if ( pContainer )
				delete pContainer;
		}
		break;
	}
	return DefWindowProc( hWnd, message, wParam, lParam );
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
