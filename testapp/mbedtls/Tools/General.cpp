#pragma once
#include "stdafx.h"
#include "General.h"

struct tzinfo {
  char name[5];
  int offset; /* +/- in minutes */
};

/* Here's a bunch of frequently used time zone names. These were supported
   by the old getdate parser. */
#define tDAYZONE -60       /* offset for daylight savings time */
static const struct tzinfo tz[]= {
  {"GMT", 0},              /* Greenwich Mean */
  {"UTC", 0},              /* Universal (Coordinated) */
  {"WET", 0},              /* Western European */
  {"BST", 0 tDAYZONE},     /* British Summer */
  {"WAT", 60},             /* West Africa */
  {"AST", 240},            /* Atlantic Standard */
  {"ADT", 240 tDAYZONE},   /* Atlantic Daylight */
  {"EST", 300},            /* Eastern Standard */
  {"EDT", 300 tDAYZONE},   /* Eastern Daylight */
  {"CST", 360},            /* Central Standard */
  {"CDT", 360 tDAYZONE},   /* Central Daylight */
  {"MST", 420},            /* Mountain Standard */
  {"MDT", 420 tDAYZONE},   /* Mountain Daylight */
  {"PST", 480},            /* Pacific Standard */
  {"PDT", 480 tDAYZONE},   /* Pacific Daylight */
  {"YST", 540},            /* Yukon Standard */
  {"YDT", 540 tDAYZONE},   /* Yukon Daylight */
  {"HST", 600},            /* Hawaii Standard */
  {"HDT", 600 tDAYZONE},   /* Hawaii Daylight */
  {"CAT", 600},            /* Central Alaska */
  {"AHST", 600},           /* Alaska-Hawaii Standard */
  {"NT",  660},            /* Nome */
  {"IDLW", 720},           /* International Date Line West */
  {"CET", -60},            /* Central European */
  {"MET", -60},            /* Middle European */
  {"MEWT", -60},           /* Middle European Winter */
  {"MEST", -60 tDAYZONE},  /* Middle European Summer */
  {"CEST", -60 tDAYZONE},  /* Central European Summer */
  {"MESZ", -60 tDAYZONE},  /* Middle European Summer */
  {"FWT", -60},            /* French Winter */
  {"FST", -60 tDAYZONE},   /* French Summer */
  {"EET", -120},           /* Eastern Europe, USSR Zone 1 */
  {"WAST", -420},          /* West Australian Standard */
  {"WADT", -420 tDAYZONE}, /* West Australian Daylight */
  {"CCT", -480},           /* China Coast, USSR Zone 7 */
  {"JST", -540},           /* Japan Standard, USSR Zone 8 */
  {"EAST", -600},          /* Eastern Australian Standard */
  {"EADT", -600 tDAYZONE}, /* Eastern Australian Daylight */
  {"GST", -600},           /* Guam Standard, USSR Zone 9 */
  {"NZT", -720},           /* New Zealand */
  {"NZST", -720},          /* New Zealand Standard */
  {"NZDT", -720 tDAYZONE}, /* New Zealand Daylight */
  {"IDLE", -720},          /* International Date Line East */
  /* Next up: Military timezone names. RFC822 allowed these, but (as noted in
     RFC 1123) had their signs wrong. Here we use the correct signs to match
     actual military usage.
   */
  {"A",  1 * 60},         /* Alpha */
  {"B",  2 * 60},         /* Bravo */
  {"C",  3 * 60},         /* Charlie */
  {"D",  4 * 60},         /* Delta */
  {"E",  5 * 60},         /* Echo */
  {"F",  6 * 60},         /* Foxtrot */
  {"G",  7 * 60},         /* Golf */
  {"H",  8 * 60},         /* Hotel */
  {"I",  9 * 60},         /* India */
  /* "J", Juliet is not used as a timezone, to indicate the observer's local
     time */
  {"K", 10 * 60},         /* Kilo */
  {"L", 11 * 60},         /* Lima */
  {"M", 12 * 60},         /* Mike */
  {"N",  -1 * 60},         /* November */
  {"O",  -2 * 60},         /* Oscar */
  {"P",  -3 * 60},         /* Papa */
  {"Q",  -4 * 60},         /* Quebec */
  {"R",  -5 * 60},         /* Romeo */
  {"S",  -6 * 60},         /* Sierra */
  {"T",  -7 * 60},         /* Tango */
  {"U",  -8 * 60},         /* Uniform */
  {"V",  -9 * 60},         /* Victor */
  {"W", -10 * 60},         /* Whiskey */
  {"X", -11 * 60},         /* X-ray */
  {"Y", -12 * 60},         /* Yankee */
  {"Z", 0},                /* Zulu, zero meridian, a.k.a. UTC */
};


