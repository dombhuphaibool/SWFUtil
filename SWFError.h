#pragma once

/*
 * Error codes and error routine prototypes
 */
#define SWF_ERR_NONE					0
#define SWF_ERR_NOT_SWF					-1			// Not a SWF file (wrong header, i.e. does not contain SWF or SWC
#define SWF_ERR_FILE_READ				-9
#define SWF_ERR_FILE_SEEK				-11
#define SWF_ERR_FILE_EOF				-13
#define SWF_ERR_FILE_TAGLIMIT			-15
#define SWF_ERR_FILE_SETPOS				-17
#define SWF_ERR_COMPRESSED_VERSION		-21			// Compressed SWF file has to be version 6 or greater
#define SWF_ERR_READ_UNFINISHED			-23
#define SWF_ERR_TAGLIMIT_VIOLATED		-25
#define SWF_ERR_TAGLIMITNOTREACHED		-27
#define SWF_ERR_DECOMPRESS_FAILED		-99
#define SWF_ERR_UNDEFINED				-9999		// General non-descriptive error


void ReportError( long errorCode, long tag = -1, long tagoffset = 0 );
void DisplayError( const char *szError, const char *szTitle = NULL );

