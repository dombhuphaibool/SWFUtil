#include "stdafx.h"
#include "SWFFile.h"
#include "SWFError.h"
#include "zlib.h"

#define NO_SWF_ERROR	(iSuccess == SWF_ERR_NONE)

SWFFile::SWFFile()
{
	m_bCompressed = false;
	m_lVersion = 0;
	m_lBufferSize = 0;
	m_pBuffer = NULL;
	m_lPosition = 0;
	m_lTagLimit = 0;
}

SWFFile::~SWFFile()
{
	if ( m_pBuffer )
		delete[] m_pBuffer;
}

int SWFFile::LoadHeader( SWFHeader &fileHeader )
{
	fileHeader.bCompressed = m_bCompressed;
	fileHeader.iVersion = m_lVersion;
	fileHeader.iFileLen = m_lBufferSize;

	return SWF_ERR_NONE;
}

int SWFFile::Initialize( const char *szFileName )
{
	int iSuccess = SWF_ERR_NONE;

	FILE *fp = fopen( szFileName, "rb" );
	if ( fp )
	{
		// Verify SWF tag (valid SWF file)
		U8 pHeader[4];
		if ( fread(pHeader, U8_SIZE, 4, fp) )
		{
			if ( ((pHeader[0] == 'F') || (pHeader[0] == 'C')) &&
				  (pHeader[1] == 'W') &&
				  (pHeader[2] == 'S') )
			{
				m_bCompressed = (pHeader[0] == 'C');
				m_lVersion = (int) pHeader[3];

				// Compression is only permitted for Flash Version 6 or greater...
				if ( m_bCompressed && (m_lVersion < 6) )
					iSuccess = SWF_ERR_COMPRESSED_VERSION;
			}
			else
			{
				iSuccess = SWF_ERR_NOT_SWF;
			}
		}
		else
		{
			iSuccess = SWF_ERR_FILE_READ;
		}

		if ( NO_SWF_ERROR )
		{
			// Get the file length
			U32 ulFileLen;		
			if ( fread(&ulFileLen, U32_SIZE, 1, fp) )
			{
				m_pBuffer = new U8[ulFileLen];
				if ( m_pBuffer )
				{
					m_lBufferSize = ulFileLen;
					m_lTagLimit = m_lBufferSize;
					memcpy( m_pBuffer, pHeader, 4 );
					memcpy( &m_pBuffer[4], &ulFileLen, 4 );
					m_lPosition += 8;

					// If the file is compressed, then everything from this point on
					// needs to be uncompressed...
					iSuccess = m_bCompressed ? LoadCompressed(fp) : Load(fp);
				}
				else
				{
					iSuccess = SWF_ERR_UNDEFINED;
				}
			}
			else
			{
				iSuccess = SWF_ERR_FILE_READ;
			}
		}
			
		fclose( fp );
	}
	
	return iSuccess;
}

int SWFFile::Load( FILE *fp )
{
	int iSuccess = SWF_ERR_NONE;
	int iRead = (int) fread( &m_pBuffer[8], m_lBufferSize-8, 1, fp );
	if ( !iRead )
		iSuccess = SWF_ERR_FILE_READ;

	return iSuccess;
}

int SWFFile::LoadCompressed( FILE *fp )
{
	int iSuccess = SWF_ERR_NONE;

	// Initialize the decompress buffer
	U8 *pDecompressBuffer = new U8[m_lBufferSize-8];
	int iRead = (int) fread( pDecompressBuffer, 1, m_lBufferSize-8, fp );

	z_stream zlibStream;
	zlibStream.next_in = (Bytef*) pDecompressBuffer;
	zlibStream.avail_in = (uInt) iRead;
	zlibStream.total_out = 0;
	zlibStream.zalloc = (alloc_func) 0;
	zlibStream.zfree = (free_func) 0;
	zlibStream.next_out = (Bytef*) &m_pBuffer[8];
	zlibStream.avail_out = m_lBufferSize-8;

	if ( NO_SWF_ERROR )
		iSuccess = (inflateInit(&zlibStream) == Z_OK) ? SWF_ERR_NONE : SWF_ERR_UNDEFINED;

	if ( NO_SWF_ERROR )
	{
		int err = inflate( &zlibStream, Z_FINISH );
		if ( err != Z_STREAM_END )
			iSuccess = SWF_ERR_DECOMPRESS_FAILED;

		inflateEnd( &zlibStream );
	}

	if ( pDecompressBuffer )
		delete[] pDecompressBuffer;

	return iSuccess;
}

