#pragma once
#include "stdafx.h"
#include "HTTPEndpoint.h"

// Define to signify that xkelib.h exists
#define HAVE_XKELIB_H		1

// Statically Defined Strings for conversions
static const std::string gszEmptyString = "";
static const CHAR * gszHttpVerbList[HTTP_REQUEST_TYPE_COUNT] = { 
	"GET", "POST", "HEAD", "PUT", "OPTIONS", "PATCH", "DELETE" 
};

// Endpoint Object Factory Method
HTTPEndpoint * HTTPEndpoint::CreateEndpoint(HTTP_REQUEST_TYPE nRequestType, const CHAR * pszUrl, const CHAR * pszTag /*= ""*/) {
	// Construct a new Endpoint and return a pointer to the object
	return new HTTPEndpoint(nRequestType, pszUrl, pszTag);
}
BOOL HTTPEndpoint::RedirectEndpoint()
{
	return FALSE;
	const CHTTPHeader& header = m_httpResponse->GetResponseHeader();
	if( header.GetStatusCode() != 302 ) return FALSE;

	std::string location = "Test"; //header.GetHeaderValue("Location");
	if( location.empty() ) return FALSE;

	// Build our new URL
	std::string tag = m_httpResponse->GetTag();
	std::string newUrl = m_urlComponents.GetBaseUrl() + location;

	m_httpResponse->ResetValues();

	// First we need to reset our response values
	return TRUE;
}

// Request Header Methods
VOID HTTPEndpoint::AddRequestHeader(const std::string& key, const std::string& value ) { 
	m_httpRequestHeaders.insert( std::make_pair( key, value ) ); 
}						
VOID HTTPEndpoint::RemoveRequestHeader( const std::string& key ) { 
	m_httpRequestHeaders.erase( key ); 
}
BOOL HTTPEndpoint::RequestHeaderExists( const std::string& key ) const { 
	return m_httpRequestHeaders.count(key) > 0 ? TRUE : FALSE;
}
const std::string& HTTPEndpoint::GetRequestHeaderValue( const std::string& key ) const { 
	return (m_httpRequestHeaders.count(key) > 0 ) ? m_httpRequestHeaders.at(key) : gszEmptyString;
}

// PostVar Methods
VOID HTTPEndpoint::AddFormData( const std::string& key, const std::string& value ) { 
	m_httpFormData.insert( std::make_pair( key, value ) ); 
}						
VOID HTTPEndpoint::RemoveFormData( const std::string& key ) { 
	m_httpFormData.erase( key ); 
}
BOOL HTTPEndpoint::FormDataExists( const std::string& key ) const { 
	return m_httpFormData.count(key) > 0 ? TRUE : FALSE;
}
const std::string& HTTPEndpoint::GetFormDataValue( const std::string& key ) const { 
	return (m_httpFormData.count(key) > 0 ) ? m_httpFormData.at(key) : gszEmptyString;
}

// Cookie Methods
const std::weak_ptr<HTTPCookieJar> HTTPEndpoint::SetCookieJar( const std::weak_ptr<HTTPCookieJar>& cookieJar ) {
	std::weak_ptr<HTTPCookieJar> oldCookieJar = m_httpCookieJar;
	m_httpCookieJar = cookieJar;
	return oldCookieJar;
}

