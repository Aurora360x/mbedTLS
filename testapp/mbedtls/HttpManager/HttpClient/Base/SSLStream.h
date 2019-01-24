#pragma once
#include "TCPStream.h"

#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_time       time 
#define mbedtls_time_t     time_t
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#endif

#define DEBUG_LEVEL 1

class SSLStream : public TCPStream
{
private:
	BOOL mInitialized;
	BOOL mConnected;

	// Declare our variables
	mbedtls_net_context server_fd;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	mbedtls_x509_crt cacert;

public:
	SSLStream( TCPStreamObserver * observer = NULL )
	{
		// Constructor
		mInitialized = FALSE;
		mObserver = observer;
		mConnected = FALSE;
	}

	DWORD Initialize( DWORD dwFlags )
	{
		// Inititialize the RNG and the session data
		mbedtls_net_init( &server_fd );
		mbedtls_ssl_init( &ssl );
		mbedtls_ssl_config_init( &conf );
		mbedtls_ctr_drbg_init( &ctr_drbg );
		mbedtls_x509_crt_init( &cacert );
		mbedtls_entropy_init( &entropy );

		#if defined(MBEDTLS_DEBUG_C)
		  mbedtls_debug_set_threshold( DEBUG_LEVEL );
		#endif

		// Seed the Random Number Generator
		const char * name = "aurora_ssl_client";
		int ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*)name, strlen(name) );
		if( ret < 0 ) {
			return E_FAIL;
		}

		// Initialize certificates
		ret = mbedtls_x509_crt_parse( &cacert, (const unsigned char*)mbedtls_test_cas_pem, mbedtls_test_cas_pem_len );
		if( ret < 0 ) {
			return E_FAIL;
		}

		// Flag our state as initialized
		mInitialized = TRUE;

		// Return successfully
		return ERROR_SUCCESS;
	}
	VOID Close( DWORD reason )
	{
		if( mConnected == TRUE ) {
			mbedtls_ssl_close_notify( &ssl );
			mConnected = FALSE;
		}

		if( mInitialized == TRUE ) {
			mbedtls_net_free( &server_fd );
			mbedtls_x509_crt_free( &cacert );
			mbedtls_ssl_free( &ssl );
			mbedtls_ssl_config_free( &conf );
			mbedtls_ctr_drbg_free( &ctr_drbg );
			mbedtls_entropy_free( &entropy );
			mInitialized = FALSE;
		}
	}
	DWORD Connect( const CHAR * pszHost, SHORT wPort )
	{
		std::string portVal = sprintfaA("%d", wPort );

		// Start our connection
		int ret = mbedtls_net_connect( &server_fd, pszHost, portVal.c_str(), MBEDTLS_NET_PROTO_TCP );
		if( ret < 0 ) {
			return E_FAIL;
		}

		// Setup defaults
		ret = mbedtls_ssl_config_defaults( &conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT );
		if( ret != 0 ) {
			return E_FAIL;
		}

		// Configure SSL
		mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_NONE );
		mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );
		mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
		#if defined(_DEBUG)
		  //mbedtls_ssl_conf_dbg( &conf, my_debug, stdout );
		#endif
		ret = mbedtls_ssl_setup( &ssl, &conf );
		if( ret != 0 ) {
			return E_FAIL;
		}

		// Finalize SSL Setup
		mbedtls_ssl_set_hostname( &ssl, pszHost );
		mbedtls_ssl_set_bio( &ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL );

		// Perform SSL Handshake
		while( (ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
		{
			if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
			{
				return E_FAIL;
			}
		}

		mConnected = TRUE;
		return ERROR_SUCCESS;
	}
	INT Receive( BYTE * pBuffer, DWORD dwSize )
	{
		// If we're not connected, then don't proceed
		if( mConnected == FALSE || mInitialized == FALSE ) return 0;

		DWORD bytesRead = 0;
		DWORD readSize = dwSize;

		BYTE * offset = &pBuffer[0];

		while(dwSize > 0 )
		{
			// Adjust our readsize to capture remaining bytes
			if( readSize > dwSize ) readSize = dwSize;

			// Receive data from the sockets
			int ret = mbedtls_ssl_read( &ssl, offset, readSize );
			if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE ) {
				continue;
			}

			// Handle error codes
			if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY ) break;
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
		// Verify that we are connected and initialized 
		if( mConnected == FALSE || mInitialized == FALSE ) return 0;

		DWORD bytesSent = 0;
		DWORD sendSize = dwSize;

		const BYTE * offset = &pBuffer[0];

		while(dwSize > 0 )
		{
			// Adjust our readsize to capture remaining bytes
			if( sendSize > dwSize ) sendSize = dwSize;

			// Receive data from the sockets
			int ret = mbedtls_ssl_write( &ssl, offset, sendSize );
			if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
			{
				return ret;
			}

			// Adjust our offset by # of bytes sent
			offset += ret;

			// Decrease the remaining byte to send & increase total bytes sent
			dwSize -= ret;
			bytesSent += ret;
		}

		// Finally, return the total number of bytes read
		return bytesSent;
	}
};