#pragma once

class TCPStreamObserver 
{
public:
	virtual VOID StatusCallback(DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationSize ) { }
};


class TCPStream
{
protected:
	TCPStreamObserver * mObserver;

public:
	VOID SetObserver( TCPStreamObserver * observer ) {
		mObserver = observer;
	}

	virtual DWORD Initialize( DWORD dwFlags ) = 0;
	virtual VOID Close( DWORD reason ) = 0;
	virtual DWORD Connect( const CHAR * pszHost, SHORT wPort ) = 0;
	virtual INT Receive( BYTE * pBuffer, DWORD dwSize) = 0;
	virtual INT Send( const BYTE * pBuffer, DWORD dwSize) = 0;
};
