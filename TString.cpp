#include "stdafx.h"
#include "TString.h"

#define DEFAULT_BUFFER_SIZE		20
#define P_CHAR					char*
#define CHAR_SIZE				sizeof(char)

static TString gNullString;
const TString& TString::Null() { return gNullString; }

void TString::Initialize()
{
	m_lDataSize = 0;
	m_pszData = NULL;
}

TString::TString()
{
	/*
	m_lDataSize = CHAR_SIZE * DEFAULT_BUFFER_SIZE;
	m_pszData = (P_CHAR) malloc( m_lDataSize );
	*/
	Initialize();
}

TString::TString( const TString& str )
{
	unsigned long ulReqLen = str.GetStringLength() + 1;
	Initialize();
	AccomodateBuffer( ulReqLen );
	strncpy( m_pszData, (const char*)str, ulReqLen );
}

TString::TString( const char* psz )
{
	unsigned long ulReqLen = (unsigned long) strlen(psz) + 1;
	Initialize();
	AccomodateBuffer( ulReqLen );
	strncpy( m_pszData, psz, ulReqLen );
}

// unsigned long ranges from 0 to 4,294,967,295
// so a buffer of size 12 is safe...
void TString::AssignFromLong( long lNum )
{
	Initialize();
	AccomodateBuffer( 12 );
	sprintf( m_pszData, "%ld", lNum );
}

// for now assume that float is maxed at
// 4,294,967,295.999
void TString::AssignFromFloat( float fNum )
{
	Initialize();
	AccomodateBuffer( 15 );
	sprintf( m_pszData, "%.2f", fNum );
}

void TString::AssignFromFixed8( long lNum )
{
	Initialize();
	AccomodateBuffer( 18 );	// 8 + decimal + 8 + newline = 18
	long preDecimal = lNum >> 8;
	long postDecimal = lNum & 0xFF;
	sprintf( m_pszData, "%d.%d", preDecimal, postDecimal );
}

void TString::AssignFromFixed16( long lNum )
{
	Initialize();
	AccomodateBuffer( 34 );	// 16 + decimal + 16 + newline = 34
	long preDecimal = lNum >> 16;
	long postDecimal = lNum & 0xFFFF;
	sprintf( m_pszData, "%d.%d", preDecimal, postDecimal );
}

TString::TString( long lNum, ValueType interpretValueAs /* = VALUE_IS_LONG */ )
{
	switch (interpretValueAs)
	{
		case VALUE_IS_LONG:		AssignFromLong( lNum );		break;
		case VALUE_IS_FIXED8:	AssignFromFixed8( lNum );	break;
		case VALUE_IS_FIXED16:	AssignFromFixed16( lNum );	break;
		default:				assert(0);					break;
	}
}

TString::TString( int iNum )
{
	AssignFromLong( (long) iNum );
}

TString::TString( short sNum )
{
	AssignFromLong( (long) sNum );
}

TString::TString( float fNum )
{
	AssignFromFloat( fNum );
}

TString::~TString()
{
	m_lDataSize = 0;
	if ( m_pszData )
		free( m_pszData );
}

bool TString::IsEmpty() const
{
	bool bIsEmpty = false;
	if ( (m_lDataSize == 0) || (m_pszData == NULL) || (m_pszData[0] == 0) )
		bIsEmpty = true;
	return bIsEmpty;
}

void TString::SetEmpty()
{
	if ( m_pszData )
		m_pszData[0] = 0;
}

unsigned long TString::GetStringLength() const
{
	unsigned long ulStrLen = 0;
	if ( m_pszData )
		ulStrLen = (unsigned long) strlen( m_pszData );
	return ulStrLen;
}

void TString::AccomodateBuffer( unsigned long ulNewSize )
{
	if ( ulNewSize > m_lDataSize )
	{
		m_lDataSize = CHAR_SIZE * ulNewSize;

		if ( m_pszData )
		{
			m_pszData = (P_CHAR) realloc( m_pszData, m_lDataSize );
		}
		else
		{
			m_pszData = (P_CHAR) malloc( m_lDataSize );
			m_pszData[0] = 0;
		}
	}
}

/*
 * Assignments
 */
void TString::AssignString( const char* psz )
{
	if ( psz )
	{
		unsigned long ulSourceLen = (unsigned long) strlen( psz );
		AccomodateBuffer( ulSourceLen + 1 );
		strncpy( m_pszData, psz, ulSourceLen + 1 );
	}
	else
	{
		// Only null it out if we have something allocated
		if ( m_pszData )
			m_pszData[0] = 0;
	}
}

TString& TString::operator=( const TString& str )
{
	return TString::operator=( (const char*) str );
}

TString& TString::operator=( const char* psz )
{
	AssignString( psz );
	return *this;
}

TString& TString::operator=( const signed char* psz )
{
	return TString::operator=( (const char*) psz );
}

/*
 * Concatenation
 */
TString& TString::operator+=( const TString& str )
{
	unsigned long ulSourceLen = str.GetStringLength();
	if ( ulSourceLen )
	{
		AccomodateBuffer( GetStringLength() + ulSourceLen + 1 );
		strncat( m_pszData, (const char*) str, ulSourceLen + 1 );
	}

	return *this;
}

TString& TString::operator+=( char letter )
{
	if ( letter )
	{
		unsigned long strLen = GetStringLength();
		AccomodateBuffer( strLen + 1 + 1 );
		m_pszData[strLen] = letter;
		m_pszData[strLen+1] = 0;
	}

	return *this;
}

TString& TString::operator+=( const char *psz )
{
	if ( psz )
	{
		AccomodateBuffer( GetStringLength() + strlen(psz) + 1 );
		strncat( m_pszData, psz, strlen(psz) + 1 );
	}

	return *this;
}

/*
 * Coercion
 */
TString::operator const char*() const
{
	return( (const char*) m_pszData );
}

TString::operator const signed char*() const
{
	return( (const signed char*) m_pszData );
}

/*
 * Comparison
 */
bool operator==( const TString& str1, const TString& str2 )
{
	return( strcmp( (const char*) str1, (const char*) str2 ) == 0 );
}

bool operator==( const TString& str1, const char* psz2 )
{
	return( strcmp( (const char*) str1, psz2 ) == 0 );
}

bool operator==( const TString& str1, const signed char *psz2 )
{
	return( strcmp( (const char*) str1, (const char*) psz2 ) == 0 );
}

bool operator!=( const TString& str1, const TString& str2 )
{
	return( strcmp( (const char*) str1, (const char*) str2 ) != 0 );
}

bool operator!=( const TString& str1, const char* psz2 )
{
	return( strcmp( (const char*) str1, psz2 ) != 0 );
}

bool operator!=( const TString& str1, const signed char *psz2 )
{
	return( strcmp( (const char*) str1, (const char*) psz2 ) != 0 );
}
