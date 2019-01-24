#pragma once
#include "stdafx.h"
#include "Application/HTTPTestApp.h"



LPDIRECT3DDEVICE9	g_pd3dDevice;

void main( void ) 
{
	// Set up our direct 3d device presentation parameters
	D3DPRESENT_PARAMETERS m_d3dpp;
	XMemSet( &m_d3dpp, 0, sizeof( D3DPRESENT_PARAMETERS ));

	m_d3dpp.BackBufferCount			= 0;
	m_d3dpp.BackBufferWidth			= APPLICATION_RESOLUTION_X;
	m_d3dpp.BackBufferHeight		= APPLICATION_RESOLUTION_Y;
	m_d3dpp.BackBufferFormat		= ( D3DFORMAT )MAKESRGBFMT( D3DFMT_R5G6B5 );
    m_d3dpp.MultiSampleType			= D3DMULTISAMPLE_NONE;
	m_d3dpp.MultiSampleQuality		= 0;
    m_d3dpp.EnableAutoDepthStencil	= FALSE; 
	m_d3dpp.DisableAutoBackBuffer	= TRUE;
	m_d3dpp.DisableAutoFrontBuffer	= TRUE;
	m_d3dpp.AutoDepthStencilFormat	= D3DFMT_D24S8;
    m_d3dpp.SwapEffect				= D3DSWAPEFFECT_DISCARD;
	m_d3dpp.PresentationInterval	= APPLICATION_INTERNAL;

	// Now we're going to set up our device creation flags
	DWORD m_dwDeviceCreationFlags = 0;
	m_dwDeviceCreationFlags			|= D3DCREATE_BUFFER_2_FRAMES | D3DCREATE_CREATE_THREAD_ON_0 | D3DCREATE_CREATE_THREAD_ON_1;

	// Now we need to create our direct 3d device
    LPDIRECT3D9 pD3D = NULL;
	pD3D = Direct3DCreate9( D3D_SDK_VERSION );
    if( D3D_OK != pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, m_dwDeviceCreationFlags, &m_d3dpp, ( ::D3DDevice** )&g_pd3dDevice ) )  {
		#ifdef _DEBUG
		  DebugBreak();
		#endif
		return;
    }

	// Set our global variables so ATG modules know whats up
	ATG::g_pd3dDevice = (ATG::D3DDevice*) g_pd3dDevice;
	
	// Release our D3D Interface as its no longer needed
	pD3D->Release(); pD3D = NULL;

	// Run our application
	CHTTPTestApp app;
	app.RunModule( g_pd3dDevice, m_d3dpp );
}