// Endpoint Configuration
VOID HTTPEndpoint::SetMD5HashVerification( BOOL verify ) {
	State.CalculateMD5 = verify;
}
DWORD HTTPEndpoint::SetInputFile( const std::string& filePath )
{
	if( m_httpResponse->mInputData.Mode != HTTP_INPUTMODE_NONE || 
		m_httpResponse->mInputData.DataBuffer != NULL || 
		m_httpResponse->mInputData.DataBufferSize > 0 || 
		m_httpResponse->mInputData.FilePath.empty() == false ) 
	{
		#if HTTP_DEBUG_WARNING
		  HTTPLog( "An input buffer or file was previously set.  This data will be replaced by %s", filePath.c_str() );
		#endif
	}

	// Verify the integrity of the supplied path
	if( filePath.length() > HTTP_OUTPUTPATH_MAXLEN || filePath.empty() == true ) {
		#if HTTP_DEBUG_ERROR
			HTTPLog( "Output file has an invalid path length of %d.  Unable to proceed.", filePath.length() );
		#endif
		return HTTP_ERR_INVALIDARG;
	}

	// Verif that the path supplied actually exists and grab file information
	HANDLE hFile = CreateFile( filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( hFile == INVALID_HANDLE_VALUE ) {
		#if HTTP_DEBUG_ERROR
			HTTPLog( "The file path %s is unable to be opened. Possibly not existing.", filePath.c_str() );
		#endif
		return HTTP_ERR_INVALIDARG;
	}

	// Determine the filesize
	DWORD fileSize = GetFileSize( hFile, NULL );

	// Close the file
	CloseHandle( hFile );
	hFile = INVALID_HANDLE_VALUE;

	// Now, let's store our file information
	m_httpResponse->mInputData.FilePath = filePath;
	m_httpResponse->mInputData.FileSize = fileSize;
	m_httpResponse->mInputData.DataBuffer = NULL;
	m_httpResponse->mInputData.DataBufferSize = 0UL;
	m_httpResponse->mInputData.DataBufferIsPersistent = FALSE;
	
	// Set our input mode
	m_httpResponse->mInputData.Mode = HTTP_INPUTMODE_FILE;

	// Return successfully
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::SetInputBuffer( const std::string& data ) {
	return SetInputBuffer( (const BYTE *)data.c_str(), data.length(), TRUE );
}
DWORD HTTPEndpoint::SetInputBuffer( const CMemoryBuffer& buffer, BOOL mustCopy /*= FALSE*/ ) {
	return SetInputBuffer( &buffer[0], buffer.length(), mustCopy );
}
DWORD HTTPEndpoint::SetInputBuffer( const BYTE * buffer, DWORD size, BOOL mustCopy /*= TRUE*/ )
{
	if( m_httpResponse->mInputData.Mode != HTTP_INPUTMODE_NONE || 
		m_httpResponse->mInputData.DataBuffer != NULL || 
		m_httpResponse->mInputData.DataBufferSize > 0 || 
		m_httpResponse->mInputData.FilePath.empty() == false ) 
	{
		#if HTTP_DEBUG_WARNING
		  HTTPLog( "An input buffer or file was previously set.  This data will be replaced by supplied data." );
		#endif
	}

	// Verify the integrity of the supplied path
	if( buffer == NULL || size == 0 ) {
		#if HTTP_DEBUG_ERROR
			HTTPLog( "Input data has invalid parameters.  Unable to proceed." );
		#endif
		return HTTP_ERR_INVALIDARG;
	}

	// Now, let's store our file information
	m_httpResponse->mInputData.FilePath = "";
	m_httpResponse->mInputData.FileSize = 0UL;	
	m_httpResponse->mInputData.DataBufferSize = size;
	m_httpResponse->mInputData.DataBufferIsPersistent = mustCopy == TRUE ? FALSE : TRUE;
	
	if( mustCopy == TRUE )
	{
		// Because the memory is not persistent, we need to create copies of it
		m_httpResponse->mInputData.DataBuffer = (BYTE*)malloc(size);
		if( m_httpResponse->mInputData.DataBuffer == NULL ) {
			#if HTTP_DEBUG_ERROR
				HTTPLog( "Input data was unable to be copied due to low memory.  Unable to proceed." );
			#endif
			return HTTP_ERR_INVALIDARG;
		}
		memcpy( m_httpResponse->mInputData.DataBuffer, buffer, size );
	}
	else 
	{
		// Since hte memory is persistent, we cna just store pointers to it for later use
		m_httpResponse->mInputData.DataBuffer = (BYTE*)buffer;
	}
	
	// Set our input mode
	m_httpResponse->mInputData.Mode = HTTP_INPUTMODE_MEMORY;

	// Return successfully
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::SetOutputMode( HTTP_OUTPUTMODE outputMode, const std::string& filePath /*= ""*/ )
{
	DWORD ret = HTTP_ERR_OK;
	if( outputMode == HTTP_OUTPUTMODE_FILE ) 
	{
		// The path specified is too long for the supported file system
		if( filePath.length() > HTTP_OUTPUTPATH_MAXLEN || filePath.empty() == true ) {
			#if HTTP_DEBUG_WARNING
			  HTTPLog( "Output file has an invalid path length of %d.  Unable to save file on system.", filePath.length() );
			#endif
			return HTTP_ERR_INVALIDARG;
		}

		// Set the output mode and filepath for use
		m_httpResponse->mOutputMode = outputMode;
		m_httpResponse->mOutputFilePath = filePath;
		m_httpResponse->mOutputFileSize = 0UL;
	} 
	else if( outputMode == HTTP_OUTPUTMODE_MEMORY ) 
	{
		// Set the output mode and filepath for use
		m_httpResponse->mOutputMode = outputMode;
		m_httpResponse->mOutputFilePath = "";
		m_httpResponse->mOutputFileSize = 0UL;
	} 
	else 
	{
		// Unrecognized OutputMode
		ret = HTTP_ERR_INVALIDARG;
		#if HTTP_DEBUG_WARNING
		  HTTPLog( "OutputMode = %d is not supported.", outputMode );
		#endif
	}

	// Return our result
	return ret;
}
VOID HTTPEndpoint::SetTag( const std::string& tag )
{
	m_httpResponse->mUser.Tag = tag;
}
VOID HTTPEndpoint::SetContentId( ULONGLONG contentId )
{
	m_httpResponse->mUser.ContentId = contentId;
}
VOID HTTPEndpoint::SetUserData( DWORD_PTR dataContext )
{
	m_httpResponse->mUser.DataContext = dataContext;
}
VOID HTTPEndpoint::SetPriority( HTTP_REQUEST_PRIORITY priority )
{
	m_httpRequestPriority = priority;
}
VOID HTTPEndpoint::SetCanceled( VOID ) {
	State.PendingCancel = TRUE;
}

// Endpoint Operations
DWORD HTTPEndpoint::AllocateMemory( VOID ) {
	if( m_receiveBuffer == NULL ) {
		// Allocate memory for the receiveBuffer
		m_receiveBuffer = (BYTE*)malloc( HTTP_RECVBUFFER_SIZE );
		if( m_receiveBuffer == NULL ) {
			#if HTTP_DEBUG_ERROR
			  HTTPLog("Unable to allocate memory for receiveBuffer.  Unable to proceed." );
			#endif
			setInternalErrorCode( HTTP_ERR_OUTOFMEMORY );
			moveStateTo( ErrorEncountered );
			return HTTP_ERR_OUTOFMEMORY;
		}
	} else {
		// If the buffer has already been allocated, then let's just use that buffer
		// instead of creating a new buffer
		#if HTTP_DEBUG_VERBOSE
		  HTTPLog( "The receiveBuffer has been previously allocated.  Proceeding with previously allocated buffer." );
		#endif
	}

	// Allocate our sendBuffer if this is going we need it
	if( m_httpResponse->GetInputMode() != HTTP_INPUTMODE_NONE ) {
		if( m_sendBuffer == NULL ) {
			// Allocate memory for sendBuffer
			m_sendBuffer = (BYTE*)malloc( HTTP_SENDBUFFER_SIZE );
			if( m_sendBuffer == NULL ) {
				#if HTTP_DEBUG_ERROR
				  HTTPLog("Unable to allocate memory for sendBuffer.  Unable to proceed." );
				#endif
				setInternalErrorCode( HTTP_ERR_OUTOFMEMORY );
				moveStateTo( ErrorEncountered );
				return HTTP_ERR_OUTOFMEMORY;
			}
		} else {
			// If the buffer has already been allocated, then let's just use that buffer
			// instead of creating a new buffer
			#if HTTP_DEBUG_VERBOSE
				HTTPLog( "The sendBuffer has been previously allocated.  Proceeding with previously allocated buffer." );
			#endif
		}
	}

	// Return successfully
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::FreeMemory( VOID )
{
	// Free dynamically allocated Receive Buffer
	if( m_receiveBuffer != NULL ) {
		free(m_receiveBuffer);
		m_receiveBuffer = NULL;
	}

	// Free dynamically allocated Send Buffer
	if( m_sendBuffer != NULL ) {
		free(m_sendBuffer);
		m_sendBuffer = NULL;
	}

	// Return successfully
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::OpenRequest( VOID )
{
	// Verify that we are in the correct state before proceeding
	if( m_internalState != Idle ) {
		#if HTTP_DEBUG_WARNING
		  HTTPLog( "The internal state should be idle before opening a request.  Current State = %d", m_internalState );
		#endif
		setInternalErrorCode( HTTP_ERR_UNEXPECTED );
		moveStateTo( ErrorEncountered );
		return HTTP_ERR_UNEXPECTED;
	}

	// Initialize 
	if( m_httpStream->Initialize(0) != ERROR_SUCCESS ) {
		#if HTTP_DEBUG_WARNING
		  HTTPLog( "Unable to initialize stream object" );
		#endif
		setInternalErrorCode( HTTP_ERR_UNEXPECTED );
		moveStateTo( ErrorEncountered );
		return HTTP_ERR_UNEXPECTED;
	}

	// Attempt Connection with host on specified port
	if( m_httpStream->Connect( m_urlComponents.Host.c_str(), m_urlComponents.Port & 0xFFFF ) != ERROR_SUCCESS ) {
		// Unable to connect to host
		#if HTTP_DEBUG_ERROR
		  HTTPLog( "Unable to connect to host '%s' on port '%d'.", m_urlComponents.Host.c_str(), m_urlComponents.Port & 0xFFFF );
		#endif

		// Set our error code and bail
		setInternalErrorCode( HTTP_ERR_CONNFAILED );
		moveStateTo( ErrorEncountered );
		return HTTP_ERR_CONNFAILED;
	}

	// Connection attemp successful, proceed
	moveStateTo( Init );
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::CloseRequest( DWORD dwReason )
{
	// Signify to our stream that the request is closing
	if( m_httpStream ) 
		m_httpStream->Close(dwReason);

	// Finalize and close any open output files
	if( m_hOutputFile != INVALID_HANDLE_VALUE )
	{
		DWORD fileSize = GetFileSize( m_hOutputFile, NULL );
		m_httpResponse->mOutputFileSize = fileSize;
		closeOutputFile();
	}

	// Finalize and close any open input files
	if( m_hInputFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hInputFile );
		m_hInputFile = INVALID_HANDLE_VALUE;
	}

	// Return succesfully
	return HTTP_ERR_OK;
}
BOOL  HTTPEndpoint::RequestCompleted( VOID )
{
	return State.Completed;
}

// Utility Methods
VOID HTTPEndpoint::createMultiPartFileHeader(const std::string& fieldName) {
	m_ContentFileHeader = "";
	m_ContentFileHeader += "--" + m_ContentBoundary + "\r\n";
	m_ContentFileHeader += "Content-Disposition: form-data; name=\"" + fieldName;
	m_ContentFileHeader += "\"; filename=\"";
	m_ContentFileHeader += "XboxFile.bin\"\r\n";
	m_ContentFileHeader += "Content-Type: application/octet-stream\r\n\r\n";
}
VOID HTTPEndpoint::createMultiPartFileFooter( VOID ) {
	m_ContentFileFooter = "\r\n--" + m_ContentBoundary + "--\r\n";
}

std::string HTTPEndpoint::buildCookieRequestHeaders( void ) {
	
	if( m_httpCookieJar.expired() == true ) { return ""; }
	HTTPCookieJarPtr cookieJar = m_httpCookieJar.lock();
	
	std::vector<const HTTPCookiePtr> httpCookies;
	if( cookieJar->FindMatchingCookies( GetURLComponents(), &httpCookies ) == 0 ) { return ""; }

	std::string cookieHeaders = "";
	for( DWORD x = 0; x < httpCookies.size(); x++ )
	{
		const HTTPCookiePtr& ptr = httpCookies[x];
		if( ptr == NULL ) continue;

		if( x == 0 ) {
			cookieHeaders += "Cookie: ";
		}	
		else if( x > 0 ) {
			cookieHeaders += "; ";
		}
		cookieHeaders += ptr->ToCookieString();
	}
	if( cookieHeaders.empty() == false ) 		cookieHeaders += "\r\n";

	return cookieHeaders;
}
std::string HTTPEndpoint::buildFormData( const std::string& boundary /*=""*/ )
{
	std::string httpFormDataString = "";
	typedef std::map<std::string, std::string>::const_iterator mapItr;
	for( mapItr itr = m_httpFormData.cbegin(); itr != m_httpFormData.cend(); ++itr )
	{
		const std::string& key = itr->first;
		const std::string& value = itr->second;

		if( boundary.empty() )
		{
			// Process PostVars with no boundary
			if( itr != m_httpFormData.cbegin()) httpFormDataString += "&";

			httpFormDataString += URLencode(key);
			httpFormDataString += "=";
			httpFormDataString += URLencode(value);
		}
		else
		{
			// Process Post vars with a boundary supplied
			httpFormDataString += "--" + boundary + "\r\n";
			httpFormDataString += "Content-Disposition: form-data; name=\"";
			httpFormDataString += URLencode(key);
			httpFormDataString += "\"\r\n\r\n";
			httpFormDataString += URLencode(value);
			httpFormDataString += "\r\n";
		}
	}

	// Return our post var string
	return httpFormDataString;
}
DWORD HTTPEndpoint::openOutputFile( const std::string& filePath, BOOL overwriteExisting /*= TRUE*/ )
{
	if( filePath.empty() == true ) return E_INVALIDARG;

	std::string tempFilePath = filePath + ".dl";
	std::string outputPath = filePath.substr( 0, filePath.find_last_of("\\") );
	
	// Recursively make directory for this output path
	RecursiveMkdir( outputPath );

	// Open our file
	HANDLE hFile = CreateFile( tempFilePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile == INVALID_HANDLE_VALUE ) {
		HTTPLog( "Error Code:  %d.  Failed to create file:  %s", GetLastError(), tempFilePath.c_str() );
		return E_FAIL;
	}

	// Successfully opened file, so let's store our information
	m_hOutputFile = hFile;

	// Return successfully
	return ERROR_SUCCESS;
}
DWORD HTTPEndpoint::closeOutputFile( VOID )
{
	// If we don't have a valid HTTP Response pointer, then something is wrong
	if( m_httpResponse == NULL ) return E_UNEXPECTED;

	// If the file handle is not valid, then something is wrong
	if( m_hOutputFile == INVALID_HANDLE_VALUE ) return E_UNEXPECTED;

	// If the destination path is empty, then we can't write this file
	if( m_httpResponse->mOutputFilePath.empty() == true ) return E_UNEXPECTED;

	// Close the file handle
	BOOL ret = CloseHandle(m_hOutputFile);
	HTTPLog( "Closing file handle:  %s - %s", m_httpResponse->mOutputFilePath.c_str(), ret == TRUE ? "Success" : "Failed" );
	m_hOutputFile = INVALID_HANDLE_VALUE;

	// Delete the existing file
	if( DeleteFile( m_httpResponse->mOutputFilePath.c_str() ) == FALSE ) {
		HTTPLog( "Error Code:  %d.  Failed to delete file:  %s", GetLastError(), m_httpResponse->mOutputFilePath.c_str() );
	}

	// Rename the temporary download file to the requested Output file
	std::string tempFilePath = m_httpResponse->mOutputFilePath + ".dl";
	int result = rename( tempFilePath.c_str(), m_httpResponse->mOutputFilePath.c_str() );
	if( result != 0 ) {
		HTTPLog( "Error Code:  %d.  Failed to rename file %s to %s.", result, tempFilePath.c_str(), m_httpResponse->mOutputFilePath.c_str() );
	}

	// Finally, return successfully
	return ERROR_SUCCESS;
}
DWORD HTTPEndpoint::readInputBuffer( const BYTE * buffer, DWORD size ) {
	// Verify that avalid pointer to a buffer is provided
	if( m_httpResponse == NULL || buffer == NULL || size == 0 ) return 0UL;

	DWORD bytesRead = 0UL;
	if( m_httpResponse->mInputData.Mode == HTTP_INPUTMODE_FILE ) {
		// Verify that we have a valid handle
		if( m_hInputFile == INVALID_HANDLE_VALUE ) {
			#if HTTP_DEBUG_ERROR
			  HTTPLog( "Unable to write file due to invalid handle.");
			#endif
			return 0UL;
		}

		// Write data to buffer
		if( ReadFile( m_hInputFile, (LPVOID)buffer, size, &bytesRead, NULL ) == FALSE || bytesRead != size ) {
			#if HTTP_DEBUG_ERROR
			  HTTPLog("Error Code:  %d.   Failed to read from file.", GetLastError() );
			#endif
			return bytesRead;
		}
	}
	else 
	{
		if( size > 0 )
		{
			const BYTE * inBuffer = m_httpResponse->GetInputBuffer() + m_nInputBufferPosition;
			memcpy( (void*)buffer, (const void*)inBuffer, size );
			m_nInputBufferPosition += size;
			bytesRead = size;
		}
	}
	
	// Return the number of bytes read
	return bytesRead;
}
DWORD HTTPEndpoint::writeOutputBuffer( const BYTE * buffer, DWORD size )
{
	// Verify that we have a valid httpResponse to write to
	if( m_httpResponse == NULL ) return 0UL;

	DWORD bytesWritten = 0UL;
	if( m_httpResponse->mOutputMode == HTTP_OUTPUTMODE_FILE ) {
		// Verify that we have a valid handle
		if( m_hOutputFile == INVALID_HANDLE_VALUE ) {
			HTTPLog("Unable to write to file due to invalid handle." );
			return 0UL;
		}

		// Write our incoming buffer to file
		if( WriteFile( m_hOutputFile, (LPCVOID)buffer, size, &bytesWritten, NULL ) == FALSE || bytesWritten != size ) {
			HTTPLog("Error Code:  %d.   Failed to write to file.", GetLastError() );
			return bytesWritten;
		}
	}
	else
	{
		// Write our incoming buffer to our MemoryBuffer
		m_httpResponse->mOutputBuffer.append( buffer, size );
		bytesWritten = size;
	}
	
	// Return the number of bytes written
	return bytesWritten;
}

// State Processing Methods
DWORD HTTPEndpoint::ProcessState( VOID )
{
	// Assume success
	DWORD ret = HTTP_ERR_OK;

	#if HTTP_DEBUG_VERBOSE
	  static const char * gszStates[] = {
		  "Idle", "Init", "OpenOutputFile", "SendingRequest", "RequestSent", "SendingInputHeader", "InputHeaderSent", "SendingInputFile", "InputFileSent", "InputFileSentFinal",
		  "SendingInputFooter", "InputFooterSent", "ReceivingResponse", "ResponseHeaderAvailable", "ReceivingResponseHeader", "ReceivingResponseChunkHeader", "ReceivingResponseChunkFooter",
		  "ReceivingResponseBody", "ResponseDataAvailable", "ResponseReceived", "Completed", "ErrorEncountered", "Canceled", "Unrecognized State" };

		  DWORD stateDescId = m_internalState < ARRAYSIZE(gszStates) ? m_internalState : ARRAYSIZE(gszStates) - 1;

		  HTTPLog( "Processing State:  %d - %s", m_internalState, gszStates[stateDescId] );
	#endif

	// Execute the current state
	switch( m_internalState ) {
		case Idle:							ret = processState_Idle(); break;
		case Init:							ret = processState_Init(); break;
		case RequestSent:					ret = processState_RequestSent(); break;
		case SendingInputHeader:			ret = processState_SendingInputHeader(); break;
		case SendingInputBuffer:			ret = processState_SendingInputBuffer(); break;
		case SendingInputFooter:			ret = processState_SendingInputFooter(); break;
		case InputFooterSent:				ret = processState_InputFooterSent(); break;
		case ResponseHeaderAvailable:		ret = processState_ResponseHeaderAvailable(); break;
		case ReceivingResponseChunkHeader:	ret = processState_ReceivingResponseChunkHeader(); break;
		case ReceivingResponseBody:			ret = processState_ReceivingResponseBody(); break;
		case ReceivingResponseChunkFooter:	ret = processState_ReceivingResponseChunkFooter(); break;
		case ResponseDataAvailable:			ret = processState_ResponseDataAvailable(); break;
		case ResponseReceived:				ret = processState_ResponseReceived(); break;
		case ErrorEncountered:				ret = processState_ErrorEncountered(); break;
		case Completed:						ret = processState_Completed(); break;
		case Canceled:						ret = processState_Canceled(); break;
		default:
		{
			// The state we received is unrecognized and could lead to problems if not investigated.
			#ifdef HTTP_DEBUG_ERROR
			  HTTPLog( "Unrecognized State:  %d", m_internalState );
			#endif
		}
	}

	// Cancel the operation, if flag has been set
	if( State.PendingCancel == TRUE ) {
		moveStateTo( Canceled );
	}

	// Return successfully
	return ret;
}
DWORD HTTPEndpoint::processState_Idle( VOID )
{
	Sleep(500);
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::processState_Init( VOID )
{
	// First, if the Host or Connection headers don't exist, then let's add default values to make sure we are compliant
	if( RequestHeaderExists( "Host" ) == FALSE ) AddRequestHeader( "Host", m_urlComponents.Host );
	if( RequestHeaderExists( "Connection" ) == FALSE ) AddRequestHeader( "Connection", "close" );
	if( RequestHeaderExists( "User-Agent" ) == FALSE ) AddRequestHeader( "User-Agent", HTTP_REQUEST_USER_AGENT );

	// PreProcess our REQUEST_TYPE specific information before building our request
	std::string httpMessageData = "";
	if( requestHasBody() == TRUE )
	{
		if( m_httpResponse->GetInputMode() == HTTP_INPUTMODE_MEMORY && m_httpFormData.empty() == true )
		{
			// Calculate our total request size
			State.InputLengthRemaining = m_httpResponse->GetInputSize();

			// We need to add a couple of headers to support this information.
			// To do this, we will overwrite any existing headers
			AddRequestHeader( "Content-Length", sprintfaA("%d", State.InputLengthRemaining) );
			if( RequestHeaderExists( "Content-Type" ) == FALSE ) {
				AddRequestHeader( "Content-Type", "application/octet-stream" );
			}
		}
		else if( m_httpResponse->GetInputMode() == HTTP_INPUTMODE_FILE || m_httpResponse->GetInputMode() == HTTP_INPUTMODE_MEMORY ) 
		{
			// If we have an input file or if we have an input file or memory buffer with post vars, then we need to submit our data in multiple parts
			m_ContentBoundary = "---------------------------103832778631715";

			// Build the PostVars string (with boundaries)
			httpMessageData = buildFormData( m_ContentBoundary );
			createMultiPartFileHeader( "file" );
			createMultiPartFileFooter();

			// Open input file to start reading
			m_hInputFile = CreateFile( m_httpResponse->GetInputFilePath().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
			if( m_hInputFile == INVALID_HANDLE_VALUE ) {
				// An error 
				setInternalErrorCode(HTTP_ERR_FILEERROR);
				moveStateTo( ErrorEncountered );
				return HTTP_ERR_FILEERROR;
			}

			if (SetFilePointer( m_hInputFile, 0, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER ) {
				// An error 
				setInternalErrorCode(HTTP_ERR_FILEERROR);
				moveStateTo( ErrorEncountered );
				return HTTP_ERR_FILEERROR;
			}

			// Calculate our total request size
			DWORD totalRequestSize = httpMessageData.length();
			totalRequestSize += m_ContentFileHeader.length();
			totalRequestSize += m_httpResponse->GetInputSize();
			totalRequestSize += m_ContentFileFooter.length();

			State.InputLengthRemaining = m_httpResponse->GetInputSize();

			// We need to add a couple of headers to support this information.
			// To do this, we will overwrite any existing headers
			AddRequestHeader( "Content-Length", sprintfaA("%d", totalRequestSize) );
			AddRequestHeader( "Content-Type", sprintfaA("multipart/form-data; boundary=%s", m_ContentBoundary.c_str()) );
		} 
		else if( m_httpFormData.empty() == false ) 
		{
			// Build the PostVar string
			httpMessageData = buildFormData();

			// We need to add a couple of headers to support this information.
			// To do this, we will overwrite any existing headers
			AddRequestHeader( "Content-Length", sprintfaA("%d", httpMessageData.length()) );
			AddRequestHeader( "Content-Type", "application/x-www-form-urlencoded" );
		}
	}

	// The next step is to build our request string
	std::string httpRequestString = m_httpVerb + " " + m_urlComponents.Path + m_urlComponents.ExtraInfo + " " + m_httpClientVersion + "\r\n";
	typedef std::map<std::string, std::string>::const_iterator mapItr;
	for( mapItr itr = m_httpRequestHeaders.cbegin(); itr != m_httpRequestHeaders.cend(); ++itr )
	{
		// Retreive key and value from the header map
		const std::string& key = itr->first;
		const std::string& value = itr->second;

		// Append header entries to the request string
		httpRequestString += key + ": " + value + "\r\n";
	}

	// Before finalizing the request string, let's add our cookies, which we want to handle differently than our headers
	httpRequestString += buildCookieRequestHeaders();

	// Finally, add our blank line to signify the end of the header
	httpRequestString += "\r\n";

	// Depening on the request type, we will add a message to our http request
	if( httpMessageData.empty() == false ) 
		httpRequestString += httpMessageData;

	// We are ready to send our HTTP REQUEST
	DWORD bytesSent = m_httpStream->Send( (const BYTE*)httpRequestString.c_str(), httpRequestString.length() );
	if( bytesSent == httpRequestString.length() ) {
		moveStateTo( RequestSent );
	} else {
		setInternalErrorCode( HTTP_ERR_RECVFAILED );
		moveStateTo( ErrorEncountered );
	}

	// Let's start our timer to measure speed
	State.Timer.Start();

	// Return successfully
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::processState_RequestSent( VOID )
{
	// If this is a POST request, we need to send the body of the request containing PostVars or FileData
	if( requestHasBody() == TRUE )
	{
		// If we have an input file, then we need to submit our data in multiple parts
		if( m_httpResponse->GetInputMode() == HTTP_INPUTMODE_MEMORY && m_httpFormData.empty() == true )
		{
			moveStateTo( SendingInputBuffer );
			return HTTP_ERR_OK;
		}
		else if( m_httpResponse->GetInputMode() == HTTP_INPUTMODE_FILE || m_httpResponse->GetInputMode() == HTTP_INPUTMODE_MEMORY )
		{
			moveStateTo(SendingInputHeader);
			return HTTP_ERR_OK;
		}
	}

	// We are ready to receive our our ResponseHeader
	if( m_httpResponse->mResponseHeader.ReceiveHeader( m_httpStream ) == TRUE ) {
		moveStateTo(ResponseHeaderAvailable);
	} else {
		moveStateTo(ErrorEncountered );
	}

	// Return successfully
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::processState_SendingInputHeader( VOID )
{
	// If this is a POST request, we need to send the body of the request containing PostVars or FileData
	if( requestHasBody() == TRUE )
	{
		// If we have an input file, then we need to submit our data in multiple parts
		if( m_httpResponse->GetInputMode() == HTTP_INPUTMODE_MEMORY && m_httpFormData.empty() == true )
		{
			moveStateTo( SendingInputBuffer );
		}
		else if( m_httpResponse->GetInputMode() == HTTP_INPUTMODE_FILE || m_httpResponse->GetInputMode() == HTTP_INPUTMODE_MEMORY )
		{
			if( m_httpStream->Send( (const BYTE*)m_ContentFileHeader.c_str(), m_ContentFileHeader.length() ) == m_ContentFileHeader.length() )
			{
				moveStateTo( SendingInputBuffer );
			}
			else
			{
				// HANDLE ERROR
			}

		}
		else
		{
			// HANDLE ERROR
		}
	}
	else
	{
		// HANDLE ERROR
	}

	// Return successfully
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::processState_SendingInputBuffer( VOID )
{
	DWORD bytesToSend = (State.InputLengthRemaining < HTTP_SENDBUFFER_SIZE ) ? State.InputLengthRemaining : HTTP_SENDBUFFER_SIZE;
	if( bytesToSend == 0 ) {
		moveStateTo( SendingInputFooter );
	}
	else 
	{
		// Read data into our buffer
		readInputBuffer( m_sendBuffer, bytesToSend );

		// There are bytes to be read from the stream
		State.BytesSent = m_httpStream->Send( m_sendBuffer, bytesToSend );
		if( State.BytesSent < 0 ) {
			// If we received a negative return value, then a upload error has occurred
			setInternalErrorCode( HTTP_ERR_SENDFAILED );
			moveStateTo(ErrorEncountered );
			return HTTP_ERR_SENDFAILED;
		} else if( State.BytesSent == 0 ) {
			moveStateTo( SendingInputFooter );
		} else {
			// Reduce the ContentLengthRemaining variable by the number of bytes that have been read
			State.InputLengthRemaining -= State.BytesSent;
			State.TotalBytesDownloaded += State.BytesSent;
			moveStateTo( SendingInputBuffer );
		}
	}

	// Return successfully
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::processState_SendingInputFooter( VOID )
{
	// If this is a POST request, we need to send the body of the request containing PostVars or FileData
	if( requestHasBody() == TRUE )
	{
		// If we have an input file, then we need to submit our data in multiple parts
		// If we have an input file, then we need to submit our data in multiple parts
		if( m_httpResponse->GetInputMode() == HTTP_INPUTMODE_MEMORY && m_httpFormData.empty() == true )
		{
			moveStateTo( InputFooterSent );
		}
		else if( m_httpResponse->GetInputMode() == HTTP_INPUTMODE_FILE || m_httpResponse->GetInputMode() == HTTP_INPUTMODE_MEMORY )
		{
			if( m_httpStream->Send( (const BYTE*)m_ContentFileFooter.c_str(), m_ContentFileFooter.length() ) == m_ContentFileFooter.length() )
			{
				moveStateTo( InputFooterSent );
			}
			else
			{
				// HANDLE ERROR
			}

		}
		else
		{
			// HANDLE ERROR
		}
	}
	else
	{
		// HANDLE ERROR
	}

	// Return successfully
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::processState_InputFooterSent( VOID )
{
	// We are ready to receive our our ResponseHeader
	if( m_httpResponse->mResponseHeader.ReceiveHeader( m_httpStream ) == TRUE ) {
		moveStateTo(ResponseHeaderAvailable);
	} else {
		moveStateTo(ErrorEncountered );
	}

	// Return successfully
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::processState_ResponseHeaderAvailable( VOID )
{
	// Grab a reference to our HTTP Header
	const CHTTPHeader& httpHeader = m_httpResponse->GetResponseHeader();

	// Process Cookies
	if( m_httpCookieJar.expired() == false )
	{
		HTTPCookieJarPtr cookieJar = m_httpCookieJar.lock();
		if( cookieJar != NULL ) 
		{
			std::vector<std::string> serverCookies;
			if( httpHeader.GetHeaderValues( "Set-Cookie", &serverCookies ) > 0 )
			{
				for( DWORD x = 0; x < serverCookies.size(); x++ )
				{
					if( cookieJar->SetCookie( serverCookies[x], m_urlComponents.Host, m_urlComponents.Path ) == FALSE )
					{
						HTTPLog("Cookie %s rejected.", serverCookies[x].c_str() );
					}
				}
			}
		}
	}


	// Logic Tree to handle differing response types
	DWORD dwStatusCode = httpHeader.GetStatusCode();
	if( dwStatusCode == HTTP_STATUS_REDIRECT )
	{
		moveStateTo(Completed);
	}
	else if( dwStatusCode < HTTP_STATUS_OK || dwStatusCode == HTTP_STATUS_NO_CONTENT || dwStatusCode == HTTP_STATUS_NOT_MODIFIED )
	{
		moveStateTo(Completed);
	}
	else if( httpHeader.IsChunked() == TRUE )
	{
		// Content is chunked, so this response should be handled differently
		//HTTPLog("Start receiving - chunked response" );
		State.Timer.Start();

		// Prepare our state variables to handle chunked data
		State.ChunkSize = 0;
		State.ContentLength = 0;
		State.ContentLengthRemaining = 0;

		// Change State to start receiving response
		moveStateTo(ReceivingResponseChunkHeader);
	}
	else 
	{
		DWORD contentLength = httpHeader.GetContentLength();
		State.ContentLengthRemaining = contentLength > 0 ? contentLength : MAXDWORD;
		State.ContentLength = State.ContentLengthRemaining;
//		HTTPLog( "Start receiving" );
		State.Timer.Start();
		moveStateTo(ReceivingResponseBody);
	}

	// Now, let's prepare our buffers
	if( m_httpResponse->mOutputMode == HTTP_OUTPUTMODE_FILE )
	{
		// Open our input file to retrieve the contents of the URL request
		openOutputFile( m_httpResponse->mOutputFilePath, TRUE );
	}
	else
	{
		m_httpResponse->mOutputBuffer.clear();

	}

	// Finally, if the user has requested MD5 hash of response content, then
	// initialize our MD5 state structure
	if( State.CalculateMD5 == TRUE ) 
	{
		#if HTTP_DEBUG_VERBOSE
		  HTTPLog( "Calculating MD5 for Endpoint" );
		#endif

		  State.CalculatingMD5 = TRUE;
		  #if defined(HAVE_XKELIB_H)
		    XeCryptMd5Init( &m_md5State);
		  #else
			#error "The xkelib library is required for MD5 computation.  "
		  #endif
	}

	// Return successfully
	return HTTP_ERR_OK;

}
DWORD HTTPEndpoint::processState_ReceivingResponseChunkHeader( VOID )
{
	std::string chunkHeader = "";
	while(1)
	{
		CHAR data = 0;
		if( m_httpStream->Receive( (BYTE*)&data, 1 ) != 1 ) {
			DebugBreak();
		}

		if( data == '\r' ) continue;
		else if( data == '\n' ) {
			break;
		} else {
			chunkHeader += data;
		}
	}

	// Parse the chunk header to determine how many bytes to read
	size_t endPos = chunkHeader.find_first_of(";");
	if( endPos == std::string::npos ) {
		endPos = chunkHeader.find_first_of("\r\n");
	}
	std::string size = chunkHeader.substr( 0, endPos );
	State.ChunkSize = strtol( size.c_str(), NULL, 16 );
	State.ContentLengthRemaining = State.ChunkSize;
	State.ContentLength += State.ChunkSize;
	moveStateTo(ReceivingResponseBody);

	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::processState_ReceivingResponseBody( VOID )
{
	// Calculate how many bytes should be read in this cycle.  We are 
	// clamped by how big the RECVBUFFER is and by how many bytes are remaining to download
	DWORD bytesToRead = (State.ContentLengthRemaining < (HTTP_RECVBUFFER_SIZE)) ? State.ContentLengthRemaining : (HTTP_RECVBUFFER_SIZE);
	if( bytesToRead == 0 ) {
		// If there are no more bytes to read, then the download (full content or a chunk)
		// was completed.
		if( m_httpResponse->GetResponseHeader().IsChunked() == TRUE ) {
			// Chunk was completed, so process the chunk footer
			moveStateTo(ReceivingResponseChunkFooter);
		} else {
			// Message was completed, so let's process this download
			moveStateTo(ResponseReceived);
		}
	} else {
		// There are bytes to be read from the stream
		State.BytesRead = m_httpStream->Receive( m_receiveBuffer, bytesToRead );
		if( State.BytesRead < 0 ) {
			// If we received a negative return value, then a download error has occurred
			setInternalErrorCode( HTTP_ERR_RECVFAILED );
			moveStateTo(ErrorEncountered );
			return HTTP_ERR_RECVFAILED;
		} else if( State.BytesRead == 0 ) {
			// There is no further data to receive, so the response has been received
			moveStateTo(ResponseReceived );
		} else {
			// Data was received, so change state to next stage
			moveStateTo(ResponseDataAvailable);
		}
	}

	// Processessing was successful
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::processState_ReceivingResponseChunkFooter( VOID )
{
	BYTE trailingCRLF[2]; memset(trailingCRLF, 0, 2 );
	m_httpStream->Receive( trailingCRLF, 2 );
	State.ContentLengthRemaining = 0;
			
	// If the chunk that was processed was 0, then we've reached the end
	if( State.ChunkSize == 0 ) {
		moveStateTo(ResponseReceived);
	} else {
		moveStateTo(ReceivingResponseChunkHeader);
	}

	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::processState_ResponseDataAvailable( VOID )
{
	// Update our MD5 State
	if( State.CalculatingMD5 == TRUE ) {
		#if defined(HAVE_XKELIB_H)
		  XeCryptMd5Update( &m_md5State, m_receiveBuffer, State.BytesRead );
		#else
		  #error "The xkelib library is required for MD5 computation.  "
		#endif
	}

	// Write our data our buffer or output file
	if( writeOutputBuffer( m_receiveBuffer, State.BytesRead ) != State.BytesRead ) {
		#if HTTP_DEBUG_ERROR
		  HTTPLog( "Unable to write output data." );
		#endif

		// Unable to write to file, so let's fail
		setInternalErrorCode( HTTP_ERR_WRITEFAILED );
		moveStateTo(ErrorEncountered);
		return HTTP_ERR_WRITEFAILED;
	}

	// Reduce the ContentLengthRemaining variable by the number of bytes that have been read
	State.ContentLengthRemaining -= State.BytesRead;
	State.TotalBytesDownloaded += State.BytesRead;

	// Loop back on our state
	moveStateTo(ReceivingResponseBody);

	// Return successfully
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::processState_ResponseReceived( VOID )
{
	// Finalize our MD5 State and generate our hash strings
	if( State.CalculatingMD5 == TRUE ) 
	{
		// Finalize MD5
		BYTE md5Hash[0x10]; memset( md5Hash, 0, 0x10 );

		#if defined(HAVE_XKELIB_H)
		  XeCryptMd5Final( &m_md5State, md5Hash, 0x10 );
		#else
		  #error "The xkelib library is required for MD5 computation.  "
		#endif

		// Get our serial as a string
		if( md5Hash[0] != 0 ) {
			CHAR md5String[41]; md5String[40] = 0; UINT outLen = 0x40;
			ZeroMemory(md5String, 41);
			GetBytesString(md5Hash, 0x10, md5String, &outLen);
			m_httpResponse->mOutputMD5Hash = md5String;
		}
	}
	else
	{
		// Reset fields related to the MD5
		m_httpResponse->mOutputMD5Hash = "";
	}

	moveStateTo(Completed);
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::processState_ErrorEncountered( VOID )
{
	#if HTTP_DEBUG_VERBOSE
		static const char * gszErrorStrings[] = { "Ok", "Fail", "Unexpected", "Invalid Arg", "Out of Memory", "Connection Failed", "Receive Failed", "Not Implemented", "Unknown Error" };
		DWORD errCode = -1 * getLastInternalError();
		DWORD errorDescId = errCode < ARRAYSIZE(gszErrorStrings) ? errCode : ARRAYSIZE(gszErrorStrings) - 1;
		HTTPLog("An error was enountered.  ErrorCode:  %d - %s", getLastInternalError(), gszErrorStrings[errorDescId] );
	#elif HTTP_DEBUG_ERROR
		HTTPLog("An error was enountered.  ErrorCode:  %d", getLastInternalError() );
	#endif

	// Set our end point error code
	m_httpResponse->mErrorCode = getLastInternalError();

	moveStateTo(Completed);
	return getLastInternalError();
}
DWORD HTTPEndpoint::processState_Completed( VOID )
{
	// Stop the download timer
	State.Timer.Stop();
	m_httpResponse->mDownloadTime = (FLOAT)(State.Timer.m_fStopTime - State.Timer.m_fBaseTime);

	// Set the final bytes transferred

	m_httpResponse->mOutputFileSize = State.TotalBytesDownloaded;

	State.Completed = TRUE;

	// Final State
	return HTTP_ERR_OK;
}
DWORD HTTPEndpoint::processState_Canceled( VOID )
{
	printf( "Endpoint operation was canceled\n" );
	State.Timer.Stop();

	// Set the download time
	m_httpResponse->mDownloadTime = (FLOAT)(State.Timer.m_fStopTime - State.Timer.m_fBaseTime);
	m_httpResponse->mOutputFileSize = State.BytesRead;

	// Final State
	return HTTP_ERR_OK;
}

// Constructor & Destructor
HTTPEndpoint::HTTPEndpoint(HTTP_REQUEST_TYPE nRequestType, const CHAR * pszUrl, const CHAR * pszTag /*= ""*/ ) :
	m_httpRequestType(nRequestType), m_requestUrl(pszUrl)
{
	// Parse the URL into components and store for later use
	m_urlComponents			= Uri::Parse(m_requestUrl);

	// Store our request type verb
	m_httpVerb				= gszHttpVerbList[nRequestType];
	m_httpClientVersion		= HTTP_REQUEST_VERSION;

	// Determine our URL scheme and create our streaming object
	if( strcmpi( m_urlComponents.Protocol.c_str(), "http" ) == 0 ) {
		m_httpRequestScheme = HTTP_REQUEST_SCHEME_HTTP;
		m_httpStream = new HTTPStream();
	} else if( strcmpi( m_urlComponents.Protocol.c_str(), "https" ) == 0 ) {
		m_httpRequestScheme = HTTP_REQUEST_SCHEME_HTTPS;
		m_httpStream = new SSLStream();
	} else {
		// Unsupported protocol
		m_httpRequestScheme = HTTP_REQUEST_SCHEME_UNKNOWN;
		m_httpStream = NULL;
	}

	// Initialize our response variable
	m_httpCookieJar.reset();
	m_httpRequestPriority = HTTP_REQUEST_PRIORITY_NORMAL;
	m_httpResponse = std::shared_ptr<HTTPResponse>(new HTTPResponse());
	m_httpResponse->mRequestType = nRequestType;
	m_receiveBuffer = NULL;
	m_sendBuffer = NULL;

	// Store our tag
	m_httpResponse->mUser.Tag = pszTag;
	m_httpResponse->mUrlComponents = m_urlComponents;

	// Initialize our state variables
	State.PendingCancel = FALSE;
	State.BytesRead = 0UL;
	State.ContentLengthRemaining = 0UL;
	State.InputLengthRemaining = 0UL;
	State.BytesSent = 0UL;
	State.TotalBytesDownloaded = 0UL;
	State.ContentLength = 0UL;
	State.CalculateMD5 = FALSE;
	State.CalculatingMD5 = FALSE;
	State.Completed = FALSE;

	// Initialize file handles
	m_hOutputFile = INVALID_HANDLE_VALUE;
	m_hInputFile = INVALID_HANDLE_VALUE;
	m_nInputBufferPosition = 0UL;

	// Update the State Machine
	moveStateTo( Idle );
}
HTTPEndpoint::~HTTPEndpoint() 
{
	// Close Request
	CloseRequest(2);

	// Free Memory
	FreeMemory();
}
