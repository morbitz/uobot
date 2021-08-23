#include <stdio.h>
#include <windows.h>
#include <wininet.h>
#include "uobot_log.h"

#define CHUNKSIZE 2048

unsigned int HDSerial=0;

unsigned char UrlAddr[]=
"\xE7\xE7\xE7\xBE\xF7\xE5\xF9\xFC\xF4"
"\xF1\xFB\xFF\xE0\xBE\xE3\xFF\xEA\xE5"
"\xF5\xE2\xF1\xBE\xF3\xFF\xFD\xBE\xF2\xE2";


unsigned char GetUrl[]=
"\xBF\xF1\xE5\xE4\xF8\xBE\xE0"
"\xF8\xE0\xAF\xFB\xF5\xE9\xAD";

unsigned char *DesenCript(unsigned char *buf, int len)
{
	int x;
	for(x=0; x<len; x++)
		buf[x] ^= 0x90;
	return buf;
}


BOOL GetVolumeSerial(unsigned int *dwVolSerial) {
    BOOL bIsRetrieved;
    bIsRetrieved = GetVolumeInformation((LPCTSTR)"C:\\",(LPTSTR)NULL,(DWORD)NULL,(LPDWORD)dwVolSerial,(LPDWORD)NULL,(LPDWORD)NULL,(LPTSTR)NULL,(DWORD)NULL);
    return bIsRetrieved;
}

int CheckUobot(void)
{
    char UserAgent[]="Mozilla/4.0 (compatible; MSIE 4.01; Windows 98)";
    HINTERNET hHttpOpen = NULL;
    HINTERNET hHttpSession = NULL;
    HINTERNET hHttpRequest = NULL;
    BOOL RetValue=FALSE;
    char PHPUrl[100];
    char charBuffer[CHUNKSIZE];
    DWORD dwRead;

    if(GetVolumeSerial(&HDSerial)==FALSE)
	{
		LogPrint(3, "[AUTH] Nao foi possivel achar o serial do hd\n");
        return 0;
	}
    sprintf(PHPUrl, "%s%X", DesenCript(GetUrl, strlen(GetUrl)), HDSerial);
    hHttpOpen = InternetOpen(UserAgent, // Application identification
                            INTERNET_OPEN_TYPE_DIRECT,  // No proxy server access
                            NULL, // No name for proxy server
                            NULL, // No bypass addresses
                            0); 
    if(hHttpOpen==NULL)
	{
		LogPrint(1, "[AUTH] hHttpOpen\n");
        return 0;
	}
    hHttpSession = InternetConnect( hHttpOpen, // Handle
                                    DesenCript(UrlAddr, strlen(UrlAddr)), // Server name
                                    80,// HTTP Port number
                                    NULL, // No username
                                    NULL, // No password
                                    INTERNET_SERVICE_HTTP, // Service or protocol
                                    0,// No flags
                                    0);// No context
    if(hHttpSession==NULL)
    {
        LogPrint(1, "[AUTH] hHttpSession\n");
        return 0;
    }

    hHttpRequest = HttpOpenRequest(hHttpSession, // Handle from InternetConnect
                                NULL, // HTTP verb is 'GET'
                                PHPUrl,// CrackedUrl path
                                NULL, // Default version is 'HTTP/1.0'
                                NULL, // No referrer
                                NULL, // Only 'text/*' files are accepted
                                0, // No flags
                                0);
    if(hHttpRequest==NULL)
    {
        LogPrint(1, "[AUTH] hHttpRequest\n");
        return 0;
    }
    RetValue = HttpSendRequest(hHttpRequest, // Using the handle that was just created
                                NULL,// No additional headers
                                0, // No length required
                                0, // No optional data
                                0); // No length required
    if(RetValue==FALSE)
    {
        LogPrint(1, "[AUTH] RetValue\n");
        return 0;
    }
    if(!InternetReadFile(hHttpRequest, 
                         charBuffer,
                         CHUNKSIZE - 1,
                         &dwRead))
    {
		LogPrint(1, "[AUTH]: Nao foi possivel ler o site\n");
		return 0;
	}
	charBuffer[dwRead] = '\0';
	if(hHttpOpen)
		InternetCloseHandle(hHttpOpen);
	if(hHttpSession)
		InternetCloseHandle(hHttpSession);
	if(strstr(charBuffer, "KEYOK")) return 1;
    return 0;
}


