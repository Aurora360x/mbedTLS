#pragma once

#include "stdafx.h"
#include <string>
#include <cctype>
#include <algorithm>
#include "HTTPTypes.h"
#include "HTTPHeader.h"

static const std::string gszEmptyString = "";

CHTTPHeader::CHTTPHeader( VOID )
{
	Reset();
}

VOID CHTTPHeader::Reset( VOID )
{
	mStatusCode = 0;
	mHttpVersion = "";
	mReasonPhrase = "";
	mHeaderEntries.clear();
}

BOOL CHTTPHeader::ReceiveHeader( TCPStream * stream )
{
	if( stream == NULL ) return FALSE;

	//if( stream.IsConnected() == FALSE ) return FALSE;

	std::string statusLine = "";
	statusLine.reserve(1024);

	// First let's read our status line
	bool readingStatus = true;
	while( readingStatus )
	{
		CHAR data = 0;
		if( stream->Receive( (BYTE*)&data, 1 ) != 1 ) return FALSE;

		if( data == '\r' ) continue;
		else if( data == '\n' ) {
			break; // line complete
		}
		else 
		{
			// No line terminators, so append character to string
			statusLine.append(1, data);
		}
	}

	// Do status line parsing
	size_t verPos = statusLine.find( " ", 0 );
	size_t statusPos = statusLine.find( " ", verPos + 1 );
	mHttpVersion = statusLine.substr(0, verPos);
	std::string statusValue = statusLine.substr( verPos + 1, statusPos - verPos - 1 );
	mStatusCode = strtol( statusValue.c_str(), NULL, 10 );
	mReasonPhrase = statusLine.substr(statusPos + 1);

	// Start reading in Headers
	int newlines = 0;
	statusLine.clear();
	while( newlines != 2 )
	{
		CHAR data = 0;
		if( stream->Receive( (BYTE*)&data, 1) != 1 ) return FALSE;

		if( data == '\r' ) continue;
		else if( data == '\n' ) {
			newlines++;

			// If we have the empty line, then we skip
			if( statusLine.length() == 0 ) continue;

			// Parse this header entry
			size_t colonPos = statusLine.find_first_of( ":" );
			std::string headerName = TrimStr(statusLine.substr( 0, colonPos ), " ", true, true );
			std::string headerValue = TrimStr(statusLine.substr( colonPos + 1, statusLine.length() - colonPos - 1 ), " ", true, true);

			mHeaderEntries.insert( std::make_pair(headerName, headerValue ) );
			statusLine.clear();
			continue;
		}
		else 
		{
			// No line terminators, so append character to string
			newlines = 0;
			statusLine.append(1, data);
		}
	}


	#if HTTP_DEBUG_VERBOSE
	  DumpHeaderToLog();
	#endif
	return TRUE;
}

const std::string CHTTPHeader::GetHeaderValue( const std::string& name ) const
{
	std::vector<std::string> values;
	if( GetHeaderValues( name, &values ) == 0 ) {
		return gszEmptyString;
	}

	// Return the first string in the vector
	return values[0];
}

DWORD CHTTPHeader::GetHeaderValues( const std::string& name, std::vector<std::string> * values ) const
{
	// Check if this entry is in or hear map first
	if( mHeaderEntries.count( name ) == 0 ) return 0;

	// Retrieve the value at the specified header 
	DWORD valueCounter = 0UL;
	typedef std::multimap<std::string, std::string> _headerMap;
	std::pair<_headerMap::const_iterator, _headerMap::const_iterator> ret = mHeaderEntries.equal_range(name);
	for( _headerMap::const_iterator itr = ret.first; itr != ret.second; ++itr )
	{
		if( values ) {
			values->push_back( itr->second );
		}
		valueCounter++;
	}

	return valueCounter;
}

DWORD CHTTPHeader::GetStatusCode( void ) const { return mStatusCode; }
const std::string& CHTTPHeader::GetHttpVersion( void ) const { return mHttpVersion; }
const std::string& CHTTPHeader::GetReasonPhrase( void ) const { return mReasonPhrase; }

BOOL CHTTPHeader::IsChunked( void ) const
{
	std::vector<std::string> headerVal;
	if( GetHeaderValues( "Transfer-Encoding", &headerVal ) == 0 ) return FALSE;
	return strcmpi( headerVal[0].c_str(), "chunked" ) == 0 ? TRUE : FALSE;
}

DWORD CHTTPHeader::GetContentLength( void ) const
{
	std::vector<std::string> headerVal;
	if( GetHeaderValues( "Content-Length", &headerVal ) == 0 ) return 0UL;

	// return the converted value
	return strtol( headerVal[0].c_str(), NULL, 10 );
}

VOID CHTTPHeader::DumpHeaderToLog( void ) const 
{
	printf("HTTP Status Line:\n" );
	printf("Version String : %s\n", mHttpVersion.c_str() );
	printf("Status Code : %d\n", mStatusCode );
	printf("Reason Phrase : %s\n", mReasonPhrase.c_str() );
	printf("\n");
	printf("HTTP Header Entries:\n" );
	DumpHeaderEntriesToLog();
}

VOID CHTTPHeader::DumpHeaderEntriesToLog( void ) const
{
	typedef std::multimap<std::string, std::string>::const_iterator mapiter;

	
	int count = 0;
	for( mapiter itr = mHeaderEntries.cbegin(); itr != mHeaderEntries.cend(); ++itr )
	{
		const std::string& name = itr->first;
		const std::string& value = itr->second;
		count++;
		printf("%d) %s : %s\n", count, name.c_str(), value.c_str() );
	}
}

DWORD CHTTPHeader::GetHeaderCount( void ) const
{
	return mHeaderEntries.size();
}