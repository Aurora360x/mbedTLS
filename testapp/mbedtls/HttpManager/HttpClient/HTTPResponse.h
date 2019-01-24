#pragma once
#include "HTTPTypes.h"
#include "HTTPHeader.h"
#include "Tools/MemoryBuffer.h"

class HTTPResponse
{
friend class HTTPEndpoint;
private:
	// Private Member Variables
	HTTP_REQUEST_TYPE mRequestType;
	HTTP_OUTPUTMODE mOutputMode;
	CHTTPHeader mResponseHeader;
	CMemoryBuffer mOutputBuffer;
	std::string mOutputFilePath;
	DWORD mOutputFileSize;
	std::string mOutputMD5Hash;
	FLOAT mDownloadTime;
	DWORD mErrorCode;
	BOOL mCanceled;
	Uri mUrlComponents;

	struct {
		HTTP_INPUTMODE Mode;
		std::string FilePath;
		DWORD FileSize;
		BYTE * DataBuffer;
		DWORD DataBufferSize;
		BOOL DataBufferIsPersistent;
	} mInputData;

	struct {
		std::string Tag;
		ULONGLONG	ContentId;
		DWORD_PTR	DataContext;
	} mUser;
 
public:
	// Url Retrieval Methods
	std::string GetFullUrl( VOID ) const { return mUrlComponents.GetFullUrl(); }
	std::string GetBaseUrl( VOID ) const { return mUrlComponents.GetBaseUrl(); }
	const Uri& GetUrlComponents( VOID ) const { return mUrlComponents; }

	// Client Variable Retrieval Methods
	const std::string& GetTag( VOID ) const { return mUser.Tag; }
	ULONGLONG GetContentId( VOID ) const { return mUser.ContentId; }
	DWORD_PTR GetUserData( VOID ) const { return mUser.DataContext; }

	// Response State Methods
	HTTP_REQUEST_TYPE GetRequestType( VOID ) const { return mRequestType; }
	const CHTTPHeader& GetResponseHeader( VOID ) const { return mResponseHeader; }
	BOOL IsCanceled( VOID ) const { return mCanceled; }
	DWORD GetStatusCode( VOID ) const { return mResponseHeader.GetStatusCode(); }
	HTTP_OUTPUTMODE GetOutputMode( VOID ) const { return mOutputMode; }
	HTTP_INPUTMODE GetInputMode( VOID ) const {return mInputData.Mode; }
	DWORD GetErrorCode( VOID ) const { return mErrorCode; }
	BOOL IsValid( VOID ) const { return (mCanceled == FALSE && mErrorCode == HTTP_ERR_OK) ? TRUE : FALSE; }

	// Input Buffer Methods
	BOOL IsInputDataBufferPesistent( VOID ) const { return mInputData.DataBufferIsPersistent; }
	const BYTE * GetInputBuffer( VOID ) const { return mInputData.DataBuffer; }
	const std::string& GetInputFilePath( VOID ) const { return mInputData.FilePath; }
	DWORD GetInputSize( VOID ) const { 
		if( mInputData.Mode == HTTP_INPUTMODE_FILE ) return mInputData.FileSize; 
		else if( mInputData.Mode == HTTP_INPUTMODE_MEMORY ) return mInputData.DataBufferSize;
		else return 0UL;
	}

	// Output Buffer Methods
	const CMemoryBuffer& GetOutputBuffer( VOID ) const { return mOutputBuffer; }
	const std::string& GetOutputFilePath( VOID ) const { return mOutputFilePath; }
	DWORD GetOutputSize( VOID ) const { return mOutputMode == HTTP_OUTPUTMODE_FILE ? mOutputFileSize : mOutputBuffer.length(); }

	// Download Speed Methods
	FLOAT GetDownloadTime( VOID ) const { return mDownloadTime; }
	FLOAT GetDownloadSpeed( VOID ) const { return (FLOAT)(GetOutputSize() * 8.0f / (1024.0f * 1024.0f) / (mDownloadTime)); }

	// MD5 Validation Methods
	BOOL ValidateOutputMD5( const std::string& md5string ) const
	{
		return strcmpi( md5string.c_str(), mOutputMD5Hash.c_str() ) == 0 ? TRUE : FALSE;
	}
	BOOL ValidateOutputMD5FromServer( VOID ) const { 
		return ValidateOutputMD5( mResponseHeader.GetHeaderValue( "X-Content-MD5" ) );
	}

	// Utility Methods
	VOID ResetValues( VOID )
	{
		mResponseHeader.Reset();
		mOutputBuffer.clear();

		mUser.Tag = "";
		mUser.ContentId = 0ULL;
		mUser.DataContext = NULL;

		mInputData.Mode = HTTP_INPUTMODE_NONE;
		mInputData.FilePath = "";
		mInputData.FileSize = 0UL;
		mInputData.DataBuffer = NULL;
		mInputData.DataBufferSize = 0UL;
		mInputData.DataBufferIsPersistent = FALSE;
	
		mErrorCode = HTTP_ERR_OK;
		mCanceled = FALSE;
		mDownloadTime = 0.0F;
		mOutputFilePath = "";
		mOutputFileSize = 0UL;
		mOutputMD5Hash = "";
		mOutputMode = HTTP_OUTPUTMODE_MEMORY;
	}

	// Constructor & Destructor
	HTTPResponse( VOID ) { ResetValues(); }
	virtual ~HTTPResponse( VOID ) { 
		// If we need to free our input databuffer, then do so
		if( mInputData.DataBufferIsPersistent == FALSE && mInputData.DataBuffer != NULL ) {
			free( mInputData.DataBuffer );
			mInputData.DataBuffer = NULL;
			mInputData.DataBufferSize = 0UL;
		}
	}
};

typedef std::shared_ptr<const HTTPResponse> HTTPResponseConstPtr;
typedef std::shared_ptr<HTTPResponse> HTTPResponsePtr;
