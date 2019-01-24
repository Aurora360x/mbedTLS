#pragma once
#include "stdafx.h"
#include "HttpManager.h"

HttpManager::HttpManager()
{
}

VOID HttpManager::SetCookieJar( const std::string& cookieJarPath, const BYTE * encryptionKey /*= NULL*/, DWORD encryptionKeySize /*=0UL*/ )
{
	m_pCookieJar = HTTPCookieJar::CreateCookieJar( "game:\\Downloads" , encryptionKey, encryptionKeySize );
	if( m_pCookieJar == NULL ) {
		HTTPLog( "Failed to create cookie jar:  %s", cookieJarPath.c_str() );
	}
}

VOID HttpManager::AddObserver( IHttpClientObserver& ref )
{
	mObservers.insert( &ref );
}
VOID HttpManager::RemoveObserver( IHttpClientObserver& ref )
{
	mObservers.erase( &ref );
}

HTTPEndpoint * HttpManager::CreateEndpoint( HTTP_REQUEST_TYPE nRequestType, const CHAR * pszUrl, const CHAR * pszTag /*= ""*/)
{
	return HTTPEndpoint::CreateEndpoint(nRequestType, pszUrl, pszTag);
}

HRESULT HttpManager::ExecuteEx( HTTPEndpoint * httpEndpoint, LPHTTP_PROGRESS_ROUTINE lpProgressRoutine, bool manualDelete /*= false*/, LPVOID lpData /*= NULL*/ )
{
	if( httpEndpoint == NULL ) return E_INVALIDARG;

	DWORD ret = PROGRESS_CONTINUE;
	BOOL quiet = FALSE;

	// Allocate memory 
	HRESULT retval = E_FAIL;
	retval = httpEndpoint->AllocateMemory();
	if( retval != S_OK ) {
		printf("Not able to allocate require memory for HTTP request\n" );
		return retval;
	}

	// Successfully allocated memory, next open the request
	if( S_OK == httpEndpoint->OpenRequest() )
	{
		// Notify progress routine
		ret = quiet == FALSE ? lpProgressRoutine( 0UL, 0UL, HTTP_CALLBACK_REQUEST_OPENED, lpData ) : PROGRESS_CONTINUE;
		if( ret == PROGRESS_CANCEL ) { 
			//httpEndpoint->setCancel(); 
		} else if( ret == PROGRESS_QUIET ) { 
			quiet = TRUE; 
		}

		// Loop until processing has completed
		retval = E_PENDING;
		while( retval != S_OK && httpEndpoint->RequestCompleted() == FALSE )
		{
			// Run the DoWork method
			HRESULT hr = httpEndpoint->ProcessState();
			if( hr != S_OK )
			{
				// Set the error code if there was a problem
				//httpEndpoint->SetSuccess(FALSE);
				//httpEndpoint->SetHResult(hr);
				retval = E_FAIL;
			} else {
				// Notify callback
				ret = quiet == FALSE ? lpProgressRoutine( httpEndpoint->GetContentLength(), httpEndpoint->GetBytesRead(), HTTP_CALLBACK_CHUNK_FINISHED, lpData ) : PROGRESS_CONTINUE;
				if( ret == PROGRESS_CANCEL ) {
					httpEndpoint->SetCanceled();
				} else if( ret == PROGRESS_QUIET ) {
					quiet = TRUE;
				}
			}
		}

		ret = quiet == FALSE ? lpProgressRoutine( httpEndpoint->GetContentLength(), httpEndpoint->GetBytesRead(), HTTP_CALLBACK_REQUEST_COMPLETED, lpData ) : PROGRESS_CONTINUE;
		if( ret == PROGRESS_CANCEL ) {
			httpEndpoint->SetCanceled();
		} else if( ret == PROGRESS_QUIET ) {
			quiet = TRUE;
		}

		// Adjust the result to successful
		if( retval == E_PENDING ) retval = ERROR_SUCCESS;
	}
	else
	{
		printf("An error occurred opening the request\n" );
		retval = E_FAIL;
	}

	// Done processing, let's check results and setup flags
	if( retval == S_OK && httpEndpoint->RequestCompleted() == TRUE )
	{
		//httpEndpoint->SetStatus(TRUE);
		//httpEndpoint->SetHResult(S_OK);
	}

	// Finally,we want to close this request and clean up the endpoint
	httpEndpoint->CloseRequest(0);
	ret = quiet == FALSE ? lpProgressRoutine( httpEndpoint->GetContentLength(), httpEndpoint->GetBytesRead(), HTTP_CALLBACK_REQUEST_CLOSED, lpData ) : PROGRESS_CONTINUE;
	if( ret == PROGRESS_CANCEL ) {
		httpEndpoint->SetCanceled();
	} else if( ret == PROGRESS_QUIET ) {
		quiet = TRUE;
	}

	// Next we notify observers that the download is completed
	for( ObserverItem::const_iterator itr = mObservers.begin(); itr != mObservers.end(); ++itr )
	{
		IHttpClientObserver * pObserver = (*itr);
		if( pObserver == NULL ) continue;

		// Call download complete
		pObserver->DownloadComplete( httpEndpoint->GetResponseInfo() );
	}	

	// Clean up the end point
	if( manualDelete == false ) {
		delete httpEndpoint;
		httpEndpoint = NULL;
	}

	// Return sucessfully
	return S_OK;
}

