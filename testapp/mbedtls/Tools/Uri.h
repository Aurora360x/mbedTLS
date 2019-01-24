#pragma once
#include <string>
#include <algorithm>

struct Uri
{
public:
	std::string Protocol;
	std::string Host;
	DWORD Port;
	std::string Path;
	std::string ExtraInfo;

	// Helper Methods
	std::string GetFullUrl( VOID ) const { return Protocol + "://" + Host + sprintfaA(":%d", Port) + Path + ExtraInfo; }
	std::string GetBaseUrl( VOID ) const { return Protocol + "://" + Host + sprintfaA(":%d", Port); }

	static Uri Parse( const std::string& uri )
	{
		typedef std::string::const_iterator Itr;

		Uri result;	
		if( uri.length() == 0 ) return result;

		// Reset all our our outputs
		result.Protocol = "";
		result.Port = 0;
		result.Path = "";
		result.Host = "";
		result.ExtraInfo = "";

		
		Itr uriEnd = uri.end();
		
		// Get the query start
		Itr queryStart = std::find( uri.begin(), uri.end(), '?' );

		// Protocol
		Itr protocolStart = uri.begin();
		Itr protocolEnd = std::find( uri.begin(), uri.end(), ':' );

		if( protocolEnd != uriEnd )
		{
			std::string protocol = &*(protocolEnd);
			if(( protocol.length() > 3 ) && (protocol.substr(0, 3) == "://" ))
			{
				result.Protocol = std::string( protocolStart, protocolEnd );
				std::transform( result.Protocol.begin(), result.Protocol.end(), result.Protocol.begin(), ::tolower );
				protocolEnd += 3;
			}
			else 
			{
				protocolEnd = uri.begin();
			}
		}
		else
		{
			protocolEnd = uri.begin();
		}

		// Host
		Itr hostStart = protocolEnd;
		Itr pathStart = std::find( hostStart, uriEnd, '/' );
		Itr hostEnd = std::find(protocolEnd, (pathStart != uriEnd) ? pathStart : queryStart, ':' );
		result.Host = std::string( hostStart, hostEnd );
		std::transform( result.Host.begin(), result.Host.end(), result.Host.begin(), ::tolower );

		// Port
		if( (hostEnd != uriEnd ) && ((&*(hostEnd))[0] == ':' ))
		{
			hostEnd++;
			Itr portEnd = (pathStart != uriEnd ) ? pathStart : queryStart;
			std::string portVal  =std::string(hostEnd, portEnd );
			result.Port = strtoul( portVal.c_str(), NULL, 10 );
		}
		if( result.Port == 0 && strcmpi(result.Protocol.c_str(), "https" ) == 0 )
			result.Port = 443;
		else if (result.Port == 0 && strcmpi(result.Protocol.c_str(), "http" ) == 0 )
			result.Port = 80;
	

		// Path
		if( pathStart != uriEnd )
		{
			result.Path = std::string( pathStart, queryStart );
		}
		if( result.Path.empty() ) result.Path = "/";

		// Query String
		if( queryStart != uriEnd )
		{
			result.ExtraInfo = std::string( queryStart, uriEnd );
		}

		return result;
	}
};

