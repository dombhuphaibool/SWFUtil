#ifndef __SWFFORMAT_H__
#define __SWFFORMAT_H__

#include "MCTreeView.h"
#include "TreeData.h"
#include "DataSource.h"
#include "DisplayObject.h"
#include <vector>
#include <string>

#define SWF_TAG_UNRECOGNIZED			0
#define SWF_TAG_END						1

class SWFFile;

/******************************************************************************
 *
 * SWF primitive types
 *
 *****************************************************************************/
typedef char			S8;
typedef short			S16;
typedef long			S32;
typedef unsigned char	U8;
typedef	unsigned short	U16;
typedef unsigned long	U32;

#define S8_SIZE		sizeof(char)
#define S16_SIZE	sizeof(short)
#define S32_SIZE	sizeof(long)
#define	U8_SIZE		sizeof(unsigned char)
#define U16_SIZE	sizeof(unsigned short)
#define U32_SIZE	sizeof(unsigned long)

typedef U8	SWFLangCode;

struct RGB {
	U8 red;
	U8 green;
	U8 blue;
};

struct RGBA {
	U8 red;
	U8 green;
	U8 blue;
	U8 alpha;
};

int ReadSWFRGB( SWFFile &swfFile, RGB &rgb );

// our representation of SWF RECT
struct SWFRect {
	long left;
	long right;
	long top;
	long bottom;

	SWFRect() : left(0), right(0), top(0), bottom(0) {}
};

int ReadSWFRect( SWFFile &swfFile, SWFRect &rect );

// our representation of SWF MATRIX
struct SWFMatrix {
	bool hasScale;
	long xScale;
	long yScale;

	bool hasRotate;
	long rotateSkew0;
	long rotateSkew1;

	long xTranslate;
	long yTranslate;

	SWFMatrix() :
		hasScale(false),
		xScale(0),
		yScale(0),
		hasRotate(false),
		rotateSkew0(0),
		rotateSkew1(0),
		xTranslate(0),
		yTranslate(0)
	{
	}
};

int ReadSWFMatrix( SWFFile &swfFile, SWFMatrix &matrix );

// our representation of SWF Color Xform
struct SWFColorXForm {
	bool bHasAdd;
	bool bHasMult;
	bool bHasAlpha;

	long multRed;
	long multGreen;
	long multBlue;

	long addRed;
	long addGreen;
	long addBlue;

	long multAlpha;
	long addAlpha;

	SWFColorXForm() :
		bHasAdd(false),
		bHasMult(false),
		bHasAlpha(false),
		multRed(0),
		multGreen(0),
		multBlue(0),
		addRed(0),
		addGreen(0),
		addBlue(0),
		multAlpha(0),
		addAlpha(0)
	{
	}
};

int ReadSWFColorXForm( SWFFile &swfFile, SWFColorXForm &color );

/******************************************************************************
 *
 * SWF tag codes
 *
 *****************************************************************************/
// Display List
#define SWFTAG_PlaceObject				4
#define SWFTAG_PlaceObject2				26
#define SWFTAG_RemoveObject				5
#define SWFTAG_RemoveObject2			28
#define SWFTAG_ShowFrame				1
#define SWFTAG_SetTabIndex				66
#define SWFTAGNAME_PlaceObject			"PlaceObject"
#define SWFTAGNAME_PlaceObject2			"PlaceObject2"
#define SWFTAGNAME_RemoveObject			"RemoveObject"
#define SWFTAGNAME_RemoveObject2		"RemoveObject2"
#define SWFTAGNAME_ShowFrame			"ShowFrame"
#define SWFTAGNAME_SetTabIndex			"SetTabIndex"

// Control Tags
#define SWFTAG_SetBackgroundColor		9
#define SWFTAG_FrameLabel				43
#define SWFTAG_Protect					24
#define SWFTAG_End						0
#define SWFTAG_ExportAssets				56
#define SWFTAG_ImportAssets				57
#define SWFTAG_EnableDebugger			58
#define SWFTAG_EnableDebugger2			64
#define SWFTAGNAME_SetBackgroundColor	"SetBackgroundColor"
#define SWFTAGNAME_FrameLabel			"FrameLabel"
#define SWFTAGNAME_Protect				"Protect"
#define SWFTAGNAME_End					"End"
#define SWFTAGNAME_ExportAssets			"Export Assets"
#define SWFTAGNAME_ImportAssets			"Import Assets"
#define SWFTAGNAME_EnableDebugger		"Enable Debugger"
#define SWFTAGNAME_EnableDebugger2		"Enable Debugger2"

// Shapes
#define SWFTAG_DefineShape				2
#define SWFTAG_DefineShape2				22
#define SWFTAG_DefineShape3				32
#define SWFTAGNAME_DefineShape			"DefineShape"
#define SWFTAGNAME_DefineShape2			"DefineShape2"
#define SWFTAGNAME_DefineShape3			"DefineShape3"

// Bitmaps
#define SWFTAG_DefineBits				6
#define SWFTAG_JPEGTables				8
#define SWFTAG_DefineBitsJPEG2			21
#define SWFTAG_DefineBitsJPEG3			35
#define SWFTAG_DefineBitsLossless		20
#define SWFTAG_DefineBitsLossless2		36
#define SWFTAGNAME_DefineBits			"DefineBits"
#define SWFTAGNAME_JPEGTables			"JPEGTables"
#define SWFTAGNAME_DefineBitsJPEG2		"DefineBitsJPEG2"
#define SWFTAGNAME_DefineBitsJPEG3		"DefineBitsJPEG3"
#define SWFTAGNAME_DefineBitsLossless	"DefineBitsLossless"
#define SWFTAGNAME_DefineBitsLossless2	"DefineBitsLossless2"

// Text
#define SWFTAG_DefineFont				10
#define SWFTAG_DefineFont2				48
#define SWFTAG_DefineFontInfo			13
#define SWFTAG_DefineFontInfo2			62
#define SWFTAG_DefineText				11
#define SWFTAG_DefineText2				33
#define SWFTAG_DefineEditText			37
#define SWFTAGNAME_DefineFont			"DefineFont"
#define SWFTAGNAME_DefineFont2			"DefineFont2"
#define SWFTAGNAME_DefineFontInfo		"DefineFontInfo"
#define SWFTAGNAME_DefineFontInfo2		"DefineFontInfo2"
#define SWFTAGNAME_DefineText			"DefineText"
#define SWFTAGNAME_DefineText2			"DefineText2"
#define SWFTAGNAME_DefineEditText		"DefineEditText"

// ActionScript
#define SWFTAG_DoAction					12
#define SWFTAGNAME_DoAction				"DoAction"
#define SWFTAG_DoInitAction				59
#define SWFTAGNAME_DoInitAction			"DoInitAction"

