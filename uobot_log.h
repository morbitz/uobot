

// Log File
FILE *logFile;
int LogFlag;

// Log functions
void LogDump(unsigned char *pBuffer, int length);
void LogPrint(int Type, const char *strFormat, ...);
void LogOpen(char *file);
void LogClose(void);
void MBOut(const char *title, const char *msg, ...);
void Say(char *Texto, ...);

// Stuff functions
void pack16(unsigned char *Buf, unsigned short x);
void pack32(unsigned char *Buf, unsigned int x);
unsigned short unpack16(const unsigned char *Buf);
unsigned int unpack32(const unsigned char *Buf);
int UnicodeToAscii(const char *UnicodeText, int Len, char *AsciiText);
int chartoint32(char *a, int split, int *c);
unsigned long ArgToInt(char *Arg);