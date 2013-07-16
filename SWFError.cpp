#include "stdafx.h"
#include "SWFError.h"

void ReportError( long errorCode, long tag, long tagoffset )
{
	const char* msg = "Unknown Error.";
	
	switch( errorCode )
	{
		case SWF_ERR_NOT_SWF:
		{
			msg = ( "Not a valid SWF file" );
		}
		break;

		case SWF_ERR_FILE_READ:
		{
			msg = ( "Error reading SWF file" );
		}
		break;

		case SWF_ERR_FILE_EOF:
		{
			msg = ( "EOF reached!" );
		}
		break;

		case SWF_ERR_FILE_SEEK:
		{
			msg = ( "Cannot seek past EOF" );
		}
		break;

		case SWF_ERR_FILE_TAGLIMIT:
		{
			msg = ( "Cannot set tag limit past EOF" );
		}
		break;

		case SWF_ERR_FILE_SETPOS:
		{
			msg = ( "Cannot set file read position to be less than 0 or greater than file size" );
		}
		break;

		case SWF_ERR_TAGLIMITNOTREACHED:
		{
			msg = ( "Tag limit not reached -- the tag header declares a length that is longer than the apparently-contained data. This is usually not fatal, but is not to-spec." );
		}
		break;

		case SWF_ERR_COMPRESSED_VERSION:
		{
			msg = ( "Compressed SWF file has the wrong version number" );
		}
		break;

		case SWF_ERR_READ_UNFINISHED:
		{
			msg = ( "Did not finish reading SWF file!" );
		}
		break;

		case SWF_ERR_DECOMPRESS_FAILED:
		{
			msg = ( "Could not decompress SWF file!" );
		}
		break;

		case SWF_ERR_UNDEFINED:
		{
			msg = ( "Unrecognized Error!" );
		}
		break;
	}

	char buf[256];
	sprintf(buf, "%s (tag = %d, tag offset = %d)", msg, tag, tagoffset);
	DisplayError(buf);
}

void DisplayError( const char *szError, const char *szTitle /* = NULL */)
{
	char *szDialogTitle = szTitle ? (char*) szTitle : "Inspect SWF";
	MessageBox( NULL, szError, szTitle, MB_OK | MB_ICONEXCLAMATION );
}