// Sound
#define SWFTAG_DefineSound				14
#define SWFTAG_StartSound				15
#define SWFTAG_SoundStreamHead			18
#define SWFTAG_SoundStreamHead2			45
#define SWFTAG_SoundStreamBlock			19
#define SWFTAGNAME_DefineSound			"DefineSound"
#define SWFTAGNAME_StartSound			"StartSound"
#define SWFTAGNAME_SoundStreamHead		"SoundStreamHead"
#define SWFTAGNAME_SoundStreamHead2		"SoundStreamHead2"
#define SWFTAGNAME_SoundStreamBlock		"SoundStreamBlock"

// Buttons
#define SWFTAG_DefineButton				7
#define SWFTAG_DefineButton2			34
#define SWFTAG_DefineButtonCxform		23
#define SWFTAG_DefineButtonSound		17
#define SWFTAGNAME_DefineButton			"DefineButton"
#define SWFTAGNAME_DefineButton2		"DefineButton2"
#define SWFTAGNAME_DefineButtonCxform	"DefineButtonColorTransform"
#define SWFTAGNAME_DefineButtonSound	"DefineButtonSound"

// Sprites and Movie Clips
#define SWFTAG_DefineSprite				39
#define SWFTAGNAME_DefineSprite			"DefineSprite"

// Unrecognized Tag
#define SWFTAGNAME_Unrecognized			"Unrecognized Tag"
#define EMPTY_SPACE						" "

/******************************************************************************
 *
 * SWF ActionScript codes
 *
 *****************************************************************************/
#define SWFACTION_GOTOFRAME					0x81
#define SWFACTION_GETURL					0x83
#define SWFACTION_NEXTFRAME					0x04
#define SWFACTION_PREVFRAME					0x05
#define SWFACTION_PLAY						0x06
#define SWFACTION_STOP						0x07
#define SWFACTION_TOGGLEQUAL				0x08
#define SWFACTION_STOPSOUNDS				0x09
#define SWFACTION_WAITFRAME					0x8A
#define SWFACTION_SETTARGET					0x8B
#define SWFACTION_GOTOLABEL					0x8C

#define SWFACTIONNAME_GOTOFRAME				"GotoFrame"
#define SWFACTIONNAME_GETURL				"GetURL"
#define SWFACTIONNAME_NEXTFRAME				"NextFrame"
#define SWFACTIONNAME_PREVFRAME				"PreviousFrame"
#define SWFACTIONNAME_PLAY					"Play"
#define SWFACTIONNAME_STOP					"Stop"
#define SWFACTIONNAME_TOGGLEQUAL			"ToggleQuality"
#define SWFACTIONNAME_STOPSOUNDS			"StopSounds"
#define SWFACTIONNAME_WAITFRAME				"WaitForFrame"
#define SWFACTIONNAME_SETTARGET				"SetTarget"
#define SWFACTIONNAME_GOTOLABEL				"GotoLabel"

#define SWFACTION_PUSH						0x96
#define SWFACTION_POP						0x17
#define SWFACTION_ADD						0x0A
#define SWFACTION_SUBTRACT					0x0B
#define SWFACTION_MULTIPLY					0x0C
#define SWFACTION_DIVIDE					0x0D
#define SWFACTION_EQUALS					0x0E
#define SWFACTION_LESS						0x0F
#define SWFACTION_AND						0x10
#define SWFACTION_OR						0x11
#define SWFACTION_NOT						0x12

#define SWFACTIONNAME_PUSH					"Push"
#define SWFACTIONNAME_POP					"Pop"
#define SWFACTIONNAME_ADD					"Add"
#define SWFACTIONNAME_SUBTRACT				"Subtract"
#define SWFACTIONNAME_MULTIPLY				"Multiply"
#define SWFACTIONNAME_DIVIDE				"Divide"
#define SWFACTIONNAME_EQUALS				"Equals"
#define SWFACTIONNAME_LESS					"LessThan"
#define SWFACTIONNAME_AND					"And"
#define SWFACTIONNAME_OR					"Or"
#define SWFACTIONNAME_NOT					"Not"

#define SWFACTION_STREQUALS					0x13
#define SWFACTION_STRLEN					0x14
#define SWFACTION_STRADD					0x21
#define SWFACTION_STREXTRACT				0x15
#define SWFACTION_STRLESS					0x29
#define SWFACTION_MBSTRLEN					0x31
#define SWFACTION_MBSTREXTRACT				0x35

#define SWFACTIONNAME_STREQUALS				"StringEquals"
#define SWFACTIONNAME_STRLEN				"StringLength"
#define SWFACTIONNAME_STRADD				"StringAdd"
#define SWFACTIONNAME_STREXTRACT			"StringSubtract"
#define SWFACTIONNAME_STRLESS				"StringLess Than"
#define SWFACTIONNAME_MBSTRLEN				"Multi-ByteStringLength"
#define SWFACTIONNAME_MBSTREXTRACT			"Multi-ByteStringExtract"

#define SWFACTION_TOINT						0x18
#define SWFACTION_CHARTOASCII				0x32
#define SWFACTION_ASCIITOCHAR				0x33
#define SWFACTION_MBCHARTOASCII				0x36
#define SWFACTION_MBASCIITOCHAR				0x37

#define SWFACTIONNAME_TOINT					"To Integer"
#define SWFACTIONNAME_CHARTOASCII			"Character To Ascii"
#define SWFACTIONNAME_ASCIITOCHAR			"Ascii To Character"
#define SWFACTIONNAME_MBCHARTOASCII			"Multi-Byte Character to Ascii"
#define SWFACTIONNAME_MBASCIITOCHAR			"Ascii to Multi-Byte Character"

#define SWFACTION_JUMP						0x99
#define SWFACTION_IF						0x9D
#define SWFACTION_CALL						0x9E

#define SWFACTIONNAME_JUMP					"Jump"
#define SWFACTIONNAME_IF					"If"
#define SWFACTIONNAME_CALL					"Call"

#define SWFACTION_GETVAR					0x1C
#define SWFACTION_SETVAR					0x1D

#define SWFACTIONNAME_GETVAR				"GetVariable"
#define SWFACTIONNAME_SETVAR				"SetVariable"

#define SWFACTION_GETURL2					0x9A
#define SWFACTION_GOTOFRAME2				0x9F
#define SWFACTION_SETTARGET2				0x20
#define SWFACTION_GETPROP					0x22
#define SWFACTION_SETPROP					0x23
#define SWFACTION_CLONESPRITE				0x24
#define SWFACTION_REMOVESPRITE				0x25
#define SWFACTION_STARTDRAG					0x27
#define SWFACTION_ENDDRAG					0x28
#define SWFACTION_WAITFRAME2				0x8D

