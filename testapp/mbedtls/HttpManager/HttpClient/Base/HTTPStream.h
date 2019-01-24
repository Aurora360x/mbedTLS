#pragma once
#include "TCPStream.h"

class HTTPStream : public TCPStream
{
private: 
	SOCKET fd;
	BOOL connected;
public:
	HTTPStream( TCPStreamObserver * observer = NULL )
	{
		// Constructor
		mObserver = observer;
		connected = FALSE;
		fd = INVALID_SOCKET;
	}

	DWORD Initialize( DWORD dwFlags )
	{
		return 0;
	}
	VOID Close( DWORD reason )
	{
		if( fd != INVALID_SOCKET )
		{
			shutdown( fd, SD_BOTH );
			closesocket(fd );
			fd = INVALID_SOCKET;
		}

		connected = FALSE;
	}
	DWORD Connect( const CHAR * pszHost, SHORT wPort )
	{
		connected = FALSE;
		int ret = 0;
		SOCKADDR_IN sockAddr = { 0 };
		sockAddr.sin_port = htons(wPort);
		sockAddr.sin_family = AF_INET;
		DWORD ip = 0;
		if( (ip = inet_addr( pszHost)) != INADDR_NONE )
		{
			sockAddr.sin_addr.S_un.S_addr = ip;
			fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
			if( fd == INVALID_SOCKET )
			{
				ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
				return ret;
			}

			// Make socket insecure
			BOOL bBroadcast = TRUE;
			if( setsockopt( fd, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )
			{
				DebugBreak();
			}

			if( connect( fd, (const struct sockaddr*)&sockAddr, sizeof(sockAddr)) == 0 )
			{
				ret = 0;
				connected = TRUE;
			}
			else
			{
				// Close
				closesocket( fd );
				ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
			}
		}
		else
		{
			HANDLE myEvent = WSACreateEvent();
			XNDNS * pxndns = NULL;
			INT iErr = XNetDnsLookup( pszHost, myEvent, &pxndns );
			while (pxndns->iStatus == WSAEINPROGRESS) 
			{
				WaitForSingleObject(myEvent, INFINITE);
			}
			CloseHandle( myEvent );

			for( DWORD x = 0; x < pxndns->cina; x++ )
			{
				sockAddr.sin_addr = pxndns->aina[x];
				fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
				if( fd == INVALID_SOCKET )
				{
					ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
					continue;
				}

				// Make socket insecure
				BOOL bBroadcast = TRUE;
				if( setsockopt( fd, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )
				{
					DebugBreak();
				}

				if( connect( fd, (const struct sockaddr*)&sockAddr, sizeof(sockAddr)) == 0 )
				{
					ret = 0;
					connected = TRUE;
					break;
				}

				// Close
				closesocket( fd );
				ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
			}

			// Free DNS
			XNetDnsRelease( pxndns );
			pxndns = NULL;
		}

		return ret;
	}
	INT Receive( BYTE * pBuffer, DWORD dwSize)
	{
		if( connect == FALSE ) return 0;

		DWORD bytesRead = 0;
		DWORD readSize = dwSize;

		BYTE * offset = &pBuffer[0];

		while(dwSize > 0 )
		{
			// Adjust our readsize to capture remaining bytes
			if( readSize > dwSize ) readSize = dwSize;

			// Receive data from the sockets
			int ret = recv( fd, (char*)offset, readSize, 0 );
			if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE ) {
				continue;
			}

			// Handle error codes
			if( ret < 0 ) break;
			if( ret == 0 ) break;

			// Adjust our offset by # of bytes read
			offset += ret;

			// Decrease the remaining byte to read & increase total bytes read
			dwSize -= ret;
			bytesRead += ret;
		};

		// Finally, return the total number of bytes read
		return bytesRead;
	}
	INT Send( const BYTE * pBuffer, DWORD dwSize )
	{
		if( connected == FALSE ) return 0;

		DWORD bytesSent = 0;
		DWORD sendSize = dwSize;

		const BYTE * offset = &pBuffer[0];

		while(dwSize > 0 )
		{
			// Adjust our readsize to capture remaining bytes
			if( sendSize > dwSize ) sendSize = dwSize;

			// Receive data from the sockets
			int ret = send( fd, (char*)offset, sendSize, 0 );
			if( ret == SOCKET_ERROR )
			{
				return -1;

			}
			//if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE ) {
			//	continue;
			//}

			// Handle error codes
			if( ret < 0 ) break;
			if( ret == 0 ) break;

			// Adjust our offset by # of bytes read
			offset += ret;

			// Decrease the remaining byte to read & increase total bytes read
			dwSize -= ret;
			bytesSent += ret;

		}

		// Finally, return the total number of bytes read
		return bytesSent;
	}
};