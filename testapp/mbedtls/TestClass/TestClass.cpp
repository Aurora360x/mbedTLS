#pragma once
#include "stdafx.h"
#include "TestClass.h"
#include "HttpManager/HttpManager.h"

static HttpManager httpClient;
static HTTPCallbackClass httpCallbacks;

VOID HTTPCallbackClass::DownloadComplete( HTTPResponseConstPtr httpRequestInfo )
	{
		if( httpRequestInfo->IsValid() == FALSE )
		{
			printf( "HTTPResponse is not valid!\n" );
		}
		else
		{
			printf( "Base Url:  %s\n", httpRequestInfo->GetBaseUrl().c_str() );
			printf( "Full Url:  %s\n", httpRequestInfo->GetFullUrl().c_str() );
		}

		if( httpRequestInfo->GetTag() == "TestRemoteFileUpload32kb" )
		{
			printf("TestRemoteFileUpload32kb - Successful\n" );
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestRemoteFileUpload32kbWithPostVars" )
		{
			printf("TestRemoteFileUpload32kbWithPostVars - Successful\n" );
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestRemotePOSTData" )
		{
			printf("TestRemotePOSTData - Successful\n" );
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestRemotePUTData" )
		{
			printf("TestRemotePUTData - Successful\n" );
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestRemotePATCHData" )
		{
			printf("TestRemotePATCHData - Successful\n" );
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestRemoteDELETEData" )
		{
			printf("TestRemoteDELETEData - Successful\n" );
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestRemotePOST" )
		{
			printf("TestRemotePOST - Successful\n" );
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestRemoteOPTIONS" )
		{
			printf("TestRemoteOPTIONS - Successful\n" );
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestBadConnection" )
		{
			if( httpRequestInfo->GetErrorCode() == HTTP_ERR_CONNFAILED )
			{
				printf("TestBadConn - Successful\n" );
			} else {
				printf("TestBadConn - Failed\n" );
			}
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestStatusCode" )
		{
			std::string resp  = sprintfaA("TestStatusCode - %d - ", httpRequestInfo->GetContentId() );
			if( httpRequestInfo->GetStatusCode() == httpRequestInfo->GetContentId() && httpRequestInfo->GetOutputSize() == 0 ) {
				printf("%s Successful\n", resp.c_str() );
			} else {
				printf("%s Failed\n", resp.c_str() );
			}
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestChunk" )
		{
			if( httpRequestInfo->GetOutputSize() == 1024 ) {
				printf("TestChunk - Successful\n" );
			} else {
				printf("TestChunk - Failed\n" );
			}
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestSetCookie" )
		{
			if( httpRequestInfo->GetStatusCode() == 200 ) 
			{
				//httpRequestInfo->GetOutputBuffer().print_buffer();
				printf("TestSetCookie - Successful\n" );
			} else {
				printf("TestSetCookie - Failed\n" );
			}
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestCookie" )
		{
			if( httpRequestInfo->GetStatusCode() == 200 ) 
			{
				//httpRequestInfo->GetOutputBuffer().print_buffer();
				printf("TestCookie - Successful\n" );
			} else {
				printf("TestCookie - Failed\n" );
			}
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestLocalGET" )
		{
			if( httpRequestInfo->GetStatusCode() == 200 ) {
				printf("TestLocalGET - Successful\n" );
			} else {
				printf("TestLocalGET - Failed\n" );
			}
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestRemoteHTTPSGET" )
		{
			if( httpRequestInfo->GetStatusCode() == 200 ) {
				httpRequestInfo->GetOutputBuffer().print_buffer();
				printf("TestRemoteHTTPSGET - Successful\n" );
			} else {
				printf("TestRemoteHTTPSGET - Failed\n" );
			}
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestRemoteGET" )
		{
			if( httpRequestInfo->GetStatusCode() == 200 ) {
				printf("TestRemoteGET - Successful\n" );
			} else {
				printf("TestRemoteGET - Failed\n" );
			}
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestMD5Verify" )
		{
			if( httpRequestInfo->ValidateOutputMD5FromServer() == TRUE ) {
				printf("TestMD5Verify - Successful\n" );
			} else {
				printf("TestMD5Verify - Failed\n" );
			}
			return;
		}
		else if( httpRequestInfo->GetTag() == "TestDownloadSpeed" )
		{
			if( httpRequestInfo->GetStatusCode() == 200 )
			{
				FLOAT bytesPerSecond = httpRequestInfo->GetDownloadSpeed();
				printf("TestDownloadspeed - Download Speed:  %4.2f MB/s (%4.2f Mbps)\n", bytesPerSecond / 8.0f, bytesPerSecond );

				StringToFileA( sprintfaA( "Download Speed:  %4.2f MB/s (%4.2f Mbps)", bytesPerSecond / 8.0f, bytesPerSecond ), "game:\\log.txt" );
			}
			else 
			{
				printf("TestDownloadspeed - failed\n" );
			}
			return;
		}
	}

// TEST METHODS
HTTPEndpoint * TestChunk()
{
	const std::string url = "http://httpbin.org/stream-bytes/1024?seed=1&chunk_size=256";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_GET, url.c_str(), "TestChunk" );
	ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.Chunk.bin" );
	return ep;
}
HTTPEndpoint * TestSetCookie()
{
	const std::string& url = "http://dfr.Hydrocon.com/cookies.php";
	//const std::string& url = "http://www.xboxunity.net";
	//const std::string url = "http://httpbin.org/cookies/set?sessId=0123456789ABCDEF&username=MaesterRowen";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_GET, url.c_str(), "TestSetCookie" );
	//ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.Cookie.bin" );
	return ep;
}
HTTPEndpoint * TestCookie()
{
	const std::string& url = "http://dfr.hydrocon.com/cookies.php";
	//const std::string url = "http://httpbin.org/cookies";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_GET, url.c_str(), "TestCookie" );
	//ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.Cookie.bin" );
	return ep;
}
HTTPEndpoint * TestLocalGET()
{
	const std::string url = "http://192.168.1.25:8082/5MB.zip";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_GET, url.c_str(), "TestLocalGET" );
	ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.LocalGET.bin" );
	return ep;
}
HTTPEndpoint * TestRemoteGET()
{
	//const std::string url = "http://napi.dabee.ca/title/415608C3/cover/59d4ef21f8311e2cedb4a92b/download";
	const std::string url = "http://httpbin.org/get?test=fun";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_GET, url.c_str(), "TestRemoteGET" );
	ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.RemoteGET.bin" );
	return ep;
}
HTTPEndpoint * TestRemoteHTTPSGET()
{
	//const std::string url = "https://www.realmodscene.com";
	const std::string url = "https://raw.githubusercontent.com/curl/curl/master/lib/cookie.c";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_GET, url.c_str(), "TestRemoteHTTPSGET" );
	//ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.RemoteHTTPSGET.bin" );
	return ep;
}
HTTPEndpoint * TestMD5Verify( BOOL forceFail = FALSE )
{
	const std::string url = forceFail == TRUE ? "http://httpbin.org" : "http://napi.dabee.ca/title/415608C3/cover/59d4ef21f8311e2cedb4a92b/download";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_GET, url.c_str(), "TestMD5Verify" );
	ep->SetMD5HashVerification(TRUE);
	ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.MD5Verify.bin" );
	return ep;
}
HTTPEndpoint * TestDownloadSpeed()
{
	const std::string url = "http://192.168.1.25:8082/20MB.zip";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_GET, url.c_str(), "TestDownloadSpeed" );
	ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.DownloadSpeed.bin" );
	return ep;
}
HTTPEndpoint * TestStatusCode(DWORD statusCode)
{
	const std::string url = "http://httpbin.org/status/" + sprintfaA("%d", statusCode );
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_GET, url.c_str(), "TestStatusCode" );
	ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.StatusCode.bin" );
	ep->SetContentId(statusCode);
	return ep;
}
HTTPEndpoint * TestBadConnection()
{
	const std::string url = "http://192.168.1.26/get";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_GET, url.c_str(), "TestBadConnection" );
	ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.BadConnection.bin" );
	return ep;
}
HTTPEndpoint * TestRedirect()
{
	const std::string url = "http://httpbin.org/redirect/1";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_GET, url.c_str(), "TestRedirect" );
	ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.Redirect.bin" );
	return ep;
}
HTTPEndpoint * TestRemotePOST()
{
	const std::string url = "http://httpbin.org/post";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_POST, url.c_str(), "TestRemotePOST" );

	ep->AddFormData( "Key1", "Value1" );
	ep->AddFormData( "Key2", "Value2" );

	//ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.RemotePOST.bin" );
	return ep;
}
HTTPEndpoint * TestRemoteFileUpload32kb()
{
	const std::string url = "http://httpbin.org/post";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_POST, url.c_str(), "TestRemoteFileUpload32kb" );

	ep->SetInputFile( "game:\\32kb.zip" );
	ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\downloads\\Test.RemoteFileUpload32kb.bin" );

	return ep;
}
HTTPEndpoint * TestRemoteFileUpload32kbWithPostVars()
{
	const std::string url = "http://httpbin.org/post";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_POST, url.c_str(), "TestRemoteFileUpload32kbWithPostVars" );

	ep->SetInputFile( "game:\\32kb.zip" );
	ep->AddFormData( "FileName", "32kb.zip" );
	ep->AddFormData( "FileType", "ZIP File" );

	return ep;
}
HTTPEndpoint * TestRemotePOSTData()
{
	const std::string url = "http://httpbin.org/post";
	//const std::string url = "http://54.221.226.80/post";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_POST, url.c_str(), "TestRemotePOSTData" );

	const char * szPostData = "{ \"KeyName\" : \"ValueData\" }";
	
	ep->SetInputBuffer((const BYTE*)szPostData, strlen( szPostData ), TRUE );

	//ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.RemotePOST.bin" );
	return ep;
}
HTTPEndpoint * TestRemotePUTData()
{
	const std::string url = "http://httpbin.org/put";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_PUT, url.c_str(), "TestRemotePUTData" );

	const char * szPostData = "{ \"KeyName\" : \"ValueData\" }";
	
	ep->SetInputBuffer((const BYTE*)szPostData, strlen( szPostData ), TRUE );

	//ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.RemotePOST.bin" );
	return ep;
}
HTTPEndpoint * TestRemotePATCHData()
{
	const std::string url = "http://httpbin.org/patch";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_PATCH, url.c_str(), "TestRemotePATCHData" );

	const char * szPostData = "{ \"KeyName\" : \"ValueData\" }";
	
	ep->SetInputBuffer((const BYTE*)szPostData, strlen( szPostData ), TRUE );

	//ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.RemotePOST.bin" );
	return ep;
}
HTTPEndpoint * TestRemoteDELETEData()
{
	const std::string url = "http://httpbin.org/delete";
	//const std::string url = "http://54.221.226.80/delete";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_DELETE, url.c_str(), "TestRemoteDELETEData" );


	ep->SetInputBuffer("{\"KeyName\":\"ValueData\"}");

	//ep->SetOutputMode( HTTP_OUTPUTMODE_FILE, "game:\\Downloads\\Test.RemotePOST.bin" );
	return ep;
}

HTTPEndpoint * TestRemoteOPTIONS()
{
	const std::string url = "http://httpbin.org/anything";
	HTTPEndpoint * ep = httpClient.CreateEndpoint( HTTP_REQUEST_TYPE_OPTIONS, url.c_str(), "TestRemoteOPTIONS" );

	return ep;

}

#include "HttpManager/HttpClient/HTTPCookie.h"

DWORD RunThread( VOID )
{
	//HTTPDate date;
	//std::string timestamp = date.GetTimestamp();
	//printf("%s\n", timestamp.c_str() );
	//SYSTEMTIME sysTime = date.GetTimeFromTimestamp(timestamp);
	//Sleep(0);


	//BOOL loop1 = TRUE;
	//while(loop1)
	//{
	//	httpClient.Execute( TestSetCookie() );
		httpClient.Execute(TestRemoteHTTPSGET());
		Sleep(500);
	//	httpClient.Execute( TestCookie() );
	//	Sleep(250);
	//	httpClient.DumpInfo();
	//}
	//BOOL loop2 = TRUE;
	//while(loop2)
	//{
	//	Sleep(250);
	//}
	

	//httpClient.Execute( TestBadConnection() );
	//httpClient.Execute( TestRedirect() );
	//httpClient.Execute( TestRemotePOST() );
	//httpClient.Execute( TestRemotePOSTData() );
	//httpClient.Execute( TestRemoteFileUpload32kb() );
	//httpClient.Execute( TestRemoteFileUpload32kbWithPostVars() );

	// PUT, PATCH & DELETE Tests
	//httpClient.Execute( TestRemotePUTData() );
	//httpClient.Execute( TestRemotePATCHData() );
	//httpClient.Execute( TestRemoteDELETEData() );
	//httpClient.Execute( TestRemoteOPTIONS() );

	//// Run status code tests
	//httpClient.Execute( TestStatusCode(100) );
	//httpClient.Execute( TestStatusCode(101) );
	//httpClient.Execute( TestStatusCode(200) );
	//httpClient.Execute( TestStatusCode(204) );
	//httpClient.Execute( TestStatusCode(304) );
	
	//// Run tests
	//httpClient.Execute( TestSetCookie() );
	//httpClient.Execute( TestCookie() );
	//httpClient.Execute( TestChunk() );
	//httpClient.Execute( TestLocalGET() );
	//httpClient.Execute( TestRemoteGET() );
	//httpClient.Execute( TestMD5Verify(FALSE) );
	//httpClient.Execute( TestMD5Verify(FALSE) );
	//httpClient.Execute( TestDownloadSpeed() );

	return 0;

}

DWORD RunTests( VOID )
{
	XNetStartupParams xnsp;
	memset(&xnsp, 0, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;

	xnsp.cfgSockDefaultRecvBufsizeInK = 64; // default = 16 
	xnsp.cfgSockDefaultSendBufsizeInK = 64; // default = 16 

	INT iResult = XNetStartup( &xnsp );

	if( iResult != NO_ERROR )
	{
		return false;
	}

	// Get MAC Address and MD5 Hash it for our key
	BYTE * outBuffer = NULL; WORD outLen = 0UL;
	ExGetXConfigSetting(XCONFIG_SECURED_CATEGORY, XCONFIG_SECURED_MAC_ADDRESS, NULL, NULL, &outLen);
	outBuffer = new BYTE[outLen];
	ExGetXConfigSetting(XCONFIG_SECURED_CATEGORY, XCONFIG_SECURED_MAC_ADDRESS, outBuffer, outLen, NULL);
	BYTE md5Bytes[0x10]; ZeroMemory(md5Bytes, 0x10);
	XeCryptMd5( (const PBYTE)outBuffer, outLen, NULL, 0, NULL, 0, md5Bytes, 0x10 );
	SAFE_DELETE( outBuffer );	

	httpClient.SetCookieJar( "game:\\Downloads" );//, md5Bytes, 0x10 );
	httpClient.AddObserver(httpCallbacks);

	WSADATA wsaData;
	WSAStartup( MAKEWORD(2,2), &wsaData );

	DWORD threadId = 0;
	HANDLE hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)RunThread, NULL, CREATE_SUSPENDED, &threadId );
	SetThreadPriority( hThread, 0 );
	XSetThreadProcessor( hThread, 4 );
	ResumeThread( hThread );
	CloseHandle( hThread );

	return S_OK;
}