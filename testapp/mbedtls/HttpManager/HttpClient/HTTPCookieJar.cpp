#pragma once
#include "stdafx.h"
#include "HTTPTypes.h"
#include "HTTPCookieJar.h"

#define MAX_LINELENGTH 5000

#define MAX_NAME 4096
#define MAX_NAME_TXT "4095"

//void* operator new(std::size_t n)
//{
//	printf("[allocating %d bytes\n", n );
//    return malloc(n);
//}

std::shared_ptr<HTTPCookieJar> HTTPCookieJar::CreateCookieJar( const std::string& cookieJarPath, const BYTE * encryptionKey /*=NULL*/, DWORD encryptionKeySize /*= 0*/ )
{
	struct ctor_cookiejar : public HTTPCookieJar 
	{
		ctor_cookiejar(const std::string& cookieJarPath, const BYTE * encryptionKey, DWORD encryptionKeySize  ) 
			: HTTPCookieJar( cookieJarPath, encryptionKey, encryptionKeySize )
		{
		}
	};
	return std::make_shared<HTTPCookieJar>(ctor_cookiejar(cookieJarPath, encryptionKey, encryptionKeySize));
}

BOOL HTTPCookieJar::SaveCookieToDevice( HTTPCookieConstPtr& cookie )
{
	std::string pp = sprintfaA("%s %s %s %d", cookie->GetName().c_str(), cookie->GetDomain().c_str(), cookie->GetPath().c_str(), cookie->GetIsSecured() );
	std::string filePath = m_cookieJarPath + "\\" + GenerateMd5String( (const BYTE*)pp.c_str(), pp.length() ) + ".cookie";

	HANDLE hFile = CreateFile( filePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	//if( hFile == INVALID_HANDLE_VALUE )	{ return FALSE; }

	BOOL ret = FALSE;

	// Create our cookie contents
	std::string cookieFile = cookie->GenerateFileContents();

	// Encrypt cookies
	size_t cookieLen = cookieFile.length();
	if( m_encryptCookies == TRUE )
	{
		size_t encryptedLen = (cookieFile.length() + 0xF) & ~0xF;
		BYTE * encryptedData = (BYTE*)malloc(encryptedLen);
		if( encryptedData != NULL )
		{
			memset( encryptedData, 0, encryptedLen );
			memcpy( encryptedData, cookieFile.c_str(), cookieLen );

			XECRYPT_AES_STATE state;
			BYTE iv[0x10]; memset( iv, 0, 0x10 );
			XeCryptAesKey( &state, m_encryptionKey );
			XeCryptAesCbc( &state, encryptedData, encryptedLen, encryptedData, (PBYTE)iv, TRUE );

			DWORD bytesWritten = 0UL;
			WriteFile( hFile, encryptedData, encryptedLen, &bytesWritten, NULL );
			ret = TRUE;

			// Free the memory
			free(encryptedData);
			encryptedData = NULL;
		}
	}
	else 
	{
		// Write the string to file
		DWORD bytesWritten = 0UL;
		WriteFile( hFile, cookieFile.c_str(), cookieFile.length(), &bytesWritten, NULL );
		ret = TRUE;
	}

	// Close the file handle
	CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;


	// return successfully
	return ret;
}

BOOL HTTPCookieJar::matchDomain( const char * cookie_domain, const char * uri_hostname ) {
	size_t cookie_domain_len = strlen(cookie_domain);
	size_t uri_hostname_len = strlen(uri_hostname);
	if( uri_hostname_len < cookie_domain_len ) return FALSE;
	if( strcmpi( cookie_domain, uri_hostname + uri_hostname_len - cookie_domain_len ) != 0) return FALSE;
	if( uri_hostname_len == cookie_domain_len ) return TRUE;
	if( '.' == *(uri_hostname + uri_hostname_len - cookie_domain_len - 1)) return TRUE;
	return FALSE;
}

BOOL HTTPCookieJar::matchPath( const char *cookie_path, const char * uri_path ) {
	size_t cookie_path_len = strlen(cookie_path);
	if( 1 == cookie_path_len ) return TRUE;
	size_t uri_path_len = strlen(uri_path);
	if( uri_path_len < cookie_path_len ) return FALSE;
	if( strncmp( cookie_path, uri_path, cookie_path_len )) return FALSE;
	if( cookie_path_len == uri_path_len ) return TRUE;
	if( uri_path[cookie_path_len] == '/' ) return TRUE;
	return FALSE;
}

DWORD HTTPCookieJar::FindMatchingCookies( const Uri& urlComponents, std::vector<const HTTPCookiePtr> * pCookieList )
{
	DWORD cookieCounter = 0UL;

	typedef std::vector<HTTPCookiePtr>::const_iterator iter;
	for( iter itr = m_cookieJar.cbegin(); itr != m_cookieJar.cend(); ++itr )
	{
		const HTTPCookiePtr& item = (*itr);
		if( item == NULL || item->IsExpired() == TRUE ) continue;

		const std::string& cookie_domain = item->GetDomain();

		// First check the secured flag (requiring HTTPS protocol)
		if( item->GetIsSecured() == TRUE && strcmpi( urlComponents.Protocol.c_str(), "https" ) != 0 ) continue;

		// Do domain-matching
		if( matchDomain( cookie_domain.c_str(), urlComponents.Host.c_str() ) == FALSE ) continue;

		// Do path matching
		const std::string& cookie_path = item->GetPath();
		if( matchPath( cookie_path.c_str(), urlComponents.Path.c_str() ) == FALSE ) continue;

		// At this point, we have a cookie to apply to the request
		if( pCookieList ) pCookieList->push_back( item );

		cookieCounter++;
	}

	// Return the number of cookies found
	return cookieCounter;
}

HTTPCookieJar::HTTPCookieJar( const std::string& cookieJarPath, const BYTE * encryptionKey /*=NULL*/, DWORD encryptionKeySize /*= 0*/ )
{
	m_cookieJarPath = cookieJarPath;
	memset(m_encryptionKey, 0, XECRYPT_AES_KEY_SIZE);
	if( encryptionKey != NULL && encryptionKeySize > 0 ) {
		m_encryptCookies = TRUE;
		memcpy_s( m_encryptionKey, XECRYPT_AES_KEY_SIZE, encryptionKey, encryptionKeySize );
	}

	// First we need open each persistent cookie from the file system
	WIN32_FIND_DATA findFileData = { 0 };

	std::string searchPath = cookieJarPath + "\\*.cookie";

	HANDLE hSearch = FindFirstFile( searchPath.c_str(), &findFileData );
	if( hSearch == INVALID_HANDLE_VALUE ) return;
	do {

		// Not interested in subdirectories
		if( (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY ) {
			continue;
		}

		// Open this cookie
		std::string filePath = cookieJarPath + "\\" + findFileData.cFileName;
		HANDLE hFile = CreateFile( filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if( hFile == INVALID_HANDLE_VALUE )	continue;

		// Allocate buffer to hold cookie data
		DWORD fileSize = GetFileSize( hFile, NULL );
		CHAR * cookieBuffer = new CHAR[fileSize + 1];
		if( cookieBuffer != NULL ) 
		{		
			// Null terminate char buffer
			cookieBuffer[fileSize] = 0x0;

			// Read hte file into the buffer
			DWORD bytesRead = 0UL;
			if( ReadFile( hFile, cookieBuffer, fileSize, &bytesRead, NULL ) == TRUE && bytesRead == fileSize )
			{
				// Decrypt Buffer
				if( m_encryptCookies == TRUE )
				{
					XECRYPT_AES_STATE state;
					BYTE iv[0x10]; memset( iv, 0, 0x10 );
					XeCryptAesKey( &state, m_encryptionKey );
					XeCryptAesCbc( &state, (const PBYTE)cookieBuffer, fileSize, (PBYTE)cookieBuffer, (PBYTE)iv, FALSE );
				}

				// Data was successfully read so let's try to create a cookie from this data
				HTTPCookiePtr cookie = HTTPCookie::CreateFromBuffer( cookieBuffer, strlen(cookieBuffer) );
				if( cookie != NULL )
				{
					if( cookie->IsExpired() == TRUE ) 
					{
						// Delete this cookie from the system
						CloseHandle( hFile );
						hFile = INVALID_HANDLE_VALUE;
						DeleteFile(filePath.c_str());

						// Notify log
						HTTPLog("Cookie %s for %s%s is expired and has been deleted.", cookie->GetName().c_str(), cookie->GetDomain().c_str(), cookie->GetPath().c_str() );
					}
					else 
					{
						// Add this cookie to our cookie list
						m_cookieJar.push_back(cookie);
					}
				} else {
					HTTPLog("Cookie %s is not valid.  Ignoring.", filePath.c_str() );
				}
			}

			// Free allocated memory
			delete [] cookieBuffer;
			cookieBuffer = NULL;
		}

		// Close our open file handle
		if( hFile != INVALID_HANDLE_VALUE ) {
			CloseHandle(hFile);
		}

		// Check the next file
	} while( FindNextFile( hSearch, &findFileData ));

	// Close search handle
	CloseHandle(hSearch);
	hSearch = NULL;
}

std::string HTTPCookieJar::sanitizePath( const char * cookie_path )
{
	// Duplicate our string
	char * new_path = strdup( cookie_path );
	if( new_path == NULL ) return NULL;

	// Check if the site put the path in quotes
	size_t len = strlen(new_path);
	if(new_path[0] == '\"' ) {
		// Remove the leading quote
		memmove( (void*)new_path, (const void*)(new_path + 1), len );
		len--;
	}

	// If the path ends in a quote, then remove that
	if( len > 0 && (new_path[len - 1] == '\"' )) {
		new_path[len - 1] = 0x00;
		len--;
	}


	// RFC 6256 5.2.4 - The Path Attribute
	if( new_path[0] != '/') {
		// Let cookie_path be the default path
		free(new_path);
		return "/";
	}

	// Remove the trailing / from path
	if( len > 1 && new_path[len - 1] == '/' ) {
		new_path[len - 1] = 0x0;
		len--;
	}

	// Generate a new string object with our char*
	std::string ret = std::string( new_path, len );

	// Free the char*
	free(new_path);

	// Return string
	return ret;
}

BOOL HTTPCookieJar::SetCookie( const std::string& cookieString, const std::string& defaultDomain, const std::string& defaultPath )
{
	// Setup default cookie values for later use
	std::string cName = "", cContent = "", cPath = "", cDomain = "", cExpireStr = "", cMaxAge = "";
	BOOL cSecured = FALSE, cHttpOnly = FALSE, cTailMatch = FALSE, cBadCookie = FALSE;
	SYSTEMTIME cExpires = { 0 };

	// Define some temporary bufffers
	char key[MAX_NAME], value[MAX_NAME];
	const char * lineptr = cookieString.c_str();

	// Calculate the length of our cookie string
	size_t linelength = cookieString.length();
	if( linelength > MAX_LINELENGTH ) {
		#if HTTP_DEBUG_WARNING	
		  HTTPLog( "Cookie length of %d exceeds the maximum length and is being discarded.", linelength );
		#endif
		return FALSE;
	}

	// Find the first instance of ';' in our cookie string
	const char * semiptr = strchr( lineptr, ';' );
	while(*lineptr && ISBLANK( *lineptr )) lineptr++;		// Skip any blank spaces or tabs

	// Store the intial offset into the cookie string
	const char * ptr = lineptr;
	do
	{
		key[0] = value[0] = 0; // Initliaze the work buffers

		// Read in text wtih specified format to get our key and value
		if( 1 <= sscanf(ptr, "%" MAX_NAME_TXT "[^;\r\n=] =%" MAX_NAME_TXT "[^;\r\n]", key, value))
		{
			BOOL done = FALSE, sep = FALSE;

			// Determine the string length of our key and value
			size_t klen = strlen(key);
			size_t vlen = strlen(value);

			// Check if our key or value fields are too long
			if( klen >= (MAX_NAME - 1) || vlen >= (MAX_NAME - 1) || ((klen + vlen) > MAX_NAME) ) {
				// The cookie has fields that are exceeding limit
				#if HTTP_DEBUG_WARNING
				  HTTPLog("The cookie key-value pairs exceed the maximum character limit and is being discarded." );
				#endif
				return FALSE;
			}

			// Grab a pointer to the end of our key
			const char * endofk = &ptr[ klen ];

			// Determine if the we have an '=' character
			sep = (*endofk == '=') ? TRUE : FALSE;

			// Trim the key to remove and spaces
			if( klen > 0 ) {
				endofk--;
				if( ISBLANK( *endofk ) ) {
					while( *endofk && ISBLANK(*endofk) ) {
						endofk--;
						klen--;
					}
					key[klen] = 0;
				}
			}

			// Trim the value to remove any trailing whitespaces
			while( vlen && ISBLANK(value[vlen-1])) {
				value[vlen - 1] = 0;
				vlen--;
			}

			// Trim the value to remove any leading whitespaces
			const char * valueptr = value;
			while( *valueptr && ISBLANK( *valueptr ) ) {
				valueptr++;
			}

			// If the cookie name has not yet been set and we have a value, then let's store it
			if( cName.empty() && sep == TRUE ) {
				cName = key;
				cContent = valueptr;

				// If either the name or content are empty, then this cookie is not valid
				if( cName.empty() || cContent.empty() ) {
					cBadCookie = TRUE;
					break;
				}
			} else if( vlen == 0 ) {
				// No values were set, so this must be an attribute of some form

				done = TRUE;
				if( strcmpi( "secure", key ) == 0 ) {
					cSecured = TRUE;		
				} else if( strcmpi( "httponly", key ) == 0 ) {
					cHttpOnly = TRUE;
				} else if( sep )
				{
					done = FALSE;
				}
			}

			// If we do have a value and our name is already figured out
			// then we are left with a few more attributes to parse
			if( done ) {
				// Do nothing
			}
			else if( strcmpi( "path", key ) == 0) 
			{
				// Store the cookie path
				cPath = sanitizePath(valueptr);
				if( cPath.empty() ) {
					cBadCookie = TRUE;
					break;
				}
			}
			else if( strcmpi( "domain", key ) == 0 )
			{
				// Ignore any leading dot prefix
				if( '.' == valueptr[0] ) valueptr++;

				bool is_ip = isip( defaultDomain.empty() == false ? defaultDomain.c_str() : valueptr );
				if( defaultDomain.empty() || (is_ip && strcmpi( valueptr, defaultDomain.c_str() ) != 0 ) 
					|| (!is_ip && matchDomain( valueptr, defaultDomain.c_str() )))
				{
					// Store the cookie domain
					cDomain = valueptr;
					if( cDomain.empty() ) {
						cBadCookie = TRUE;
						break;
					}

					// Transform the domain to all lowercase
					std::transform( cDomain.begin(), cDomain.end(), cDomain.begin(), ::tolower );

					// Set our tailmatch flag
					if( !is_ip ) cTailMatch = TRUE;
				}
				else 
				{
					cBadCookie = TRUE;
					#if HTTP_LOG_WARNING
					  HTTPLog( "Skipped cookie with bad domain-match domain:  %s", valueptr );
					#endif
				}
			}
			else if( strcmpi( "max-age", key ) == 0 )
			{
				// Store the Max-Age attribute
				cMaxAge = valueptr;
				if( cMaxAge.empty() )
				{
					cBadCookie = TRUE;
					break;
				}
			}
			else if( strcmpi( "expires", key ) == 0 )
			{
				// Store the Expires attribute (as a string)
				cExpireStr = valueptr;
				if( cExpireStr.empty() )
				{
					cBadCookie = TRUE;
					break;
				}
			}
		}

		// If there was no semicolon found previously, then skip and continue
		if( !semiptr || !*semiptr ) {
			semiptr = NULL;
			continue;
		}

		// Adjust our ptr to start just past the last semi colon
		ptr = semiptr + 1;

		// Skip any blanks while setting up the new ptr position
		while( *ptr && ISBLANK(*ptr) ) ptr++;

		// Using the new string, find the next ';'
		semiptr = strchr(ptr, ';' );
		if( !semiptr && *ptr ) {
			semiptr = strchr( ptr, '\0' );
		}
	} while (semiptr != NULL);

	// Now we start operating on our unparsed strings to get into a format our cookie likes
	if( cMaxAge.empty() == false )
	{
		// MaxAge is the number of seconds until a cookie expires
		long long val = _atoi64(cMaxAge.c_str() );
		FILETIME ft = { 0 }; SYSTEMTIME st = { 0 };
		GetSystemTime( &st );
		SystemTimeToFileTime( &st, &ft );
		LARGE_INTEGER * li = (LARGE_INTEGER*)&ft;
		li->QuadPart += val;
	}
	else if( cExpireStr.empty() == false )
	{
		// Parse the expiration date string to determine expiration of cookie
		int result = parsedate( cExpireStr.c_str(), &cExpires );
		if( result != 0 ) 
		{
			memset( &cExpires, 0, sizeof(SYSTEMTIME) );
		}
	}

	// Apply our default domain and path if one was not provided in cookie
	if( cBadCookie == FALSE && cDomain.empty() && !defaultDomain.empty() ) cDomain = defaultDomain;
	if( cBadCookie == FALSE && cPath.empty() && !defaultPath.empty() ) cPath = sanitizePath(defaultPath.c_str());

	// Finally, the cookie string has been fully parsed;  let's add the cookie to
	// our cookie set for later retrieval
	if( cBadCookie == TRUE ) {
		#if HTTP_DEBUG_ERROR
		  HTTPLog("Unable to parse cookie string:  %s", cookieString.c_str() );
		#endif
		return NULL;
	}

	// Add this cookie
	return AddCookie( cName, cContent, cDomain, cPath, cExpires, cSecured, cHttpOnly ) != NULL ? TRUE : FALSE;
}

HTTPCookiePtr HTTPCookieJar::GetCookie( const std::string& name, const std::string& domain, const std::string& path, BOOL secured )
{
	// Searching for a matching cookie
	HTTPCookiePtr cookie = NULL;
	typedef std::vector<HTTPCookiePtr>::const_iterator iter;
	for( iter itr = m_cookieJar.cbegin(); itr != m_cookieJar.cend(); ++itr )
	{
		const HTTPCookieConstPtr& item = (*itr);
		if( item == NULL && item->IsExpired() == TRUE ) continue;

		// Compare Domains (case-insensitive)
		if( strcmpi( item->GetDomain().c_str(), domain.c_str() ) != 0 ) continue;

		// Compare Name (case-insensitive)
		if( strcmpi( item->GetName().c_str(), name.c_str() ) != 0 ) continue;

		// Compare Path (case-sensitive)
		if( strcmp( item->GetPath().c_str(), path.c_str() ) != 0 ) continue;

		// Compare Secured
		if( item->GetIsSecured() == secured ) continue;

		// At this point, we found a matching cookie in our list, so we need to return that as the base
		cookie = std::const_pointer_cast<HTTPCookie>(item);
		break;
	}

	// If we still don't have a valid cookie object, then let's create a new one
	if( cookie == NULL ) 
	{
		// First let's create our new cookie object
		SYSTEMTIME expiration = { 0 };
		cookie = HTTPCookie::Create(name, domain, path, secured);
		if( cookie == NULL ) {
			#if HTTP_DEBUG_VERBOSE	
			  HTTPLog( "Unable to create cookie" );
			#endif
			return NULL;
		}

		// Add this cookie to our cookie list
		m_cookieJar.push_back(cookie);
	}

	// Return our cookie
	return cookie;
}

HTTPCookieConstPtr HTTPCookieJar::AddCookie( const std::string& name, const std::string& content, const std::string& domain, const std::string& path, const SYSTEMTIME& expiration, BOOL secured, BOOL httpOnly )
{
	// Searching for a matching cookie
	HTTPCookiePtr cookie = NULL;
	typedef std::vector<HTTPCookiePtr>::iterator iter;
	for( iter itr = m_cookieJar.begin(); itr != m_cookieJar.end(); ++itr )
	{
		const HTTPCookieConstPtr& item = (*itr);
		if( item == NULL ) continue;

		// Compare Domains (case-insensitive)
		if( strcmpi( item->GetDomain().c_str(), domain.c_str() ) != 0 ) continue;

		// Compare Name (case-insensitive)
		if( strcmpi( item->GetName().c_str(), name.c_str() ) != 0 ) continue;

		// Compare Path (case-sensitive)
		if( strcmp( item->GetPath().c_str(), path.c_str() ) != 0 ) continue;

		// Compare Secured
		if( item->GetIsSecured() != secured ) continue;

		// At this point, we found a matching cookie in our list, so we need to return that as the base
		cookie = std::const_pointer_cast<HTTPCookie>(item);
	}

	// First let's create our new cookie object
	if( cookie == NULL )
	{
		cookie = HTTPCookie::Create(name, domain, path, secured);
		if( cookie == NULL ) {
			#if HTTP_DEBUG_VERBOSE	
				HTTPLog( "Unable to create cookie" );
			#endif
			return NULL;
		}

		// Let's add this cookie to our jar to be detected by future requests
		m_cookieJar.push_back(std::const_pointer_cast<HTTPCookie>(cookie));
	}

	// At this point, we have a new cookie (or retrieved a matching cookie)
	cookie->UpdateContent( content );
	cookie->UpdateExpiration( expiration );

	// An expiration date was set, so we need to save this to the disk
	// to make it a permanent cookie
	if( expiration.wYear != 0 ) {
		SaveCookieToDevice(std::const_pointer_cast<const HTTPCookie>(cookie));
	}

	// Return our cookie
	return cookie;
}
