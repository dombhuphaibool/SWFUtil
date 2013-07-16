#pragma once

#include "SWFFormat.h"

class SWFFile {

protected:
	bool m_bCompressed;
	long m_lVersion;
	long m_lBufferSize;
	U8 *m_pBuffer;

	long m_lPosition;
	long m_lTagLimit;

public:
	SWFFile();
	~SWFFile();

	int Initialize( const char *szFileName );
	int LoadHeader( SWFHeader &fileHeader );

	int ReadBytes( void *pDest, long numBytes );
	int Seek( long numBytes );
	int IsEOF();

	int SetTagLimit( long tagLimit );
	int AdvanceTagLimit( long delta );
	void InitializeTagLimit();
	void DiscardTagLimit();
	int TagLimitReached();
	void SeekToTagLimit();

	void Reset();

	int GetSize();
	int GetPosition();
	int SetPosition( int newPosition );

	// Use with extreme care... (It's a temporary hack)
	char *GetBufferAtCurrentPos();

protected:
	int Load( FILE *fp );
	int LoadCompressed( FILE *fp );

};