HRESULT HttpManager::Execute( HTTPEndpoint * httpEndpoint, bool manualDelete /*= false*/, HTTPCookieJarPtr customCookieJar /*= NULL*/ )
{
	// Check argument validity
	if( httpEndpoint == NULL ) return E_INVALIDARG;

	// Set our cookie jar
	if( customCookieJar != NULL ) {
		httpEndpoint->SetCookieJar( customCookieJar );
	} else {
		httpEndpoint->SetCookieJar( m_pCookieJar );
	}


	// Allocate memory 
	DWORD retval = httpEndpoint->AllocateMemory();
	if( retval == HTTP_ERR_OK )
	{
		// Open our request
		retval = httpEndpoint->OpenRequest();
		if( retval == HTTP_ERR_OK )
		{
			// Pump the httpEndpoint work method until completion or error
			while( httpEndpoint->RequestCompleted() == FALSE )
			{
				// Run the DoWork() method
				retval = httpEndpoint->ProcessState();
				if( retval != HTTP_ERR_OK ) {
					#if HTTP_DEBUG_ERROR
					  HTTPLog( "Failed to execute endpoint state machine" );
					#endif
				}
			}

			// Request completed, so let's clean up
			DWORD dwReason = retval == HTTP_ERR_OK ? 0 : 1;
			if( httpEndpoint->CloseRequest( dwReason ) != HTTP_ERR_OK ) {
				#if HTTP_DEBUG_ERROR
				  HTTPLog( "The request failed to close causing potential memory leak.  CloseRequest reason = %d", dwReason );
				#endif
			}

		}	// End of OpenRequest
		else 
		{

		}

		// Free any allocated memory
		httpEndpoint->FreeMemory();

	} // End of AllocateMemory

	// Check the status code for any redirects
	#if defined(HTTPMANAGER_ENABLE_REDIRECTS)
	DWORD statusCode = httpEndpoint->GetResponseInfo()->GetStatusCode();
	if( statusCode == HTTP_STATUS_REDIRECT )
	{
		if( httpEndpoint->RedirectEndpoint() == TRUE )
		{
			return Execute( httpEndpoint, manualDelete );
		}
	}
	else 
	{
	#endif
		// At this point, we are done with network things and ready to process the result
		for( ObserverItem::const_iterator itr = mObservers.begin(); itr != mObservers.end(); ++itr )
		{
			IHttpClientObserver * pObserver = (*itr);
			if( pObserver == NULL ) continue;

			// Call download complete
			pObserver->DownloadComplete( httpEndpoint->GetResponseInfo() );
		}
	#if defined(HTTPMANAGER_ENABLE_REDIRECTS)
	}
	#endif

	// Clean up the end point
	if( manualDelete == false ) {
		delete httpEndpoint;
		httpEndpoint = NULL;
	}

	// Return sucessfully
	return S_OK;
}