int SWFFile::ReadBytes( void *pDest, long numBytes )
{
	int iSuccess = SWF_ERR_NONE;

	// First check to see if we violated the tag limit
	assert( m_lPosition+numBytes <= m_lTagLimit );
	if ( m_lPosition+numBytes > m_lTagLimit )
	{
		iSuccess = SWF_ERR_TAGLIMIT_VIOLATED;
		m_lPosition = m_lTagLimit; // Advance the read position to the next tag...
	}

	if ( NO_SWF_ERROR )
	{
		assert( m_lPosition+numBytes <= m_lBufferSize );
		if ( m_lPosition+numBytes <= m_lBufferSize )
		{
			memcpy( pDest, &m_pBuffer[m_lPosition], numBytes );
			m_lPosition += numBytes;
		}
		else
		{
			iSuccess = SWF_ERR_FILE_EOF;
		}
	}

	return iSuccess;
}

int SWFFile::Seek( long numBytes )
{
	int iSuccess = SWF_ERR_NONE;

	if ( m_lPosition + numBytes > m_lBufferSize )
		iSuccess = SWF_ERR_FILE_SEEK;
	else
		m_lPosition += numBytes;

	// Check to see if we violated the tag limit
	// If we seek pass the tag limit. Reset the tag limit to be
	// at the same location as the current read position
	assert( m_lPosition <= m_lTagLimit );
	if ( m_lPosition > m_lTagLimit )
		m_lTagLimit = m_lPosition;

	return iSuccess;
}

int SWFFile::IsEOF()
{
	return( m_lPosition == m_lBufferSize );
}

void SWFFile::InitializeTagLimit()
{
	m_lTagLimit = m_lPosition;
}

int SWFFile::SetTagLimit( long tagLimit )
{
	int iSuccess = SWF_ERR_NONE;

	if ( tagLimit <= m_lBufferSize )
		m_lTagLimit = tagLimit;
	else
		iSuccess = SWF_ERR_FILE_TAGLIMIT;

	return iSuccess;
}

int SWFFile::AdvanceTagLimit( long delta )
{
	int iSuccess = SWF_ERR_NONE;

	if ( m_lTagLimit+delta <= m_lBufferSize )
		m_lTagLimit += delta;
	else
		iSuccess = SWF_ERR_FILE_TAGLIMIT;

	return iSuccess;
}

void SWFFile::Reset()
{
	m_lPosition = 0;
	m_lTagLimit = m_lBufferSize;
}

void SWFFile::DiscardTagLimit()
{
	m_lTagLimit = m_lBufferSize;
}

int SWFFile::TagLimitReached()
{
	return( m_lPosition == m_lTagLimit );
}

void SWFFile::SeekToTagLimit()
{
	m_lPosition = m_lTagLimit;
}

int SWFFile::GetSize()
{ 
	return m_lBufferSize;
}

int SWFFile::GetPosition()
{
	return m_lPosition;
}

int SWFFile::SetPosition( int newPosition )
{
	int iSuccess = SWF_ERR_NONE;

	if ( (newPosition > m_lBufferSize) || (newPosition < 0) )
		iSuccess = SWF_ERR_FILE_SETPOS;
	else
		m_lPosition = newPosition;

	// Check to see if we violated the tag limit
	// If we seek pass the tag limit. Reset the tag limit to be
	// at the same location as the current read position
	assert( m_lPosition <= m_lTagLimit );
	if ( m_lPosition > m_lTagLimit )
		m_lTagLimit = m_lPosition;

	return iSuccess;
}

char *SWFFile::GetBufferAtCurrentPos()
{
	return( (char *) &m_pBuffer[m_lPosition] );
}