#define SWFACTIONNAME_GETURL2				"Get URL2"
#define SWFACTIONNAME_GOTOFRAME2			"Goto Frame2"
#define SWFACTIONNAME_SETTARGET2			"Set Target2"
#define SWFACTIONNAME_GETPROP				"Get Property"
#define SWFACTIONNAME_SETPROP				"Set Property"
#define SWFACTIONNAME_CLONESPRITE			"Clone Sprite"
#define SWFACTIONNAME_REMOVESPRITE			"Remove Sprite"
#define SWFACTIONNAME_STARTDRAG				"Start Drag"
#define SWFACTIONNAME_ENDDRAG				"End Drag"
#define SWFACTIONNAME_WAITFRAME2			"Wait For Frame2"

#define SWFACTION_TRACE						0x26
#define SWFACTION_GETTIME					0x34
#define SWFACTION_RAND						0x30

#define SWFACTIONNAME_TRACE					"Trace"
#define SWFACTIONNAME_GETTIME				"Get Time"
#define SWFACTIONNAME_RAND					"Random Number"

#define SWFACTION_CALLFUNC					0x3D
#define SWFACTION_CALLMETH					0x52
#define SWFACTION_CONSTPOOL					0x88
#define SWFACTION_DEFFUNC					0x9B
#define SWFACTION_DEFFUNC2					0x8E
#define SWFACTION_DEFLOCAL					0x3C
#define SWFACTION_DEFLOCAL2					0x41
#define SWFACTION_DELETE					0x3A
#define SWFACTION_DELETE2					0x3B
#define SWFACTION_ENUM						0x46
#define SWFACTION_EQUALS2					0x49
#define SWFACTION_GETMEMBER					0x4E
#define SWFACTION_INITARRAY					0x42
#define SWFACTION_INITOBJECT				0x43
#define SWFACTION_NEWMETH					0x53
#define SWFACTION_NEWOBJECT					0x40
#define SWFACTION_SETMEMBER					0x4F
#define SWFACTION_TARGETPATH				0x45
#define SWFACTION_WITH						0x94

#define SWFACTIONNAME_CALLFUNC				"CallFunction"
#define SWFACTIONNAME_CALLMETH				"CallMethod"
#define SWFACTIONNAME_CONSTPOOL				"ConstPool"
#define SWFACTIONNAME_DEFFUNC				"DefineFunction"
#define SWFACTIONNAME_DEFFUNC2				"DefineFunction2"
#define SWFACTIONNAME_DEFLOCAL				"DefineLocal"
#define SWFACTIONNAME_DEFLOCAL2				"DefineLocal2"
#define SWFACTIONNAME_DELETE				"Delete"
#define SWFACTIONNAME_DELETE2				"Delete2"
#define SWFACTIONNAME_ENUM					"Enumerate"
#define SWFACTIONNAME_EQUALS2				"Equals2"
#define SWFACTIONNAME_GETMEMBER				"GetMember"
#define SWFACTIONNAME_INITARRAY				"InitArray"
#define SWFACTIONNAME_INITOBJECT			"InitObject"
#define SWFACTIONNAME_NEWMETH				"NewMethod"
#define SWFACTIONNAME_NEWOBJECT				"NewObject"
#define SWFACTIONNAME_SETMEMBER				"SetMember"
#define SWFACTIONNAME_TARGETPATH			"TargetPath"
#define SWFACTIONNAME_WITH					"With"

#define SWFACTION_TONUMBER					0x4A
#define SWFACTION_TOSTRING					0x4B
#define SWFACTION_TYPEOF					0x44

#define SWFACTIONNAME_TONUMBER				"Convert To Number"
#define SWFACTIONNAME_TOSTRING				"Convert To String"
#define SWFACTIONNAME_TYPEOF				"Type Of"

#define SWFACTION_ADD2						0x47
#define SWFACTION_LESS2						0x48
#define SWFACTION_MODULO					0x3F

#define SWFACTIONNAME_ADD2					"Add2"
#define SWFACTIONNAME_LESS2					"Less2"
#define SWFACTIONNAME_MODULO				"Modulo"

#define SWFACTION_BITAND					0x60
#define SWFACTION_BITLSHIFT					0x63
#define SWFACTION_BITOR						0x61
#define SWFACTION_BITRSHIFT					0x64
#define SWFACTION_BITURSHIFT				0x65
#define SWFACTION_BITXOR					0x62
#define SWFACTION_DECREMENT					0x51
#define SWFACTION_INCREMENT					0x50
#define SWFACTION_PUSHDUP					0x4C
#define SWFACTION_RETURN					0x3E
#define SWFACTION_STACKSWAP					0x4D
#define SWFACTION_STOREREG					0x87

#define SWFACTIONNAME_BITAND				"Bitwise And"
#define SWFACTIONNAME_BITLSHIFT				"Bitwise Left Shift"
#define SWFACTIONNAME_BITOR					"Bitwise Or"
#define SWFACTIONNAME_BITRSHIFT				"Bitwise Right Shift"
#define SWFACTIONNAME_BITURSHIFT			"Unsigned Bitwise Right Shift" 
#define SWFACTIONNAME_BITXOR				"Bitwise XOR"
#define SWFACTIONNAME_DECREMENT				"Decrement"
#define SWFACTIONNAME_INCREMENT				"Increment"
#define SWFACTIONNAME_PUSHDUP				"Push Duplicate"
#define SWFACTIONNAME_RETURN				"Return"
#define SWFACTIONNAME_STACKSWAP				"Swap top two items on stack"
#define SWFACTIONNAME_STOREREG				"Store in Register"

#define SWFACTION_INSTANCEOF				0x54
#define SWFACTION_ENUM2						0x55
#define SWFACTION_STRICTEQUALS				0x66
#define SWFACTION_GREATER					0x67
#define SWFACTION_STRGREATER				0x68

#define SWFACTIONNAME_INSTANCEOF			"InstanceOf"
#define SWFACTIONNAME_ENUM2					"Enumerate2"
#define SWFACTIONNAME_STRICTEQUALS			"Strict Equals (===)"
#define SWFACTIONNAME_GREATER				"Greater"
#define SWFACTIONNAME_STRGREATER			"String Greater"

#define SWFACTIONNAME_UNRECOGNIZED			"Unrecognized Action Code"

/******************************************************************************
 *
 * SWF Display Labels
 *
 *****************************************************************************/