enum assume {
	DATE_MDAY,
	DATE_YEAR,
	DATE_TIME
};

static const char * const wkday[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };


static const char * const weekday[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };
static const char * const month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


int checkday(const char * check, size_t len )
{
	int i;
	const char * const *what;
	bool found = false;
	if( len > 3 )
		what = &weekday[0];
	else 
		what = &wkday[0];
	for( i = 0; i < 7; i++ )
	{
		if(strcmpi( check, what[0]) == 0 ) {
			found = true;
			break;
		}
		what++;
	}
	return found ? i : - 1;
}

int checkmonth( const char * check )
{
	int i;
	const char * const *what;
	bool found = false;
	what = &month[0];
	for( i = 0; i < 12; i++ )
	{
		if( strcmpi( check, what[0]) == 0 ) {
			found = true;
			break;
		}
		what++;
	}
	return found ?  i : -1;
}	

int checktz( const char * check )
{
	unsigned int i;
	const struct tzinfo * what;
	bool found = false;

	what = tz;
	for( i = 0; i < sizeof(tz)/sizeof(tz[0]); i++ )
	{
		if( strcmpi( check, what->name ) == 0 ) {
			found = true;
			break;
		}
		what++;
	}
	return found ? what->offset * 60 : - 1;
}

void skip( const char ** date )
{
	// skip everything that aren't letters or numbers
	while(**date && !ISALNUM(**date))
		(*date)++;
}

