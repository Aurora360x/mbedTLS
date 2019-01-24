#pragma once
#include "Base/TCPStream.h"

class CHTTPHeader
{
private:
	DWORD mStatusCode;
	std::string mHttpVersion;
	std::string mReasonPhrase;
	std::multimap<std::string, std::string> mHeaderEntries;

public:
	VOID Reset( VOID );

	CHTTPHeader( VOID );
	//CHTTPHeader( mbedtls_ssl_context * ssl );
	BOOL ReceiveHeader( TCPStream * stream );
	DWORD GetStatusCode( void ) const;
	const std::string& GetHttpVersion( void ) const;
	const std::string& GetReasonPhrase( void ) const;
	const std::string GetHeaderValue( const std::string& name ) const;
	DWORD GetHeaderValues( const std::string& name, std::vector<std::string> * values ) const;
	DWORD GetHeaderCount( void ) const;
	VOID DumpHeaderToLog( void ) const;
	VOID DumpHeaderEntriesToLog( void ) const;
	DWORD GetContentLength( void ) const;

	BOOL IsChunked( void ) const;
};