// SWFRect fields
#define SWFNAME_LEFT					"left"
#define SWFNAME_RIGHT					"right"
#define SWFNAME_TOP						"top"
#define SWFNAME_BOTTOM					"bottom"
// SWFMatrix fields
#define SWFNAME_SCALEX					"Scale X"
#define SWFNAME_SCALEY					"Scale Y"
#define SWFNAME_ROTATE0					"Rotate/Skew 0"
#define SWFNAME_ROTATE1					"Rotate/Skew 1"
#define SWFNAME_TRANSLATEX				"Translate X"
#define SWFNAME_TRANSLATEY				"Translate Y"
// SWFShapeTag fields
#define SWFNAME_SHAPEID					"Shape ID"
#define SWFNAME_SHAPEWITHSTYLE			"Shapes"
#define SWFNAME_SHAPEBOUNDS				"Shape Bounds"
// SWFShapeWithStyle fields
#define SWFNAME_FILLSTYLEARRAY			"FillStyles"
#define SWFNAME_LINESTYLEARRAY			"LineStyles"
#define SWFNAME_NUMFILLBITS				"NumFillBits"
#define SWFNAME_NUMLINEBITS				"NumLineBits"
#define SWFNAME_SHAPERECORDS			"ShapeRecords"
// SWFFillStyle
#define SWFNAME_FILLSTYLETYPE			"type"
#define SWFNAME_SOLIDFILL				"Solid Fill"
#define SWFNAME_LINEARGRAD				"Linear Gradient Fill"
#define SWFNAME_RADIALGRAD				"Radial Gradient Fill"
#define SWFNAME_TILEDBITMAP				"Tiled Bitmap Fill"
#define SWFNAME_CLIPPEDBITMAP			"Clipped Bitmap Fill"
#define SWFNAME_GRADIENTMATRIX			"Gradient Matrix"
#define SWFNAME_GRADIENTFILL			"Gradient Fill"
#define SWFNAME_BITMAPID				"Bitmap ID"
#define SWFNAME_BITMAPMATRIX			"Bitmap Matrix"
// SWF Shape Records
#define SWFNAME_ENDSHAPEREC				"End Shape Record"
#define SWFNAME_STYLECHANGEREC			"Style Change Record"
#define SWFNAME_STRAIGHTEDGEREC			"Straight Edge Record"
#define SWFNAME_CURVEDEDGEREC			"Curved Edge Record"
#define SWFNAME_GENERALSTRAIGHTEDGE		"General Straight Edge Record"
#define SWFNAME_VERTSTRAIGHTEDGE		"Vertical Straight Edge Record"
#define SWFNAME_HORZSTRAIGHTEDGE		"Horizontal Straight Edge Record"
#define SWFNAME_CONTROLDELTAX			"Control Delta X"
#define SWFNAME_CONTROLDELTAY			"Control Delta Y"
#define SWFNAME_ANCHORDELTAX			"Anchor Delta X"
#define SWFNAME_ANCHORDELTAY			"Anchor Delta Y"
#define SWFNAME_MOVEDELTAX				"Move Delta X"
#define SWFNAME_MOVEDELTAY				"Move Delta Y"
#define SWFNAME_FILLCHANGE0				"Fill Style Change 0"
#define SWFNAME_FILLCHANGE1				"Fill Style Change 1"
#define SWFNAME_LINECHANGE				"Line Style Change"
// SWF Bitmaps
#define SWFNAME_BITMAPFORMAT			"format"
#define SWFNAME_COLORTABLESIZE			"color table size"
#define SWFNAME_DATASIZE				"data size"
// SWF Font & Text
#define SWFNAME_FONTID					"Font ID"
#define SWFNAME_OFFSETTABLE				"Offset Table"
#define SWFNAME_SHAPETABLE				"Shape Table"
#define SWFNAME_CHARACTERID				"Character ID"
#define SWFNAME_FONTNAME				"Font Name"
#define SWFNAME_SHIFTJIS				"ShiftJIS"
#define SWFNAME_ANSI					"ANSI"
#define SWFNAME_ITALIC					"Italic"
#define SWFNAME_BOLD					"Bold"
#define SWFNAME_WIDECHAR				"Wide Character"
#define SWFNAME_CODETABLE				"Code Table"
#define SWFNAME_TEXTSTYLECHANGE			"Text Style Change"
#define SWFNAME_GLYPH					"Glyph"
#define SWFNAME_GLYPHENTRIES			"Glyph Entries"
#define SWFNAME_XOFFSET					"X Offset"
#define SWFNAME_YOFFSET					"Y Offset"
#define SWFNAME_TEXTHEIGHT				"Text Height"
#define SWFNAME_INDEX					"Index"
#define SWFNAME_ADVANCE					"Advance"
#define SWFNAME_TEXTRECORDS				"Text Records"
#define SWFNAME_FONTNAME				"Font Name"
#define SWFNAME_NUMGLYPHS				"Number of Glyphs"
#define SWFNAME_CODETABLE				"Code Table"
#define SWFNAME_ASCENT					"Ascent"
#define SWFNAME_DESCENT					"Descent"
#define SWFNAME_LEADING					"Leading"
#define SWFNAME_VARNAME					"Variable Name"
#define SWFNAME_INITIALTEXT				"Initial Text"
#define SWFNAME_FONTHEIGHT				"Font Height"
#define SWFNAME_TEXTCOLOR				"Text Color"
#define SWFNAME_MAXLENGTH				"Max Length"
#define SWFNAME_LAYOUT					"Layout"
#define SWFNAME_LEFTMARGIN				"Left Margin"
#define SWFNAME_RIGHTMARGIN				"Right Margin"
#define SWFNAME_INDENT					"Indent"
#define SWFNAME_LEADING					"Leading"
#define SWFNAME_WORDWRAP				"Word Wrap"
#define SWFNAME_MULTILINE				"Multi-Line"
#define SWFNAME_PASSWORD				"Password"
#define SWFNAME_READONLY				"Read-Only"
#define SWFNAME_AUTOSIZE				"Auto-Size"
#define SWFNAME_NOSELECT				"No-Select"
#define SWFNAME_BORDER					"Border"
#define SWFNAME_HTML					"HTML"
#define SWFNAME_USEOUTLINE				"Use-Outline"

// Display List
#define SWFNAME_ID						"id"
#define SWFNAME_FRAMECOUNT				"Frame Count"
#define SWFNAME_CHARACTERID				"Character ID"
#define SWFNAME_CLIPDEPTH				"Clip Depth"

// SWF Color
#define SWFNAME_RED						"red"
#define SWFNAME_GREEN					"green"
#define SWFNAME_BLUE					"blue"
#define SWFNAME_ALPHA					"alpha"
// SWF Gradient
#define SWFNAME_NUMCTRLPOINTS			"Number of Control Points"
// SWF Color XForm
#define SWFNAME_ADDRED					"addRed"
#define SWFNAME_ADDGREEN				"addGreen"
#define SWFNAME_ADDBLUE					"addBlue"
#define SWFNAME_ADDALPHA				"addAlpha"
#define SWFNAME_MULTRED					"multRed"
#define SWFNAME_MULTGREEN				"multGreen"
#define SWFNAME_MULTBLUE				"multBlue"
#define SWFNAME_MULTALPHA				"multAlpha"

