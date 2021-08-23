#define JOURNAL_SIZE 100

#define JOURNAL_SPEECH 0
#define JOURNAL_SYSMSG 1
#define JOURNAL_EMOTE 2 /* adds *'s as part of text */
#define JOURNAL_YOUSEE 6 /* You see: */
#define JOURNAL_EMPHASIS 7 /* clears previous messages */
#define JOURNAL_WHISPER 8
#define JOURNAL_YELL 9
#define JOURNAL_SPELL 10
#define JOURNAL_NONE -1
#define JOURNAL_ANY -2

typedef struct tagJournalEntry
{
	int Type;
	char *Text;
}JournalEntry;

void CleanJournal(void);
void JournalAdd(char *Speaker, char *Text, int Type);
void JournalRemove(unsigned int Line);
char *GetJournalLine(unsigned int Line);
char *JournalGetLast(void);
void JournalDump(char * Filename);
void SetJournalLine(unsigned int Line, char *Replace);
int IsInJournal(char *Text, int Type);

JournalEntry Journal[JOURNAL_SIZE];
BOOL JournalReady;
unsigned int JournalCount;