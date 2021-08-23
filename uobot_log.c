#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "uobot_handles.h"
#include "uobot_net.h"
#include "uobot_log.h"
#include "uobot_obj.h"
#include "uobot_jornal.h"
#include "uobot_target.h"
#include "uobot_spells.h"
#include "uobot_threads.h"
#include "uobot_commands.h"
#include "uobot.h"
#include <commctrl.h>
#include "resource.h"
#include "uobot_windows.h"

// Log File
FILE *logFile=0;
int LogFlag=0;

// Log functions
void LogDump(unsigned char *pBuffer, int length);
void LogPrint(int Type, const char *strFormat, ...);
void LogOpen(char *file);
void CloseLog(void);


// Stuff functions
void pack16(unsigned char *Buf, unsigned short x);
void pack32(unsigned char *Buf, unsigned int x);
unsigned short unpack16(const unsigned char *Buf);
unsigned int unpack32(const unsigned char *Buf);
int UnicodeToAscii(const char *UnicodeText, int Len, char *AsciiText);
int chartoint32(char *a, int split, int *c);


int UnicodeToAscii(const char *UnicodeText, int Len, char *AsciiText)
{
	int i = 0, NonAsciiCompatible = 0;

	for(i = 0; i < Len; i+=2)
	{
		if(UnicodeText[i] != 0x00)
			NonAsciiCompatible++;

        AsciiText[i/2] = UnicodeText[i+1];
	}

	AsciiText[i++/2] = '\0'; /* null terminate it */
    
	return NonAsciiCompatible;
}

unsigned int unpack32(const unsigned char *Buf)
{
    return (Buf[0] << 24) | (Buf[1] << 16) | (Buf[2] << 8) | Buf[3];
}

unsigned short unpack16(const unsigned char *Buf)
{
    return (Buf[0] << 8) | Buf[1];
}

void pack32(unsigned char *Buf, unsigned int x)
{
    Buf[0] = (unsigned char)(x >> 24);
    Buf[1] = (unsigned char)((x >> 16) & 0xff);
    Buf[2] = (unsigned char)((x >> 8) & 0xff);
    Buf[3] = (unsigned char)(x & 0xff);

	return;
}

void pack16(unsigned char *Buf, unsigned short x)
{
	Buf[0] = x >> 8;
	Buf[1] = x & 0xff;

	return;
}

void LogPrint(int Type, const char *strFormat, ...)
{
	char final[8000];
	va_list args;
	time_t tempo;
	struct tm *timeinfo;

	if(Type==0) return;

	time ( &tempo );
	timeinfo = localtime ( &tempo );

	va_start(args, strFormat);
	memset(final, 0, sizeof(final));
	vsprintf(final, strFormat, args);
	va_end(args);
	if(Type&0x02) {
		if(logFile)
		{
			fprintf(logFile, "[%02d:%02d:%02d] %s", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, final);
			fflush(logFile);
		}
	}

	final[strlen(final)-1] = 0;
	strcat(final, "\r\n");
	if(Type&0x01 && LogFlag == 0) TextBoxCat(0, 0, final);

	return;
}

void LogDump(unsigned char *pBuffer, int length)
{
	int actLine = 0, actRow = 0;
	if(!logFile) return;
	for(actLine = 0; actLine < (length / 16) + 1; actLine++)
	{
		fprintf(logFile, "%04x: ", actLine * 16);
		for(actRow = 0; actRow < 16; actRow++)
		{
			if(actLine * 16 + actRow < length) fprintf(logFile, "%02x ", (unsigned int)((unsigned char)pBuffer[actLine * 16 + actRow]));
			else fprintf(logFile, "-- ");
		}
		fprintf(logFile, ": ");
		for(actRow = 0; actRow < 16; actRow++)
		{
			if(actLine * 16 + actRow < length) fprintf(logFile, "%c", isprint(pBuffer[actLine * 16 + actRow]) ? pBuffer[actLine * 16 + actRow] : '.');
		}
		fprintf(logFile, "\n");
	}

	fprintf(logFile, "\n");
	fflush(logFile);
	return;
}


void LogOpen(char *file)
{
	if((logFile=fopen(file, "w"))==NULL)
	{
		return;
	}
}

void LogClose(void)
{
	if(logFile!=NULL)
	{
		fclose(logFile);
		logFile = 0;
	}
}


int chartoint32(char *a, int split, int *c)
{
	char *b=a;
	int d;
	b = strchr(a, split);
	d = b - a;
	*b = 0x00;
	*c=(int)atof(a);
	*b = (char)split;
	return d;
}


void Say(char *Texto, ...)
{
	unsigned char *SpeechPacket = NULL;
	unsigned short Len=0, Color = m_speechcolor, Font = 3;
	unsigned short x=0;
	char final[4096];
	va_list args;

	if(!IsConnected){
		LogPrint(1, "[SAY] Voce precisa estar conectado para falar\n");
		return;
	}

	va_start(args, Texto);
	memset(final, 0, sizeof(final));
	vsprintf(final, Texto, args);
	va_end(args);

	Len = (int)(12 + (strlen(final)*2) + 2);
	SpeechPacket = (unsigned char *)malloc(Len);

	SpeechPacket[0] = 0xAD;
    /* tamanho */
	pack16(SpeechPacket + 1, Len);
    /* normal talk mode */
	SpeechPacket[3] = 0; 
    /* cor pre-definida */
	pack16(SpeechPacket + 4, Color); 
    /* fonte pre-definida */
	pack16(SpeechPacket + 6, Font);
    /* linguagem: ingles */
    memcpy(SpeechPacket + 8, "ptb", 4); 
    /* unicode string */
    MultiByteToWideChar(CP_ACP, 0, final, -1, (unsigned short *)(SpeechPacket + 12), Len - 12);
	for(x=0; x < Len-12-2; x += 2)
		*((unsigned short *)(SpeechPacket + 12 + x)) = (SpeechPacket[12 + x] << 8) | SpeechPacket[12 + x +1];
//	strncpy((char*)(SpeechPacket + 8), final, strlen(final));

    send_server(SpeechPacket, Len);
	free(SpeechPacket);
	return;
}

void MBOut(const char *title, const char *msg, ...)
{
	char final[4096];
	va_list list;
	va_start(list, msg);
	vsprintf((char*)final, msg, list);
	va_end(list);

	MessageBox(NULL, final, title, 0);
	
	return;
}

unsigned long ArgToInt(char *Arg)
{
	unsigned long Value = 0;
	char neg = 0;

	if(!strlen(Arg))
		return 0;

	/* negativo */
	if(Arg[0] == '-')
	{
		Arg++;
		neg = 1;
	}

	/* check if its a serial (hexa or decimal) */
	if((!strncmp(Arg, "0x", 2) || !strncmp(Arg, "0X", 2)) && (strlen(Arg) > 2 && isxdigit(Arg[2])))
	{
		Value = strtol(Arg, NULL, 16);
		if(neg)
			return 0 - Value;
	}
	else if(isdigit(Arg[0]))
	{
		Value = strtol(Arg, NULL, 10);
		if(neg)
			return 0 - Value;
	}

	if(neg)
		return 0 - Value;

	return Value;
}