// Sound
#define SWFNAME_PLAYBACKRATE			"Playback Rate"
#define SWFNAME_PLAYBACKSIZE			"Playback Size"
#define SWFNAME_PLAYBACKTYPE			"Playback Type"
#define SWFNAME_COMPRESSIONTYPE			"Compression Type"
#define SWFNAME_STREAMRATE				"Stream Rate"
#define SWFNAME_STREAMSIZE				"Stream Size"
#define SWFNAME_STREAMTYPE				"Stream Type"
#define SWFNAME_SAMPLECOUNT				"Sample Count"
#define SWFNAME_LATENCYSEEK				"Latency Seek"

#define SWFNAME_5_5KHZ					"5.5 kHz"
#define SWFNAME_11KHZ					"11 kHz"
#define SWFNAME_22KHZ					"22 kHz"
#define SWFNAME_44KHZ					"44 kHz"
#define SWFNAME_UNCOMPRESSED			"uncompressed"
#define SWFNAME_ADPCM					"ADPCM"
#define SWFNAME_MP3						"MP3"
#define SWFNAME_UNCOMPRESSED_LITTLEENDIAN	"uncompressed little-endian"
#define SWFNAME_NELLYMOSER				"Nellymoser"
#define SWFNAME_8BIT					"8-bit"
#define SWFNAME_16BIT					"16-bit"
#define SWFNAME_MONO					"mono"
#define SWFNAME_STEREO					"stereo"

// General
#define SWFNAME_COUNT					"count"
#define SWFNAME_COLOR					"color"
#define SWFNAME_WIDTH					"width"
#define SWFNAME_HEIGHT					"height"
#define SWFNAME_RATIO					"ratio"
#define SWFNAME_DELTAX					"delta X"
#define SWFNAME_DELTAY					"delta Y"
#define SWFNAME_BOUNDS					"Bounds"
#define SWFNAME_MATRIX					"Matrix"
#define SWFNAME_DEPTH					"Depth"
#define SWFNAME_COLORXFORM				"Color Transform"
#define SWFNAME_NAME					"Name"
#define SWFNAME_FLAGS					"Flags"
#define SWFNAME_UNRECOGNIZED			"unrecognized"

/******************************************************************************
 *
 * SWF Display Lists
 *
 *****************************************************************************/
/*
typedef struct SWFPlaceObject {
	short id;
	short depth;
	SWFMatrix matrix;
	SWFColorXForm colorTransform;
} SWFPlaceObject;

typedef struct SWFPlaceObject2 {
	short id;
	short depth;
	SWFMatrix matrix;
	SWFColorXForm colorTransform;
} SWFPlaceObject2;
*/

/******************************************************************************
 *
 * SWF Gradients
 *
 *****************************************************************************/
struct SWFGradientRecord {
	short ratio;
	union {
		RGB color;
		RGBA shape3Color;
	} u;
	struct SWFGradientRecord *next;

	SWFGradientRecord() :
		ratio(0),
		next(NULL)
	{
	}
};

struct SWFGradient {
	short numControlPoints;
	SWFGradientRecord *pRecord;

	SWFGradient() :
		numControlPoints(0),
		pRecord(NULL)
	{
	}
};

/******************************************************************************
 *
 * SWF Shapes
 *
 *****************************************************************************/
enum SWFFillStyleType {
	SWFSolidFill,
	SWFLinearGradientFill,
	SWFRadialGradientFill,
	SWFTiledBitmapFill,
	SWFClippedBitmapFill
};

#define FILL_SOLID				0x00
#define FILL_GRADIENT_LINEAR	0x10
#define FILL_GRADIENT_RADIAL	0x12
#define FILL_BITMAP_TILED		0x40
#define FILL_BITMAP_CLIPPED		0x41

struct SWFFillStyle {
	SWFFillStyleType type;

	RGB color;
	RGBA shape3Color;
	SWFMatrix gradientMatrix;
	SWFGradient gradientFill;
	U16 bitmapID;
	SWFMatrix bitmapMatrix;

	struct SWFFillStyle *next;

	SWFFillStyle() :
		type(SWFSolidFill),
		bitmapID(0),
		next(NULL)
	{
	}
};

struct SWFFillStyleArray {
	U16 count;
	SWFFillStyle *pStyle;

	SWFFillStyleArray() :
		count(0),
		pStyle(NULL)
	{
	}
};

int ReadSWFFillStyle( SWFFile &swfFile, SWFFillStyle &fillStyle );
int ReadSWFFillStyleArray( SWFFile &swfFile, SWFShapeType type, SWFFillStyleArray &fillStyleArray );

struct SWFLineStyle {
	U16 width;
	union {
		RGB color;
		RGBA shape3Color;
	} u;
	struct SWFLineStyle *next;

	SWFLineStyle() :
		width(0),
		next(NULL)
	{
	}
};

struct SWFLineStyleArray {
	U16 count;
	SWFLineStyle *pStyle;

	SWFLineStyleArray() :
		count(0),
		pStyle(NULL)
	{
	}
};

// int ReadSWFLineStyle( SWFFile &swfFile, SWFLineStyle &lineStyle );
// int ReadSWFShape3LineStyle( SWFFile &swfFile, SWFShape3LineStyle &lineStyle );
int ReadSWFLineStyleArray( SWFFile &swfFile, SWFShapeType type, SWFLineStyleArray &lineStyleArray );

// Shape records
//	1. End Shape
//	2. Style Change
//	3. Straight Edge
//	4. Curved Edge

enum SWFShapeRecordType {
	SWFsrtEndShape,
	SWFsrtStyleChange,
	SWFsrtStraightEdge,
	SWFsrtCurvedEdge
};

// Data structure for a linked list of shape records. 
// End of records are terminated by NULL on next field.
struct SWFShapeRecord {
	SWFShapeRecordType type;
	void *pRecord; // A pointer to SWFStyleChange, SWFStraightEdge, or SWFCurvedEdge struct
	struct SWFShapeRecord *next;

	SWFShapeRecord() :
		type(SWFsrtEndShape),
		pRecord(NULL),
		next(NULL)
	{
	}
};

struct SWFStyleChangeRecord {
	bool bNewStyles;
	bool bLineStyleChanged;
	bool bFillStyle0Changed;
	bool bFillStyle1Changed;
	bool bMoveTo;
	long moveDeltaX;
	long moveDeltaY;
	long fillStyle0;
	long fillStyle1;
	long lineStyle;

