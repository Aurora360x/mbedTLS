#pragma once
#include "HTTPTypes.h"
#include "Base/HTTPStream.h"
#include "Base/SSLStream.h"
#include "Tools/Uri.h"

// Parts of the HTTP Endpoint
#include "HTTPHeader.h"
#include "HTTPResponse.h"
#include "HTTPCookieJar.h"

class IHttpClientObserver {
public:
	virtual VOID DownloadComplete( HTTPResponseConstPtr httpResponseInfo ) = 0;
};

#define HTTP_RECVBUFFER_SIZE		1024 * 16
#define HTTP_SENDBUFFER_SIZE		1024 * 16

class HTTPEndpoint
{
public:
	static HTTPEndpoint * CreateEndpoint(HTTP_REQUEST_TYPE nRequestType, const CHAR * pszUrl, const CHAR * pszTag = "");
	virtual ~HTTPEndpoint();

	BOOL RedirectEndpoint();

	// Request Helper Methods
	HTTP_REQUEST_TYPE GetRequestType() const			{ return m_httpRequestType; }
	const std::string& GEndetClientVersion() const			{ return m_httpClientVersion; }
	const std::string& GetRequestURL() const			{ return m_requestUrl; }
	const Uri& GetURLComponents() const					{ return m_urlComponents; }

	// Request Header Methods
	VOID AddRequestHeader(const std::string& key, const std::string& value);
	VOID RemoveRequestHeader( const std::string& key );
	BOOL RequestHeaderExists( const std::string& key ) const;
	const std::string& GetRequestHeaderValue( const std::string& key ) const;

	// Post Var Methods
	VOID AddFormData( const std::string& key, const std::string& value );
	VOID RemoveFormData( const std::string& key );
	BOOL FormDataExists( const std::string& key ) const;
	const std::string& GetFormDataValue( const std::string& key ) const;

	// Cookie Methods
	const std::weak_ptr<HTTPCookieJar> SetCookieJar( const std::weak_ptr<HTTPCookieJar>& cookieJar );

	// Endpoint Configuration
	VOID	SetMD5HashVerification( BOOL verify );
	DWORD   SetInputFile( const std::string& filePath );
	DWORD   SetInputBuffer( const std::string& data );
	DWORD   SetInputBuffer( const CMemoryBuffer& buffer, BOOL isPersistant = FALSE );
	DWORD   SetInputBuffer( const BYTE * buffer, DWORD size, BOOL isPersistent = FALSE );
	DWORD	SetOutputMode( HTTP_OUTPUTMODE outputMode, const std::string& fileName = "" );
	VOID	SetTag( const std::string& tag );
	VOID	SetContentId( ULONGLONG contentId );
	VOID	SetUserData( DWORD_PTR context );
	VOID	SetPriority( HTTP_REQUEST_PRIORITY priority );
	VOID	SetCanceled( VOID );

	// Endpoint Flow Control
	DWORD AllocateMemory( VOID );
	DWORD OpenRequest( VOID );
	DWORD CloseRequest( DWORD dwReason );
	DWORD FreeMemory( VOID );
	BOOL RequestCompleted( VOID );
	DWORD ProcessState( VOID );

	// Endpoint Access
	HTTPResponseConstPtr GetResponseInfo()				{ return m_httpResponse; }
	DWORD GetContentLength() const						{ return State.ContentLength; }
	DWORD GetBytesRead() const							{ return State.ContentLength - State.ContentLengthRemaining; }
	DWORD GetPriority() const							{ return 0; }
	BOOL  IsCanceled( VOID ) const						{ return State.PendingCancel; }

private:
	// Maps to store request headers and post vars set by client
	std::map<std::string, std::string> m_httpRequestHeaders;
	std::map<std::string, std::string> m_httpFormData;


	// Using smart poitners to ensure this response is adequately freed
	std::shared_ptr<HTTPResponse> m_httpResponse;

