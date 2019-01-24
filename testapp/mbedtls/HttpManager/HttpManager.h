#pragma once
#include "HttpClient/HTTPEndpoint.h"
#include "HttpClient/HTTPCookieJar.h"

//#define HTTPMANAGER_ENABLE_REDIRECTS	

#define HTTP_CALLBACK_REQUEST_OPENED	1
#define HTTP_CALLBACK_CHUNK_FINISHED	2
#define HTTP_CALLBACK_REQUEST_COMPLETED	3
#define HTTP_CALLBACK_REQUEST_CANCELED  4
#define HTTP_CALLBACK_REQUEST_CLOSED	5

typedef DWORD (*LPHTTP_PROGRESS_ROUTINE)(DWORD TotalFileSize, DWORD TotalBytesTransferred, DWORD dwCallbackReason, LPVOID lpData );


class HttpManager
{
private:
	std::set<IHttpClientObserver * const> mObservers;
	typedef std::set<IHttpClientObserver * const> ObserverItem;

public:
	HRESULT Execute( HTTPEndpoint * pEndpoint, bool manualDelete = false, HTTPCookieJarPtr customCookieJar = NULL );
	HRESULT ExecuteEx( HTTPEndpoint * pEndpoint, LPHTTP_PROGRESS_ROUTINE lpProgressRoutine, bool manualDelete = false, LPVOID lpData = NULL );
	HTTPEndpoint * CreateEndpoint( HTTP_REQUEST_TYPE nRequestType, const CHAR * pszUrl, const CHAR * pszTag = "" );

	VOID AddObserver( IHttpClientObserver& ref );
	VOID RemoveObserver( IHttpClientObserver& ref );

	HttpManager( VOID );

	VOID DumpInfo( VOID )
	{
		if( m_pCookieJar ) {
			printf( "Cookies Cached:  %d\n", m_pCookieJar->GetCookieCount() );
		}
	}

	VOID SetCookieJar( const std::string& cookieJarPath, const BYTE * encryptionKey = NULL, DWORD encryptionKeySize = 0UL );

private:

	// Poitner to our active cookiejar
	HTTPCookieJarPtr m_pCookieJar;
};