	SWFFillStyleArray fillStyleArray;
	SWFLineStyleArray lineStyleArray;
	short numFillBits;
	short numLineBits;

	SWFStyleChangeRecord() :
		bNewStyles(false),
		bLineStyleChanged(false),
		bFillStyle0Changed(false),
		bFillStyle1Changed(false),
		bMoveTo(false),
		moveDeltaX(0),
		moveDeltaY(0),
		fillStyle0(0),
		fillStyle1(0),
		lineStyle(0),
		numFillBits(0),
		numLineBits(0)
	{
	}
};

enum SWFStraightLineType {
	SWFGeneralLine,
	SWFVerticalLine,
	SWFHorizontalLine
};

struct SWFStraightEdgeRecord {
	SWFStraightLineType lineType;
	long deltaX;
	long deltaY;
	long horzDeltaX;
	long vertDeltaY;

	SWFStraightEdgeRecord() :
		lineType(SWFGeneralLine),
		deltaX(0),
		deltaY(0),
		horzDeltaX(0),
		vertDeltaY(0)
	{
	}
};

struct SWFCurvedEdgeRecord {
	long controlDeltaX;
	long controlDeltaY;
	long anchorDeltaX;
	long anchorDeltaY;

	SWFCurvedEdgeRecord() :
		controlDeltaX(0),
		controlDeltaY(0),
		anchorDeltaX(0),
		anchorDeltaY(0)
	{
	}
};
/*
typedef struct SWFShape {
	short numFillBits;
	short numLineBits;
	SWFShapeRecord *pRecordList;
} SWFShape;
*/

struct SWFShapeWithStyle {
	SWFFillStyleArray fillStyleArray;
	SWFLineStyleArray lineStyleArray;
	short numFillBits;
	short numLineBits;
	SWFShapeRecord *pRecordList;

	SWFShapeWithStyle() :
		numFillBits(0),
		numLineBits(0),
		pRecordList(NULL)
	{
	}
};

int ReadSWFShapeWithStyle( SWFFile &swfFile, SWFShapeType type, SWFShapeWithStyle &shapeWithStyle );
int ReadSWFShape( SWFFile &swfFile, SWFShapeWithStyle &shape );

enum SWFShapeType {
	SWFShape1,
	SWFShape2,
	SWFShape3
};

struct SWFShapeTag {
	SWFShapeType type;
	U16 id;
	SWFRect bounds;
	SWFShapeWithStyle shapeWithStyle;

	SWFShapeTag() :
		type(SWFShape1),
		id(0)
	{
	}
};

int ReadSWFShapeTag( SWFFile &swfFile, SWFShapeTag &shapeTag );

/******************************************************************************
 *
 * SWF Bitmaps
 *
 *****************************************************************************/
struct SWFBitsLosslessTag {
	U16 id;
	U8 format;
	U16 width;
	U16 height;
	long tableSize;
	long dataSize;

	SWFBitsLosslessTag() :
		id(0),
		format(0),
		width(0),
		height(0),
		tableSize(0),
		dataSize(0)
	{
	}
};

int ReadSWFBitsLosslessTag( SWFFile &swfFile, SWFBitsLosslessTag &bitmapTag, long tagLength );
int ReadSWFBitsLossless2Tag( SWFFile &swfFile, SWFBitsLosslessTag &bitmapTag, long tagLength );

/******************************************************************************
 *
 * SWF Text
 *
 *****************************************************************************/
struct SWFFont {
	U16 fontID;
	long numGlyphs;
	short *pOffsetTable;
	SWFShapeWithStyle *pShapeTable;

	SWFFont() :
		fontID(0),
		numGlyphs(0),
		pOffsetTable(NULL),
		pShapeTable(NULL)
	{
	}
};

int ReadSWFFontTag( SWFFile &swfFile, SWFFont &font, long *glyphCountTable );

#define DF2_HASLAYOUT		0x80
#define DF2_SHIFJIS			0x40
#define DF2_ANSI			0x10
#define DF2_WIDEOFFSETS		0x08
#define DF2_WIDECODES		0x04
#define DF2_ITALIC			0x02
#define DF2_BOLD			0x01

struct SWFKerningRecord {
	U16 code1;
	U16 code2;
	S16 adjustment;

	SWFKerningRecord() :
		code1(0),
		code2(0),
		adjustment(0)
	{
	}
};

struct SWFFont2 {
	U16 fontID;
	bool bHasLayout;
	bool bShiftJIS;
	bool bANSI;
	bool bWideOffsets;
	bool bWideCodes;
	bool bItalic;
	bool bBold;
	SWFLangCode languageCode;
	U8 *szFontName;
	U16 numGlyphs;
	U32 *pFontOffsetTable;
	U32 codeTableOffset;
	SWFShapeWithStyle *pShapeTable;
	U16 *pCodeTable;

	// Layout
	S16 ascent;
	S16 descent;
	S16 leading;
	S16 *pAdvanceTable;
	SWFRect *pBoundsTable;

	// Kerning
	U16 kerningCount;
	SWFKerningRecord *pKerningTable;

	SWFFont2() :
		fontID(0),
		bHasLayout(false),
		bShiftJIS(false),
		bANSI(false),
		bWideOffsets(false),
		bWideCodes(false),
		bItalic(false),
		bBold(false),
		languageCode(0),
		szFontName(NULL),
		numGlyphs(0),
		pFontOffsetTable(NULL),
		codeTableOffset(0),
		pShapeTable(NULL),
		pCodeTable(NULL),
		ascent(0),
		descent(0),
		leading(0),
		pAdvanceTable(NULL),
		pBoundsTable(NULL),
		kerningCount(0),
		pKerningTable(NULL)
	{
	}
};

int ReadSWFFont2Tag( SWFFile &swfFile, SWFFont2 &font, SWFCodeTable **hCodeTableList );

#define FIH_SHIFJIS			0x10
#define FIH_ANSI			0x08
#define FIH_ITALIC			0x04
#define FIH_BOLD			0x02
#define FIH_WIDECODES		0x01

struct SWFFontInfoHeader {
	U16 fontID;
	U8 *szFontName;
	bool bShiftJIS;
	bool bANSI;
	bool bItalic;
	bool bBold;
	bool bWideCodes;

	SWFFontInfoHeader() :
		fontID(0),
		szFontName(NULL),
		bShiftJIS(false),
		bANSI(false),
		bItalic(false),
		bBold(false),
		bWideCodes(false)
	{
	}
};

struct SWFFontInfo {
	SWFFontInfoHeader header;
	U16 *pCodeTable;

	SWFFontInfo() :
		pCodeTable(NULL)
	{
	}
};

int ReadSWFFontInfoTag( SWFFile &swfFile, SWFFontInfo &fontInfo, long *glyphCountTable, SWFCodeTable **hCodeTablList );