/*
#define GET_REQUEST "GET / HTTP/1.0\r\n\r\n"

int TestGET(const std::string& url)
{
	//std::string szUrl = "https://raw.githubusercontent.com/ARMmbed/mbedtls/development/programs/ssl/ssl_client1.c";
	//std::string szUrl = "https://httpbin.org/stream-bytes/1024?seed=1&chunk_size=64";
	std::string msgBuffer = "";

	TLSStream stream;
	if( stream.Initialize() == ERROR_SUCCESS )
	{
		Uri uri = Uri::Parse(url);

		if( stream.Connect( uri ) == TRUE )
		{
			printf("URL:  %s\n", url.c_str() );

			// Build and send our GET request
			const char * pszVerb = "GET";
			HTTPRequest req = HTTPRequest(pszVerb, std::string(uri.Path + uri.ExtraInfo).c_str(), "HTTP/1.1" );
			req.AddOptionalHeader( "Host", uri.Host.c_str());
			req.AddOptionalHeader( "Connection", "close" );
			//req.AddOptionalHeader( "Cookie", "theme=light; sessionToken=abc123" );
			req.SendRequest( stream );

			// Receive our header
			CHTTPHeader header;
			header.ReceiveHeader(stream);



			// RFC2616 4.4.1 Message Length
			DWORD statusCode = header.GetStatusCode();
			if( statusCode == 302 )
			{
				stream.Close();
				std::string newUrl = uri.Protocol + "://" + uri.Host + header.GetHeaderValue("Location");
				TestGET(newUrl);
			}

			if( statusCode < 200 || statusCode == 204 || statusCode == 304 || strcmpi(pszVerb, "HEAD") == 0 )
			{
				stream.Close();
				return 0;
			}

			// Determine the length of our message body
			BOOL isChunked = header.IsChunked();
			if( isChunked == TRUE )
			{
				// Declare a memory buffer to receive our data
				CMemoryBuffer memBuffer(2048);

				// First we need to read our 
				DWORD chunkSize = 0;
				do {
					bool readingChunk = true;
					std::string chunkHeader = "";
					while( 1 ) 
					{
						CHAR data = 0;
						if( stream.ReadData( (BYTE*)&data, 1 ) != 1 ) {
							DebugBreak();
						}
						if( data == '\r' ) continue;
						else if( data == '\n' ) {
							// Chunk Header received
							break;
						} else {
							chunkHeader += data;
						}
					}

					// Parse the chunk header to obtain chunk size
					size_t endPos = chunkHeader.find_first_of(";");
					if( endPos == std::string::npos ) {
						endPos = chunkHeader.find_first_of("\r\n");
					}

					// Parse out the chunk size
					std::string size = chunkHeader.substr(0, endPos);
					chunkSize = strtol( size.c_str(), NULL, 16 );

					// Now we need to read the chunk
					if( chunkSize > 0 ) 
						stream.ReadData( memBuffer, chunkSize);

					// Now we read the trailing CRLF
					BYTE trailingCRLF[2]; memset(trailingCRLF, 0, 2 );
					stream.ReadData( trailingCRLF, 2 );

				}while(chunkSize > 0);

				FILE * f = NULL;
				_unlink( "game:\\chunkdata.bin" );
				fopen_s( &f, "game:\\chunkdata.bin", "wb" );	
				fwrite(&memBuffer[0], memBuffer.length(), 1, f );
				fclose(f);
			}
			else
			{
				DWORD contentLength = header.GetContentLength();
				if( contentLength > 0 )
				{
					// Allocate a memory buffer and receive the data
					CMemoryBuffer memBuffer(contentLength);
					stream.ReadData( memBuffer, contentLength );

					// Convert the memory buffer to a string
					msgBuffer = std::string( (const char*)&memBuffer[0], memBuffer.length() );

					FILE * f = NULL;
					_unlink( "game:\\data.bin" );
					fopen_s( &f, "game:\\data.bin", "wb" );	
					fwrite(&memBuffer[0], memBuffer.length(), 1, f );
					fclose(f);
				}
				else
				{
					// Allocate a memory buffer and receive the data
					CMemoryBuffer memBuffer(2048);
					int ret = 512;

					// Receive data until the connection is closed (should only happen wtih HTTP/1.0 clients
					do {
						ret = stream.ReadData( memBuffer, ret );
					} while( ret != 0 );

					FILE * f = NULL;
					_unlink( "game:\\datastream.bin" );
					fopen_s( &f, "game:\\datastream.bin", "wb" );	
					fwrite(&memBuffer[0], memBuffer.length(), 1, f );
					fclose(f);
				}
			}
		}

		// Close the stream
		stream.Close();
	}

	// End
	return 0;
}
int TestClient(const std::string& url )
{
	CHTTPSClient client;

	HTTPEndpoint * ep = new HTTPEndpoint(url);
	if( ep != NULL )
	{
		ep->AddHeaderField("Content-Type", "application/json" );

		client.Execute( ep, "GET" );
	}

	return 0;
}
int TestPOST(const std::string& url )
{
	std::string msgBuffer = "";

	TLSStream stream;
	if( stream.Initialize() == ERROR_SUCCESS )
	{
		Uri uri = Uri::Parse(url);

		if( stream.Connect( uri ) == TRUE )
		{
			printf("URL:  %s\n", url.c_str() );

			// Build and send our POST request
			const char * pszVerb = "POST";
			HTTPRequest req = HTTPRequest(pszVerb, std::string(uri.Path + uri.ExtraInfo).c_str(), "HTTP/1.1" );
			req.AddOptionalHeader( "Host", uri.Host.c_str());
			req.AddOptionalHeader( "Connection", "close" );
			req.AddOptionalHeader( "Content-Type", "application/x-www-form-urlencoded" );

			std::string postVars = "mykey3=myvalue3&mykey4=myvalue4";
			req.AddOptionalHeader( "Content-Length", sprintfaA( "%d", postVars.length() ).c_str() );
			req.AddBody( postVars.c_str() );
			req.SendRequest( stream );

			// Receive our header
			CHTTPHeader header;
			header.ReceiveHeader(stream);

			// RFC2616 4.4.1 Message Length
			DWORD statusCode = header.GetStatusCode();
			if( statusCode == 302 )
			{
				stream.Close();
				std::string newUrl = uri.Protocol + "://" + uri.Host + header.GetHeaderValue("Location");
				TestPOST(newUrl);
			}

			if( statusCode < 200 || statusCode == 204 || statusCode == 304 || strcmpi(pszVerb, "HEAD") == 0 )
			{
				stream.Close();
				return 0;
			}

			// Determine the length of our message body
			BOOL isChunked = header.IsChunked();
			if( isChunked == TRUE )
			{
				// Declare a memory buffer to receive our data
				CMemoryBuffer memBuffer(2048);

				// First we need to read our 
				DWORD chunkSize = 0;
				do {
					bool readingChunk = true;
					std::string chunkHeader = "";
					while( 1 ) 
					{
						CHAR data = 0;
						if( stream.ReadData( (BYTE*)&data, 1 ) != 1 ) {
							DebugBreak();
						}
						if( data == '\r' ) continue;
						else if( data == '\n' ) {
							// Chunk Header received
							break;
						} else {
							chunkHeader += data;
						}
					}

					// Parse the chunk header to obtain chunk size
					size_t endPos = chunkHeader.find_first_of(";");
					if( endPos == std::string::npos ) {
						endPos = chunkHeader.find_first_of("\r\n");
					}

					// Parse out the chunk size
					std::string size = chunkHeader.substr(0, endPos);
					chunkSize = strtol( size.c_str(), NULL, 16 );

					// Now we need to read the chunk
					if( chunkSize > 0 ) 
						stream.ReadData( memBuffer, chunkSize);

					// Now we read the trailing CRLF
					BYTE trailingCRLF[2]; memset(trailingCRLF, 0, 2 );
					stream.ReadData( trailingCRLF, 2 );
				} while(chunkSize > 0);

				FILE * f = NULL;
				_unlink( "game:\\chunkdata.bin" );
				fopen_s( &f, "game:\\chunkdata.bin", "wb" );	
				fwrite(&memBuffer[0], memBuffer.length(), 1, f );
				fclose(f);
			}
			else
			{
				DWORD contentLength = header.GetContentLength();
				if( contentLength > 0 )
				{
					// Allocate a memory buffer and receive the data
					CMemoryBuffer memBuffer(contentLength);
					stream.ReadData( memBuffer, contentLength );

					// Convert the memory buffer to a string
					//msgBuffer = std::string( (const char*)&memBuffer[0], memBuffer.length() );

					FILE * f = NULL;
					_unlink( "game:\\data3.bin" );
					fopen_s( &f, "game:\\data3.bin", "wb" );	
					fwrite(&memBuffer[0], memBuffer.length(), 1, f );
					fclose(f);
				}
				else
				{
					// Allocate a memory buffer and receive the data
					CMemoryBuffer memBuffer(2048);
					int ret = 512;

					// Receive data until the connection is closed (should only happen wtih HTTP/1.0 clients
					do {
						ret = stream.ReadData( memBuffer, ret );
					} while( ret != 0 );

					FILE * f = NULL;
					_unlink( "game:\\datastream.bin" );
					fopen_s( &f, "game:\\datastream.bin", "wb" );	
					fwrite(&memBuffer[0], memBuffer.length(), 1, f );
					fclose(f);
				}
			}
		}

		// Close the stream
		stream.Close();
	}

	// End
	return 0;
}
*/