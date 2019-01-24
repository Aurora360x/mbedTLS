#pragma once
#include "HttpManager/HttpManager.h"

DWORD RunTests();

class HTTPCallbackClass : public IHttpClientObserver 
{
public:
	// Download Complete Callback
	VOID DownloadComplete( HTTPResponseConstPtr httpRequestInfo ) override;

};