struct SWFFontInfo2 {
	SWFFontInfoHeader header;
	SWFLangCode languageCode;
	U16 *pCodeTable;

	SWFFontInfo2() :
		languageCode(0),
		pCodeTable(NULL)
	{
	}
};

int ReadSWFFontInfo2Tag( SWFFile &swfFile, SWFFontInfo2 &fontInfo, long *glyphCountTable, SWFCodeTable **hCodeTablList );

enum SWFTextRecordType {
	SWFTextStyleChange,
	SWFGlyph
};

#define TSC_HASFONT			0x08
#define TSC_HASCOLOR		0x04
#define TSC_HASYOFFSET		0x02
#define TSC_HASXOFFSET		0x01

struct SWFTextStyleChangeRecord {
	bool bHasFont;
	bool bHasColor;
	bool bHasYOffset;
	bool bHasXOffset;
	U16 fontID;
	S16 xOffset;
	S16 yOffset;
	U16 textHeight;
	union {
		RGB textColor;
		RGBA text2Color;
	} u;

	SWFTextStyleChangeRecord() :
		bHasFont(false),
		bHasColor(false),
		bHasYOffset(false),
		bHasXOffset(false),
		fontID(0),
		xOffset(0),
		yOffset(0),
		textHeight(0)
	{
	}
};

struct SWFGlyphEntry {
	long index;
	long advance;
	struct SWFGlyphEntry *next;

	SWFGlyphEntry() :
		index(0),
		advance(0),
		next(NULL)
	{
	}
};

struct SWFGlyphRecord {
	long count;
	SWFGlyphEntry *pEntries;

	SWFGlyphRecord() : 
		count(0),
		pEntries(NULL)
	{
	}
};

struct SWFTextRecordList {
	SWFTextRecordType type;
	void *pRecord;
	struct SWFTextRecordList *next;

	SWFTextRecordList() :
		type(SWFTextStyleChange),
		pRecord(NULL),
		next(NULL)
	{
	}
};

struct SWFText {
	U16 characterID;
	SWFRect bound;
	SWFMatrix matrix;
	U8 numGlyphBits;
	U8 advanceGlyphBits;
	SWFTextRecordList *pRecordList;

	SWFText() :
		characterID(0),
		numGlyphBits(0),
		advanceGlyphBits(0),
		pRecordList(NULL)
	{
	}
};

int ReadSWFTextTag( SWFFile &swfFile, SWFText &text, bool bHasAlpha );

#define ET_HASTEXT		0x80
#define ET_WORDWRAP		0x40
#define ET_MULTILINE	0x20
#define ET_PASSWORD		0x10
#define ET_READONLY		0x08
#define ET_HASTEXTCOLOR 0x04
#define ET_HASMAXLENGTH	0x02
#define ET_HASFONT		0x01

#define ET_AUTOSIZE		0x40
#define ET_HASLAYOUT	0x20
#define ET_NOSELECT		0x10
#define ET_BORDER		0x08
#define ET_HTML			0x02
#define ET_USEOUTLINE	0x01

struct SWFEditText {
	U16 characterID;
	SWFRect bound;
	bool hasText;
	bool wordWrap;
	bool multiLine;
	bool password;
	bool readOnly;
	bool hasTextColor;
	bool hasMaxLength;
	bool hasFont;
	// reserved bit
	bool autoSize;
	bool hasLayout;
	bool noSelect;
	bool border;
	// reserved bit
	bool html;
	bool useOutline;

	// if hasFont
	U16 fontID;
	U16 fontHeight;

	// if hasTextColor
	RGBA textColor;

	// if hasMaxLength
	U16 maxLength;

	// if hasLayout
	U8 align;
	U16 leftMargin;
	U16 rightMargin;
	U16 indent;
	U16 leading;

	char *szVariableName;

	// if hasText
	char *szInitialText;

	SWFEditText() :
		characterID(0),
		hasText(false),
		wordWrap(false),
		multiLine(false),
		password(false),
		readOnly(false),
		hasTextColor(false),
		hasMaxLength(false),
		hasFont(false),
		autoSize(false),
		hasLayout(false),
		noSelect(false),
		border(false),
		html(false),
		useOutline(false),
		fontID(0),
		fontHeight(0),
		maxLength(0),
		align(0),
		leftMargin(0),
		rightMargin(0),
		indent(0),
		leading(0),
		szVariableName(NULL),
		szInitialText(NULL)
	{
	}
};

int ReadSWFEditTextTag( SWFFile &swfFile, SWFEditText &text );

// Support for displaying the text representation in the text tag.
// (ie auto cross referencing between font tag and text tag)
struct SWFCodeTable {
	U16 fontID;
	U16 size;
	U16 *pTable;
	struct SWFCodeTable *next;

	SWFCodeTable() :
		fontID(0),
		size(0),
		pTable(NULL),
		next(NULL)
	{
	}
};

const SWFCodeTable *FindFontCodeTable( U16 fontID, const SWFCodeTable *pCodeTableList );
int ReleaseFontCodeTableList( SWFCodeTable *pCodeTableList );

/******************************************************************************
 *
 * SWF Sound
 *
 *****************************************************************************/
int ReadSWFSoundStreamHeader( SWFFile &swfFile, SWFSoundStreamHead &head );

enum SWFSoundStreamHeadType {
	SWFSoundStreamHead1,
	SWFSoundStreamHead2
};

enum SWFSoundRate {
	SWFSndRate_5_5kHz	= 0,
	SWFSndRate_11kHz	= 1,
	SWFSndRate_22kHz	= 2,
	SWFSndRate_44kHz	= 3
};

enum SWFSoundSize {
	SWFSndSize_8bits	= 0,
	SWFSndSize_16bits	= 1
};

enum SWFSoundType {
	SWFSndType_mono		= 0,
	SWFSndType_stero	= 1
};

enum SWFSoundCompression {
	SWFSndCompress_none			= 0,
	SWFSndCompress_ADPCM		= 1,
	SWFSndCompress_MP3			= 2,
	SWFSndCompress_littleEndian	= 3,
	SWFSndCompress_Nellymoser	= 6
};

struct SWFSoundStreamHead {
	SWFSoundStreamHeadType type;
	SWFSoundRate playbackRate;
	SWFSoundSize playbackSize;
	SWFSoundType playbackType;
	SWFSoundCompression compressionType;
	SWFSoundRate streamRate;
	SWFSoundSize streamSize;
	SWFSoundType streamType;
	U16 streamSampleCount;
	S16 latencySeek;
};

int ReadSWFMP3StreamBlock( SWFFile &swfFile, SWFMP3StreamBlock &block );

struct SWFMP3StreamBlock {
	U16 sampleCount;
};