	// Instance Variables
	BYTE				*m_receiveBuffer;
	BYTE				*m_sendBuffer;
	TCPStream			*m_httpStream;
	HTTPENDPOINT_STATE	 m_internalState;
	std::weak_ptr<HTTPCookieJar> m_httpCookieJar;

	HTTP_REQUEST_SCHEME m_httpRequestScheme;
	HTTP_REQUEST_PRIORITY m_httpRequestPriority;
	HTTP_REQUEST_TYPE m_httpRequestType;
	std::string m_httpVerb;
	std::string m_httpClientVersion;
	std::string m_requestUrl;
	Uri m_urlComponents;

	BOOL m_hasInputFile;
	HANDLE m_hOutputFile;
	HANDLE m_hInputFile;
	DWORD m_nInputBufferPosition;

	std::string m_ContentBoundary;
	std::string m_ContentFileHeader;
	std::string m_ContentFileFooter;

	XECRYPT_MD5_STATE m_md5State;
	DWORD_PTR m_pvCallbackContext;

	BOOL requestHasBody( VOID ) const { return (	m_httpRequestType == HTTP_REQUEST_TYPE_POST || m_httpRequestType == HTTP_REQUEST_TYPE_PUT ||
													m_httpRequestType == HTTP_REQUEST_TYPE_PATCH || m_httpRequestType == HTTP_REQUEST_TYPE_DELETE) ? TRUE : FALSE; }
	std::string buildCookieRequestHeaders( VOID );
	std::string buildFormData( const std::string& boundary = "" );
	VOID createMultiPartFileHeader( const std::string& fieldName );
	VOID createMultiPartFileFooter( VOID );

	struct {
		Timer Timer;
		DWORD InternalErrorCode;
		BOOL Completed;
		BOOL CalculateMD5;
		BOOL CalculatingMD5;
		BOOL PendingCancel;
		INT BytesRead;
		INT BytesSent;
		DWORD ContentLength;
		DWORD ChunkSize;
		DWORD TotalBytesDownloaded;
		DWORD InputLengthRemaining;
		DWORD ContentLengthRemaining;
	} State;	

	// Private Helper Methods
	VOID moveStateTo( HTTPENDPOINT_STATE state ) { m_internalState = state; }
	VOID setInternalErrorCode( DWORD errCode ) { State.InternalErrorCode = errCode; m_httpResponse->mErrorCode = errCode; }
	DWORD getLastInternalError( VOID ) { return State.InternalErrorCode; }
	DWORD openOutputFile( const std::string& filePath, BOOL overwriteExisting );
	DWORD closeOutputFile( VOID );
	DWORD readInputBuffer( const BYTE * buffer, DWORD size );
	DWORD writeOutputBuffer( const BYTE * buffer, DWORD size );

	// State Processing Methods
	inline DWORD processState_Idle( VOID );
	inline DWORD processState_Init( VOID );
	inline DWORD processState_RequestSent( VOID );
	inline DWORD processState_SendingInputHeader( VOID );
	inline DWORD processState_SendingInputBuffer( VOID );
	inline DWORD processState_SendingInputFooter( VOID );
	inline DWORD processState_InputFooterSent( VOID );
	inline DWORD processState_ResponseHeaderAvailable( VOID );
	inline DWORD processState_ReceivingResponseChunkHeader( VOID );
	inline DWORD processState_ReceivingResponseBody( VOID );
	inline DWORD processState_ReceivingResponseChunkFooter( VOID );
	inline DWORD processState_ResponseDataAvailable( VOID );
	inline DWORD processState_ResponseReceived( VOID );
	inline DWORD processState_ErrorEncountered( VOID );
	inline DWORD processState_Completed( VOID );
	inline DWORD processState_Canceled( VOID );

	// Private Constructor
	HTTPEndpoint(HTTP_REQUEST_TYPE nRequestType, const CHAR * pszUrl, const CHAR * pszTag = "");
};