int parsedate(const char * date, SYSTEMTIME * output )
{
	int wdaynum = -1;		// day of the week number, 0-6 (mon-sun)
	int monnum = -1;		// month number (0-11)
	int mdaynum = -1;		// day of the month (1-31)
	int hournum = -1;
	int minnum = -1;
	int secnum = -1;
	int yearnum = -1;
	int tzoff = -1;

	enum assume dignext = DATE_MDAY;
	
	const char *indate = date;	//Save the original pointer
	int part = 0;				// Max 6 parts

	while( *date && (part < 6)) {
		bool found = false;

		skip( &date );

		if( ISALPHA(*date)) 
		{	
			// A name coming up
			char buf[32] = "";
			size_t len;
			if( sscanf( date, "%31[ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz]", buf)) {
				len = strlen(buf);
			} else {
				len = 0;
			}
			
			if( wdaynum == -1 ) {
				wdaynum = checkday( buf, len );
				if( wdaynum != -1 ) found = true;
			}
			if( !found && (monnum == -1 )) {
				monnum = checkmonth(buf);
				if( monnum != -1 ) found = true;
			}
			if( !found && (tzoff == -1 )) {
				tzoff = checktz(buf);
				if( tzoff != -1 ) found = true;
			}

			if( !found )
				return -1;

			date += len;
		}
		else if(ISDIGIT(*date)) 
		{
			// a digit
			int val;
			char * end;
			int len = 0;

			  if((secnum == -1) &&
				 (3 == sscanf(date, "%02d:%02d:%02d%n",
							  &hournum, &minnum, &secnum, &len))) {
				/* time stamp! */
				len = 8;
				date += len;
			  }
			else if(( secnum == -1) && (2 == sscanf(date, "%02d:%02d%n", &hournum, &minnum, &len )))
			{
				date += len;
				secnum = 0;
			}
			else
			{
				long lval;
				int error;
				int old_errno;

				old_errno = errno;
				errno = 0;
				lval = strtol( date, &end, 10 );
				error = errno;
				if( errno != old_errno)
					errno = old_errno;

				if( error )
					return -1;
				
				val = (int)lval;

				if((tzoff == -1) &&
					((end - date) == 4) &&
					( val <= 1400 ) && 
					( indate < date ) &&
					((date[-1] == '+' || date[-1] == '-'))) 
				{
					found = true;
					tzoff = (val/100 * 60 + val % 100)*60;
					tzoff = date[-1]=='+'?-tzoff:tzoff;
				}


				// Support YYYYMMDD format
				if(((end - date) == 8 ) &&
					(yearnum == -1) &&
					(monnum == -1) &&
					(mdaynum == -1))
				{
					found = true;
					yearnum = val/10000;
					monnum = (val%10000)/100-1;
					mdaynum = val%100;
				}

				if( !found && (dignext == DATE_MDAY) && (mdaynum == -1))
				{
					if((val > 0) && (val < 32)) {
						mdaynum = val;
						found = true;
					}
					dignext = DATE_YEAR;
				}

				if(!found && (dignext == DATE_YEAR) && (yearnum == -1))
				{
					yearnum = val;
					found = true;
					if( yearnum < 1900 ) {
						if( yearnum > 70 ) {
							yearnum += 1900;
						} else {
							yearnum += 2000;
						}
					}
					if(mdaynum == -1)
						dignext = DATE_MDAY;
				}

				if(!found) return -1;

				date = end;
			}
		}

		part++;
	}

	if( -1 == secnum )
		secnum = minnum = hournum = 0;

	if(( -1 == mdaynum) ||
		(-1 == monnum) ||
		(-1 == yearnum))
	{
		return -1;
	}

	if(( mdaynum > 31) || (monnum > 11) ||
		 (hournum > 23) || (minnum > 59) || (secnum > 60))
	{
		return -1;
	}

	// If we have a valid 
	if( output ) {
		output->wSecond = secnum;
		output->wMinute = minnum;
		output->wHour = hournum;
		output->wDayOfWeek = wdaynum;
		output->wDay = mdaynum;
		output->wMonth = monnum + 1;	// Correct index for SYSTEMTIME
		output->wYear = yearnum;
		output->wMilliseconds = 0;		
	}

	return 0;
}

bool isip(const char *domain) {
  return inet_addr(domain) == INADDR_NONE ? false : true;
}

HRESULT GetBytesString(BYTE* Data, UINT DataLen, CHAR* OutBuffer, UINT* OutLen) {

	// Check our lenghts
	if(*OutLen < (DataLen * 2))
		return S_FALSE;

	*OutLen = DataLen * 2;

	// Output into our buffer as hex
	CHAR hexChars[] = "0123456789ABCDEF";
	for(UINT x = 0, y = 0; x < DataLen; x++, y+=2) {
		OutBuffer[y] = hexChars[(Data[x] >> 4)];
		OutBuffer[y + 1] = hexChars[(Data[x] & 0x0F)];
	}

	// All done =)
	return S_OK;
}

ULONGLONG GenerateTimeStamp( void) {
	SYSTEMTIME sysTime;
	GetLocalTime( &sysTime );
	FILETIME fileTime;
	SystemTimeToFileTime( &sysTime, &fileTime );
	ULONGLONG ulcurTime = 0;
	ulcurTime = fileTime.dwHighDateTime;
	ulcurTime = ulcurTime << 32 | fileTime.dwLowDateTime;
	return ulcurTime;
}