/******************************************************************************
 *
 * SWF Display List
 *
 *****************************************************************************/
struct SWFSpriteTag {
	U16 id;
	U16 frameCount;

	SWFSpriteTag() : id(0), frameCount(0) {}
};

int ReadSWFSpriteTag( SWFFile &swfFile, SWFSpriteTag &sprite );

#define	PO2_CLIPACTIONS		0x80
#define PO2_CLIPDEPTH		0x40
#define PO2_NAME			0x20
#define PO2_RATIO			0x10
#define PO2_COLORXFORM		0x08
#define PO2_MATRIX			0x04
#define PO2_CHARACTER		0x02
#define PO2_MOVE			0x01

struct SWFPlaceObject2Tag {
	bool bHasClipActions;
	bool bHasClipDepth;
	bool bHasName;
	bool bHasRatio;
	bool bHasColorXform;
	bool bHasMatrix;
	bool bHasCharacter;
	bool bMove;

	U16 depth;
	U16 characterID;
	SWFMatrix matrix;
	SWFColorXForm colorXform;
	U16 ratio;
	char *szName;
	U16 clipDepth;
	// clipActions;

	SWFPlaceObject2Tag() :
		bHasClipActions(false),
		bHasClipDepth(false),
		bHasName(false),
		bHasRatio(false),
		bHasColorXform(false),
		bHasMatrix(false),
		bHasCharacter(false),
		bMove(false),
		depth(0),
		characterID(0),
		ratio(0),
		szName(NULL),
		clipDepth(0)
	{
	}
};

int ReadSWFPlaceObject2Tag( SWFFile &swfFile, SWFPlaceObject2Tag &place );

struct SWFPlaceObjectTag {
	bool bHasColorXform;
	U16 characterID;
	U16 depth;
	SWFMatrix matrix;
	SWFColorXForm colorXform;

	SWFPlaceObjectTag() :
		bHasColorXform(false),
		characterID(0),
		depth(0)
	{
	}
};

int ReadSWFPlaceObjectTag( SWFFile &swfFile, SWFPlaceObjectTag &place );

struct SWFRemoveObjectTag {
	U16 characterID;
	U16 depth;

	SWFRemoveObjectTag() : characterID(0), depth(0) {}
};

struct SWFRemoveObject2Tag {
	U16 depth;

	SWFRemoveObject2Tag() : depth(0) {}
};

int ReadSWFRemoveObjectTag( SWFFile &swfFile, SWFRemoveObjectTag &remove );
int ReadSWFRemoveObject2Tag( SWFFile &swfFile, SWFRemoveObject2Tag &remove );

struct SWFExportAssets {
	U16 characterID;
	char name[256];

	SWFExportAssets() : characterID(0) { memset(name, 0, sizeof(char) * 256); }
};
int ReadSWFExportAssets( SWFFile &swfFile, int &count, SWFExportAssets **exports );

int ReadSWFSetTabIndex( SWFFile &swfFile, int& depth, int& index );


/******************************************************************************
 *
 * SWF ActionScript
 *
 *****************************************************************************/
enum PushValueType {
	SWFPushNone,
	SWFPushString,
	SWFPushFloat,
	SWFPushNull,
	SWFPushUndefined,
	SWFPushRegister,
	SWFPushBoolean,
	SWFPushDouble,
	SWFPushInteger,
	SWFPushConstantByte,
	SWFPushConstantWord
};

struct SWFActionRecord {
	U8 code;
	std::string description;					// misc action-specific description
	std::vector<std::string> theStrings;	// if this action is defineConstPool, the strings
												// if this action is push, the item(s) to be pushed
	std::vector<SWFActionRecord> actions;		// if this action is DefineFunction[2], the actions

	SWFActionRecord() : code(0) {}
};

int ReadSWFActionTag(SWFFile& swfFile, std::vector<SWFActionRecord>& actions, long* spriteID, long& constPoolTotal);

/******************************************************************************
 *
 * SWF File Header
 *
 *****************************************************************************/
struct SWFHeader {
	bool bCompressed;
	int iVersion;
	int iFileLen;
	SWFRect rFrameSize;
	int iFrameRateFraction;
	int iFrameRate;
	int iFrameCount;

	SWFHeader() : 
		bCompressed(false),
		iVersion(0),
		iFileLen(0),
		iFrameRateFraction(0),
		iFrameRate(0),
		iFrameCount(0)
	{
	}
};

struct SWFTagHeader {
	short type;
	long offset;	// from start of SWF
	long length;

	SWFTagHeader() : type(0), offset(0), length(0) {}
};

class SWFFile;
int ReadSWFHeader( SWFFile &swfFile, SWFHeader &fileHeader );
int ReadSWFTagHeader( SWFFile &swfFile, SWFTagHeader &tagHeader, short *pBytesRead = NULL );
int SkipSWFTag( SWFFile &swfFile, long length );


/******************************************************************************
 *
 * SWF File Analysis
 *
 *****************************************************************************/
struct SWFFileAnalysis {
	long fileSize;
	long shape;
	long bitmap;
	long lossless2;
	long jpeg2;
	long text;
	long definetext;
	long definefont2;
	long control;
	long action;
	long initaction;
	long sound;
	long button;
	long display;
	long settab;
	long definesprite;
	long other;
	long shapeCount;
	long bitmapCount;
	long definebitsCount;
	long jpegtablesCount;
	long jpeg2Count;
	long jpeg3Count;
	long losslessCount;
	long lossless2Count;
	long textCount;
	long constPool;	// also part of action total, so DON'T use a percent display
	long definefontCount;
	long definefont2Count;
	long definefontinfoCount;
	long definefontinfo2Count;
	long definetextCount;
	long definetext2Count;
	long edittextCount;
	long controlCount;
	long actionCount;
	long initactionCount;
	long soundCount;
	long buttonCount;
	long displayCount;
	long settabCount;
	long definespriteCount;
	long otherCount;
};


/*
 * SWFFormatContainer class is the root container of a window when the 
 * user decides to analyze a *.swf file. This container contains the
 * multi-column tree view as well as its corresponding data items
 * (TreeData and DataSource). It's the logical glue (or binding)
 * between MCTreeView, TreeData, and DataSource.
 */
class SWFFormatContainer
{
public:
	SWFFormatContainer();
	~SWFFormatContainer();
	int Initialize( HINSTANCE hInstance, HWND hwndParent );

	void Resize( int iWidth, int iHeight );
	void AddRootDispObject( DisplayObject *pDispObj );
	void AddDispObjectList( DispObjectList *pDispObjList );

protected:
	MCTreeView *m_pMCTreeView;
	TreeData *m_pTreeData;
	DataSource *m_pDataSource;
	DispObjectList *m_pDispObjList;
};

#endif // __SWFFORMAT_H__