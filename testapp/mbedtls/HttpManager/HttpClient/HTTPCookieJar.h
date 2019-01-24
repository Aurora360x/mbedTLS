#pragma once
#include "HTTPCookie.h"
#include "Tools/Uri.h"

class HTTPCookieJar
{
private:
	// Private constructor
	HTTPCookieJar( const std::string& cookieJarPath, const BYTE * encryptionKey = NULL, DWORD encryptionKeySize = 0 );

	// Helper Methods
	BOOL matchDomain( const char * cookie_domain, const char * uri_hostname );
	BOOL matchPath( const char * cookie_path, const char * uri_path );
	std::string sanitizePath( const char * cookie_path );

	std::vector<HTTPCookiePtr> m_cookieJar;
	std::string m_cookieJarPath;
	BOOL m_encryptCookies;
	BYTE m_encryptionKey[XECRYPT_AES_KEY_SIZE];

public:
	DWORD GetCookieCount( VOID ) { return m_cookieJar.size(); }
	BOOL IsOpen() { return TRUE; }
	BOOL SaveCookieToDevice( HTTPCookieConstPtr& cookie );

	// Find matching cookies
	DWORD FindMatchingCookies( const Uri& urlComponents, std::vector<const HTTPCookiePtr> * pCookieList );

	// Static Create Method to allow us to error on bad inputs
	static std::shared_ptr<HTTPCookieJar> HTTPCookieJar::CreateCookieJar( const std::string& cookieJarPath, const BYTE * encryptionKey = NULL, DWORD encryptionKeySize = 0 );
	HTTPCookiePtr HTTPCookieJar::GetCookie( const std::string& name, const std::string& domain, const std::string& path, BOOL secured );
	BOOL SetCookie( const std::string& cookieString, const std::string& defaultDomain, const std::string& defaultPath );

	// Create session cookies
	HTTPCookieConstPtr AddSessionCookie( const std::string& name, const std::string& content, const std::string& domain, const std::string& path, BOOL secured, BOOL httpOnly );
	HTTPCookieConstPtr AddCookie( const std::string& name, const std::string& content, const std::string& domain, const std::string& path, const SYSTEMTIME& expiration, BOOL secured, BOOL httpOnly );
	HTTPCookieConstPtr AddPersistantCookie( const std::string& name, const std::string& content, const std::string& domain, const std::string& path, const SYSTEMTIME& expiration, BOOL secured, BOOL httpOnly );
};

typedef std::shared_ptr<HTTPCookieJar> HTTPCookieJarPtr;
