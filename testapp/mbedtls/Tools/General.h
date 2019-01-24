#pragma once
#include "Timer.h"

#define ISSPACE(x)  (isspace((int)  ((unsigned char)x)))
#define ISDIGIT(x)  (isdigit((int)  ((unsigned char)x)))
#define ISALNUM(x)  (isalnum((int)  ((unsigned char)x)))
#define ISXDIGIT(x) (isxdigit((int) ((unsigned char)x)))
#define ISGRAPH(x)  (isgraph((int)  ((unsigned char)x)))
#define ISALPHA(x)  (isalpha((int)  ((unsigned char)x)))
#define ISPRINT(x)  (isprint((int)  ((unsigned char)x)))
#define ISUPPER(x)  (isupper((int)  ((unsigned char)x)))
#define ISLOWER(x)  (islower((int)  ((unsigned char)x)))
#define ISASCII(x)  (isascii((int)  ((unsigned char)x)))

#define ISBLANK(x)  (int)((((unsigned char)x) == ' ') || \
                          (((unsigned char)x) == '\t'))

#define TOLOWER(x)  (tolower((int)  ((unsigned char)x)))


// General Methods
ULONGLONG GenerateTimeStamp( void );
HRESULT GetBytesString(BYTE* Data, UINT DataLen, CHAR* OutBuffer, UINT* OutLen);
std::string GenerateMd5String( const BYTE * data, DWORD dataLen );

// File Helper Methods
inline bool FileExists(const std::string& filename);
void RecursiveMkdir(const std::string& path);

// String Helper Methods
std::vector<std::string> SplitStr(const std::string& s, const std::string& delimiter, bool includeEmpty);
std::string URLencode(const std::string& c);
std::string char2hex( char dec );
std::string sprintfaA(const char *format, ...);
std::string TrimStr(const std::string& in, const std::string& trim, bool left, bool right);
void StringToFileA(const std::string &data, const std::string& filename);

bool isip(const char *domain);
int parsedate(const char * date, SYSTEMTIME * output );
off_t strtooff(const char *nptr, char **endptr, int base);