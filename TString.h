#pragma once

class TString
{
private:
	unsigned long m_lDataSize;
	char *m_pszData;

protected:
	void AssignString( const char *psz );
	void AccomodateBuffer( unsigned long lNewSize );

public:
	enum ValueType
	{
		VALUE_IS_LONG		= 0,
		VALUE_IS_FIXED8		= 1,	// value is 8.8 - First 8 bits is pre decimal, and last 8 is post decimal
		VALUE_IS_FIXED16	= 2		// value is 16.16 - First 16 bits is pre decimal, and last 16 is post decimal
	};

public:
	TString();
	TString( const TString& );
	TString( const char* );
	TString( long lNum, ValueType interpretValueAs = VALUE_IS_LONG );
	TString( int iNum );
	TString( short sNum );
	TString( float fNum );

	~TString();

	void Initialize();
	void AssignFromLong( long lNum );
	void AssignFromFloat( float fNum );
	void AssignFromFixed8( long lNum );
	void AssignFromFixed16( long lNum );

	// Null support
	static const TString& Null();
	bool IsEmpty() const;
	void SetEmpty();

	// Assignments
	TString& operator=( const TString& );
	TString& operator=( const char* );
	TString& operator=( const signed char* );

	// Concatenation
	TString& operator+=( const TString& );
	TString& operator+=( char letter );
	TString& operator+=( const char* );

	// Coercion
	operator const char*() const;
	operator const signed char*() const;

	// Character access
	// TODO:	char operator[]( int i ) const;

	// Comparison
	friend bool operator==( const TString& str1, const TString& str2 );
	friend bool operator==( const TString& str1, const char* psz2 );
	friend bool operator==( const TString& str1, const signed char *psz2 );

	friend bool operator!=( const TString& str1, const TString& str2 );
	friend bool operator!=( const TString& str1, const char* psz2 );
	friend bool operator!=( const TString& str1, const signed char *psz2 );

	// Access
	inline unsigned long GetBufferSize() { return m_lDataSize; }
	unsigned long GetStringLength() const;
};

