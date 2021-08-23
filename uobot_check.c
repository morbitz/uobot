#include <stdio.h>
#include <windows.h>
#include <wininet.h>

#define CHUNKSIZE 2048

unsigned int HDSerial=0;


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
        return 0;
	printf("Key: %X\n", HDSerial);
    sprintf(PHPUrl, "/auth.php?key=%X", HDSerial);
    hHttpOpen = InternetOpen(UserAgent, // Application identification
                            INTERNET_OPEN_TYPE_DIRECT,  // No proxy server access
                            NULL, // No name for proxy server
                            NULL, // No bypass addresses
                            0); 
    if(hHttpOpen==NULL)
        return 0;
    hHttpSession = InternetConnect( hHttpOpen, // Handle
                                    "www.guildakop.sozuera.com.br", // Server name
                                    80,// HTTP Port number
                                    NULL, // No username
                                    NULL, // No password
                                    INTERNET_SERVICE_HTTP, // Service or protocol
                                    0,// No flags
                                    0);// No context
    if(hHttpSession==NULL)
    {
        printf("[ERROR] hHttpSession\n");
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
        printf("[ERROR] hHttpRequest\n");
        return 0;
    }
    RetValue = HttpSendRequest(hHttpRequest, // Using the handle that was just created
                                NULL,// No additional headers
                                0, // No length required
                                0, // No optional data
                                0); // No length required
    if(RetValue==FALSE)
    {
        printf("[ERROR] RetValue\n");
        return 0;
    }
    if(!InternetReadFile(hHttpRequest, 
                         charBuffer,
                         CHUNKSIZE - 1,
                         &dwRead))
    {
		printf("ERROR: cant read\n");
		return 0;
	}
	charBuffer[dwRead] = '\0';
	if(strstr(charBuffer, "KEYOK")) return 1;
    return 0;
}

int main(int argc, char **argv)
{
    unsigned int Ret;
    Ret=CheckUobot();
	printf("Retorno: %d\n", Ret);
    return Ret;
}
