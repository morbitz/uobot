typedef void (*DisconnectFuncType)(int Num);

typedef struct {
	int Used;
	char PluginName[30];
	HMODULE hModule;
	DisconnectFuncType DisconnectFunc;
} PluginType;

PluginType PluginsList[50];
unsigned int PluginsCount;

#define PLUGINSIZE sizeof(PluginsList)/sizeof(PluginType)


int PluginGetFreeIdx(void);
int PluginRemoveCommand(HMODULE hModule);
int PluginAddCommand(const char *Name, void *Handler, HMODULE hModule);
int PluginGetIdx(char *PluginName);
int PluginRemove(char *PluginName, int Idx);
int LoadPluginDll(char *PluginName);
void Plugin_Done(int Num);

void ListPluginsRunning(void);
void ListCommands(void);