std::string GenerateMd5String( const BYTE * data, DWORD dataLen ) {
	// Create our MD5 hash
	BYTE md5bytes[0x10]; memset( md5bytes, 0, 0x10 );
	XeCryptMd5( (const PBYTE)data, dataLen, NULL, 0, NULL, 0, md5bytes, 0x10 );

	// Get the md5 bytes as a string
	CHAR md5str[41]; UINT outLen = 0x40;
	memset( md5str, 0, 41 );
	GetBytesString( md5bytes, 0x10, md5str, &outLen );

	// Return a string
	return std::string( md5str );
}

bool FileExists(const std::string& filename) {
	return (GetFileAttributes(filename.c_str()) != 0xFFFFFFFF);
}
void RecursiveMkdir(const std::string& path)
{
	std::string delimeter = "\\";
	std::vector<std::string> makePath = SplitStr( path, delimeter, false );
	if( makePath.size() > 1 )
	{
		std::string tempPath = makePath[0];
		for( DWORD idx = 1; idx < makePath.size(); idx++ )
		{
			tempPath = tempPath + delimeter + makePath.at(idx);
			if( FileExists( tempPath ) == false ) {
				if( CreateDirectory( tempPath.c_str(), NULL ) == FALSE ) {
					printf("Failed to create directory:  %s\n", tempPath.c_str() );
					break;
				}
			}
		}
	}
}

std::vector<std::string> SplitStr(const std::string& s, const std::string& delimiter, bool includeEmpty)
{
	std::vector<std::string> result;

	//trim the string to prevent empty leading and/or trailing entries
	std::string str = TrimStr(s, delimiter, true, true);
	for (std::string::size_type pos = str.find(delimiter); str.length() > 0 && pos != str.npos; pos = str.find(delimiter))
	{
		if (pos > 0 || includeEmpty)
		{
			result.push_back(str.substr(0, pos));
		}
		
		str = str.substr(pos + delimiter.length());
	}

	if (str.length() > 0) result.push_back(str);
	return result;
}
std::string URLencode(const std::string& c)
{
    
    std::string escaped= "";
    int max = c.length();
    for(int i=0; i<max; i++)
    {
        if ( (48 <= c[i] && c[i] <= 57) ||//0-9
             (65 <= c[i] && c[i] <= 90) ||//abc...xyz
             (97 <= c[i] && c[i] <= 122) || //ABC...XYZ
             (c[i]=='~' || c[i]=='!' || c[i]=='*' || c[i]=='(' || c[i]==')' || c[i]=='\'')
        )
        {
            escaped.append( &c[i], 1);
        }
        else
        {
            escaped.append("%");
            escaped.append( char2hex(c[i]) );//converts char 255 to string "ff"
        }
    }
    return escaped;
}

std::string char2hex( char dec )
{
    char dig1 = (dec&0xF0)>>4;
    char dig2 = (dec&0x0F);
    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
    if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=97-10;

    std::string r;
    r.append( &dig1, 1);
    r.append( &dig2, 1);
    return r;
}

std::string sprintfaA(const char *format, ...)
{
	char temp[16384];

	va_list ap;
	va_start (ap, format);
	_vsprintf_p(temp, 16384, format, ap);
	va_end (ap);

	return temp;
}

std::string TrimStr(const std::string& in, const std::string& trim, bool left, bool right)
{
	std::string s = in;

	if (s.empty() || trim.empty() || s.length() < trim.length()) return s;

	if (left)
	{
		for (size_t pos = s.find(trim); pos == 0; pos = s.find(trim))
		{
			s.erase(0, trim.length());
		}
	}
	
	if (right)
	{
		for (size_t pos = s.rfind(trim); pos == s.length() - trim.length(); pos = s.rfind(trim))
		{
			s.erase(s.length() - trim.length(), trim.length());
		}
	}
	
	return s;
};

void StringToFileA(const std::string &data, const std::string& filename)
{
	FILE * fp;
	if (fopen_s(&fp,filename.c_str(),"wb") == 0)
	{
		fwrite(data.c_str(),1,data.size(),fp);
		fclose(fp);
	}
}

off_t strtooff(const char *nptr, char **endptr, int base)
{

	return 0;
}