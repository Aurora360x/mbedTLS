#pragma once

#define AURORA_COOKIE_HEADER "AuroraCookie\n"

#define HTTPCOOKIE_FLAG_NONE		0x0000
#define HTTPCOOKIE_FLAG_SECURED		0x0001
#define HTTPCOOKIE_FLAG_HTTPONLY	0x0002

class HTTPCookie
{
friend class HTTPCookieJar;
friend class ctor_cookie;
private:
	// Private variables
	std::string mName;
	std::string mContent;
	std::string mDomain;
	std::string mPath;
	SYSTEMTIME mExpiration;
	DWORD mAttributeFlags;

	// Constructor
	HTTPCookie( const std::string& name, const std::string& domain, const std::string& path, BOOL secured )
		: mName(name), mDomain(domain), mPath(path) 
	{
		// Set our attribute flags
		mAttributeFlags = secured == TRUE ? HTTPCOOKIE_FLAG_SECURED : 0UL;
	}

public:
	static std::shared_ptr<HTTPCookie> Create(const std::string& name, const std::string& domain, const std::string& path, BOOL secured ) {
		struct ctor_cookie : public HTTPCookie 
		{
			ctor_cookie(const std::string& name, const std::string& domain, const std::string& path, BOOL secured ) 
				: HTTPCookie( name, domain, path, secured )
			{
			}
		};
		return std::make_shared<HTTPCookie>(ctor_cookie(name, domain, path, secured));
	}

	// Getters for cookie attributes
	const std::string& GetName( VOID ) const { return mName; }
	const std::string& GetContent( VOID ) const { return mContent; }
	const std::string& GetDomain( VOID ) const { return mDomain; }
	const std::string& GetPath( VOID ) const { return mPath; }
	const SYSTEMTIME& GetExpiration( VOID ) const { return mExpiration; }
	BOOL GetIsSecured( VOID ) const { return (mAttributeFlags & HTTPCOOKIE_FLAG_SECURED) ? TRUE : FALSE; }
	BOOL GetIsHttpOnly( VOID ) const { return (mAttributeFlags & HTTPCOOKIE_FLAG_HTTPONLY) ? TRUE : FALSE; }

	VOID UpdateContent( const std::string& content ) { mContent = content; }
	VOID UpdateExpiration( const SYSTEMTIME& expiration ) { mExpiration = expiration; }
	BOOL IsExpired( VOID ) const
	{ 
		// Get the current system time (UTC)
		SYSTEMTIME currentTime = { 0 };
		GetSystemTime( &currentTime );

		// Compare to the cookie epxiration time and fail if no match
		if( mExpiration.wYear == 0 ) return FALSE;

		// Convert System Time to File Times
		FILETIME fExpiration = { 0 }, fCurrentTime = {0};
		SystemTimeToFileTime( &mExpiration, &fExpiration );
		SystemTimeToFileTime( &currentTime, &fCurrentTime );

		// Compare and return result
		return CompareFileTime( &fExpiration, &fCurrentTime ) == -1 ? TRUE : FALSE;
	}

	// Helper Method to Convert the existing name and content to a Request Header format
	std::string ToCookieString( VOID ) { return mName + "=" + mContent; }
	std::string GenerateFileContents() const
	{
		FILETIME ft = { 0 };
		SystemTimeToFileTime( &mExpiration, &ft );

		// Return our output string
		return (std::string)(AURORA_COOKIE_HEADER + mName + "\n" + mContent + "\n" + mDomain + "\n" + mPath + "\n" + 
					sprintfaA("%d\n", mAttributeFlags) + 
					sprintfaA("%d\n", ft.dwHighDateTime ) +
					sprintfaA("%d\n", ft.dwLowDateTime ));
	}

	static std::shared_ptr<HTTPCookie> CreateFromBuffer( const char * cookieStr, size_t cookieLen )
	{
		// Copy our data temporarily
		CHAR * data = new CHAR[strlen(cookieStr) + 1];
		strcpy( data, cookieStr );

		if( memcmp( AURORA_COOKIE_HEADER, cookieStr, strlen(AURORA_COOKIE_HEADER) ) != 0 ) {
			// Invalid Cookie
			return NULL;
		}

		// We're going to loop through this cookie data, one line at a time
		std::string cHeader = strtok( data, "\n" );
		std::string cName = strtok( NULL, "\n" );
		std::string cContent = strtok( NULL, "\n" );
		std::string cDomain = strtok( NULL, "\n" );
		std::string cPath = strtok( NULL, "\n" );
		std::string cFlags = strtok( NULL, "\n" );
		std::string cExpireHigh = strtok( NULL, "\n" );
		std::string cExpireLow = strtok( NULL, "\n" );

		// Delete our temp buffer
		delete [] data;
		data = NULL;

		// Convert none string lines to correct format
		DWORD flags = strtoul( cFlags.c_str(), NULL, 10 ) & 0x0000FFFF;
		FILETIME ft = { 0 };  SYSTEMTIME st = { 0 } ;
		ft.dwHighDateTime = strtoul( cExpireHigh.c_str(), NULL, 10 );
		ft.dwLowDateTime = strtoul( cExpireLow.c_str(), NULL, 10 );
		FileTimeToSystemTime( &ft, &st );

		// Now we need to create our cookie
		std::shared_ptr<HTTPCookie> cookie = HTTPCookie::Create( cName, cDomain, cPath, ((flags & HTTPCOOKIE_FLAG_SECURED) == HTTPCOOKIE_FLAG_SECURED) ? TRUE : FALSE );
		if( cookie != NULL )
		{
			cookie->UpdateContent( cContent );
			cookie->UpdateExpiration( st );
		}

		// Return the cookie
		return cookie;
	}
};

typedef std::shared_ptr<const HTTPCookie> HTTPCookieConstPtr;
typedef std::shared_ptr<HTTPCookie> HTTPCookiePtr;